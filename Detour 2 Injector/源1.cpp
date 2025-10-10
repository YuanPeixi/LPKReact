
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>

using namespace std;

bool Inject(DWORD dwId, WCHAR* szPath)//参数1：目标进程PID  参数2：DLL路径
{
    //一、在目标进程中申请一个空间
    /*
    【1.1 获取目标进程句柄】
    参数1：想要拥有的进程权限（本例为所有能获得的权限）
    参数2：表示所得到的进程句柄是否可以被继承
    参数3：被打开进程的PID
    返回值:指定进程的句柄
    */
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwId);
    /*
    【1.2 在目标进程的内存里开辟空间】
    参数1：目标进程句柄
    参数2：保留页面的内存地址，一般用NULL自动分配
    参数3：欲分配的内存大小，字节单位
    参数4：MEM_COMMIT：为特定的页面区域分配内存中或磁盘的页面文件中的物理存储
    参数5：PAGE_READWRITE 区域可被应用程序读写
    返回值：执行成功就返回分配内存的首地址，不成功就是NULL
    */
    LPVOID pRemoteAddress = VirtualAllocEx(
        hProcess,
        NULL,
        wcslen(szPath) * 2,
        MEM_COMMIT,
        PAGE_READWRITE
    );

    //二、 把dll的路径写入到目标进程的内存空间中

    DWORD dwWriteSize = 0;
    /*
    【写一段数据到刚才给指定进程所开辟的内存空间里】
    参数1：OpenProcess返回的进程句柄
    参数2：准备写入的内存首地址
    参数3：指向要写的数据的指针（准备写入的东西）
    参数4：要写入的字节数（东西的长度+0/）
    参数5： 返回值。返回实际写入的字节
    */
    BOOL bRet = WriteProcessMemory(hProcess, pRemoteAddress, szPath, wcslen(szPath) * 2, NULL);
    //三、 创建一个远程线程，让目标进程调用LoadLibrary

    /*
    参数1：该远程线程所属进程的进程句柄
    参数2：一个指向 SECURITY_ATTRIBUTES 结构的指针, 该结构指定了线程的安全属性
    参数3：线程栈初始大小,以字节为单位,如果该值设为0,那么使用系统默认大小
    参数4：在远程进程的地址空间中,该线程的线程函数的起始地址（也就是这个线程具体要干的活儿）
    参数5：传给线程函数的参数（刚才在内存里开辟的空间里面写入的东西）
    参数6：控制线程创建的标志。0（NULL）表示该线程在创建后立即运行
    参数7：指向接收线程标识符的变量的指针。如果此参数为NULL，则不返回线程标识符
    返回值：如果函数成功，则返回值是新线程的句柄。如果函数失败，则返回值为NULL
    */
    // #5.获取模块地址
    HMODULE hModule = GetModuleHandle(L"kernel32.dll");
    if (!hModule)
    {
        printf("GetModuleHandle Error !\n");
        GetLastError();
        CloseHandle(hProcess);
        return FALSE;
    }
    // #6.获取LoadLibraryA 函数地址
    LPTHREAD_START_ROUTINE dwLoadAddr = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "LoadLibraryW");
    if (!dwLoadAddr)
    {
        printf("GetProcAddress Error !\n");
        GetLastError();
        CloseHandle(hProcess);
        CloseHandle(hModule);
        return FALSE;
    }
    // //7.创建远程线程,加载dll

    HANDLE hThread = CreateRemoteThread(
        hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)dwLoadAddr,
        pRemoteAddress,
        NULL,
        NULL
    );
    //WaitForSingleObject(hThread, -1); //当句柄所指的线程有信号的时候，才会返回

    ///*
    //四、 【释放申请的虚拟内存空间】
    //参数1：目标进程的句柄。该句柄必须拥有 PROCESS_VM_OPERATION 权限
    //参数2：指向要释放的虚拟内存空间首地址的指针
    //参数3：虚拟内存空间的字节数
    //参数4：MEM_DECOMMIT仅标示内存空间不可用，内存页还将存在。
    //       MEM_RELEASE这种方式很彻底，完全回收。
    //*/
    //VirtualFreeEx(hProcess, pRemoteAddress, 1, MEM_DECOMMIT);
    return 0;
}

BOOL EnableDebugPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return FALSE;

    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL result = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
    CloseHandle(hToken);
    return result;
}

DWORD GetProcessIdByName(LPCTSTR processName) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_tcsicmp(pe32.szExeFile, processName) == 0) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    return 0;
}

bool InjectDLL(DWORD pid, LPCTSTR dllPath) {
    HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
        FALSE, pid);
    if (!hProcess) {
        wcerr << _T("OpenProcess failed: ") << GetLastError() << std::endl;
        return false;
    }

    size_t pathSize = (_tcslen(dllPath) + 1) * sizeof(TCHAR);
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!pRemoteMem) {
        CloseHandle(hProcess);
        wcerr << _T("VirtualAllocEx failed: ") << GetLastError() << std::endl;
        return false;
    }

    if (!WriteProcessMemory(hProcess, pRemoteMem, dllPath, pathSize, NULL)) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        wcerr << _T("WriteProcessMemory failed: ") << GetLastError() << std::endl;
        return false;
    }
    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandle(_T("kernel32.dll")),
#ifdef UNICODE
        "LoadLibraryW"
#else
        "LoadLibraryA"
#endif
    );
    if (!pLoadLibrary) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        wcerr << _T("GetProcAddress failed") << std::endl;
        return false;
    }

    //Before Execute I add this
    DWORD prevMemProtect = 0;
    VirtualProtectEx(hProcess, pRemoteMem, sizeof(pRemoteMem), PAGE_EXECUTE_READWRITE, &prevMemProtect);
    VirtualProtectEx(hProcess, pRemoteMem, sizeof(pRemoteMem), PAGE_READWRITE, &prevMemProtect);
    VirtualProtectEx(hProcess, pRemoteMem, sizeof(pRemoteMem), PAGE_EXECUTE_READWRITE, &prevMemProtect);

    DWORD tid=0;
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)pLoadLibrary, pRemoteMem, 0, &tid);
    if (!hRemoteThread) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        wcerr << _T("CreateRemoteThread failed: ") << GetLastError() << std::endl;
        return false;
    }

    WaitForSingleObject(hRemoteThread, INFINITE);

    DWORD exitCode;
    GetExitCodeThread(hRemoteThread, &exitCode);
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
    CloseHandle(hRemoteThread);
    CloseHandle(hProcess);

    return exitCode != 0;
}


int _tmain3(int argc, TCHAR* argv[]) {
    if (argc != 3) {
        _tprintf(_T("Usage: %s <process_name> <dll_path>\n"), argv[0]);
        return 1;
    }

    if (!EnableDebugPrivilege()) {
        wcerr << _T("Warning: Failed to enable debug privilege") << std::endl;
    }

    DWORD pid = GetProcessIdByName(argv[1]);
    if (!pid) {
        wcerr << _T("Process not found: ") << argv[1] << std::endl;
        return 1;
    }
    wcout << "ProcessId=" << pid << endl;
    if (Inject(pid, argv[2])) {
        wcout << _T("DLL injected successfully!") << std::endl;
        return 0;
    }
    else {
        wcerr << _T("DLL injection failed") << std::endl;
        return 1;
    }
}
