#include <iostream>
#include <Windows.h>
using namespace std;

#include "LoadLPK.h"

/*
持久化通过以下方式：
1.MessageHook法 =>驻留自己
				=>启用DaemonProc(explorer.exe)
					[MainProcess] SetMessageHook(0)
					[MainProcess] PostMessage(DaemonProc,WM_NULL)
					[DaemonProc] MsgHookProc()=> SetWindowsHookEx(1) UnhookWindowsHookEx(0)
2.Descendants法=>注入explorer.exe
			   =>启用Descent模式
					explorer.exe子进程均被注入
					如果当前explorer不等于先前记录值 则由其中一个进程负责重新注入
3.循环驻留法=>注入explorer.exe
			=>explorer.exe创建工作子线程负责循环注入/MSG_HOOK
*/

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

bool ExecuteRemotely(DWORD pid, const TCHAR* modulePath,const char* funcName) {//only supports: void func(void)
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        std::cerr << "OpenProcess failed: " << GetLastError() << std::endl;
        return false;
    }

    // 在目标进程分配内存
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, NULL, _tcslen(modulePath) * sizeof(TCHAR) + 1,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMem) {
        CloseHandle(hProcess);
        std::cerr << "VirtualAllocEx failed: " << GetLastError() << std::endl;
        return false;
    }

    // 获取LoadLibrary地址
    HMODULE hModule = LoadLibraryA("E:\\LPK.dll");
    if (!hModule) {
        cerr << "Faild to Get Module Handle";
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(hModule, funcName);

    if (!pLoadLibrary) {
        cerr << "Faild to Get Function Address";
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hModule);
        CloseHandle(hProcess);
        return false;
    }

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

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);

    // 清理资源
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}

int main()
{
    if (!InitSimple("C:\\Windows\\System32\\LPK64.dll"))cerr << "Faile to load dll" << endl;
    EnableDescendant();
    DWORD pid;
    cout << "pid=";
    cin >> pid;
    if (InjectDLL(pid, L"C:\\Windows\\System32\\LPK64.dll"))cout << "Inject Success" << endl; 
    while (true) {
        cout << "Installed Process Count=" << CountInst() << endl;
        cout << "Protected Process Count=" << CountProt() << endl;
        DWORD pid = 0;
        cout << "Add PID=";
        cin >> pid;
        if (pid != 0) {
            InsertProt(pid);
            pid = 0;
        }
        cout << "Remove PID=";
        cin >> pid;
        if (pid == -1)break;
        if (pid != 0) {
            RemoveProt(pid);
        }
    }
	return 0;
}

int main2()
{
	if (!InitSimple("Detour Test 2.dll")) {
		cerr << "Failed to init" << endl;
	}
	if (InstallGlobal()) {
		cout << "Set Windows Hook Successfully" << endl;
	}
	else {
		cout << "Set Windows Hook Failed" << endl;
	}
	while (true) {
		cout << "Installed Process Count=" << CountInst() << endl;
		cout << "Protected Process Count=" << CountProt() << endl;
		DWORD pid=0;
		cout << "Add PID=";
		cin >> pid;
		if (pid != 0) {
			InsertProt(pid);
			pid = 0;
		}
		cout << "Remove PID=";
		cin >> pid;
		if (pid == -1)break;
		if (pid != 0) {
			RemoveProt(pid);
		}
	}
	//RemoveGlobal();
	return 0;
}