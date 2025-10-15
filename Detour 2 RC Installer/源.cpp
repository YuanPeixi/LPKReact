#include <iostream>
#include <Windows.h>
#include "detours.h"
#include <TlHelp32.h>

using namespace std;

#pragma comment(lib,"detours.lib")

//This is a simple example show how to use this lib

DWORD GetPID(const WCHAR name[])
{
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_wcsicmp(pe32.szExeFile, name) == 0) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return 0;
}

int main()
{
	//system("copy \"Detour Test 2 RC.dll\" \"%SystemRoot%\\System32\\LPK64.dll\"");
	system("copy \"Detour Test 2 RC.dll\" \"C:\\Windows\\System32\\LPK64.dll\"");
	WCHAR cmdLine[] = L"explorer.exe";
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	TerminateProcess(OpenProcess(PROCESS_TERMINATE, FALSE, GetPID(L"explorer.exe")), 0);
	DetourCreateProcessWithDllW(NULL, cmdLine,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi,"C:\\Windows\\System32\\LPK64.dll",CreateProcessW);
	return 0;
}