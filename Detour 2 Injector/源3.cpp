
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>

// 通过进程名获取PID
DWORD GetProcessIdByName(const TCHAR* processName) {
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

// 核心注入函数
bool InjectDLL(DWORD pid, const TCHAR* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        std::cerr << "OpenProcess failed: " << GetLastError() << std::endl;
        return false;
    }

    // 在目标进程分配内存
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, NULL, _tcslen(dllPath) * sizeof(TCHAR) + 1,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMem) {
        CloseHandle(hProcess);
        std::cerr << "VirtualAllocEx failed: " << GetLastError() << std::endl;
        return false;
    }

    // 写入DLL路径
    if (!WriteProcessMemory(hProcess, pRemoteMem, dllPath,
        _tcslen(dllPath) * sizeof(TCHAR) + 1, NULL)) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        std::cerr << "WriteProcessMemory failed: " << GetLastError() << std::endl;
        return false;
    }

    // 获取LoadLibrary地址
    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandle(_T("kernel32.dll")),
#ifdef UNICODE
        "LoadLibraryW"
#else
        "LoadLibraryA"
#endif
    );

    // 创建远程线程
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)pLoadLibrary,
        pRemoteMem, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        std::cerr << "CreateRemoteThread failed: " << GetLastError() << std::endl;
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    // 清理资源
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}

int _tmain(int argc, TCHAR* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <ProcessName> <DllPath>" << std::endl;
        return 1;
    }

    DWORD pid = GetProcessIdByName(argv[1]);
    if (!pid) {
        std::cerr << "Process not found: " << argv[1] << std::endl;
        return 1;
    }

    if (InjectDLL(pid, argv[2])) {
        std::cout << "DLL injected successfully!" << std::endl;
        return 0;
    }
    return 1;
}
