// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#define DLLEXP extern "C" __declspec(dllexport)
/*
这是个简要测试：
1.正常工作 拦截OpenProcess
2.初始化方式
 a. 由MessageHook
 b. 由循环Injector
 c. 由Decedent
3.统计数据
*/
#define PID_MAX 71680
#pragma data_seg("LPKSharedMem")
int flag = 0;//0 nothing 1 reflection install
HHOOK hHook = NULL;
bool descendantMode = false;
volatile BOOL onUinstall = FALSE;
DWORD instPID[PID_MAX + 5] = { 0 }, protPID[PID_MAX + 5]= { 0 };
#pragma data_seg()
#pragma comment(linker,"/SECTION:LPKSharedMem,RWS")

HINSTANCE g_hInstance = NULL;

#ifdef _WIN64
#pragma comment(lib,"detours.lib")
#else
#pragma comment(lib,"detours.lib")
#endif // WIN64

/*
* UINT_MAX=var deleted
*/

void InsertInst(const DWORD val)
{
	if (instPID[0] >= PID_MAX)throw "Table Full";
	size_t pos = val % PID_MAX;
	while (instPID[pos] != 0&&instPID[pos]!=UINT_MAX) {
		++pos; if (pos >= PID_MAX)pos = 1;
	}
	instPID[pos] = val;
	++instPID[0];
}

DLLEXP size_t FindInst(const DWORD val)
{
	size_t cnt = 0;
	size_t pos = val % PID_MAX;
	while (instPID[pos] != val) {
		if (cnt >= PID_MAX)return UINT_MAX;
		if (instPID[pos] == 0)return UINT_MAX;
		++pos; if (pos >= PID_MAX)pos = 1;
		++cnt;
	}
	return pos;
}

void RemoveInst(const DWORD val)
{
	size_t pos = FindInst(val);
	if (pos != UINT_MAX)instPID[pos] = UINT_MAX;
	--instPID[0];
}

DLLEXP size_t CountInst()
{
	return instPID[0];
}

DLLEXP size_t FindProt(const DWORD val);

DLLEXP void InsertProt(const DWORD val)
{
	if (protPID[0] >= PID_MAX)MessageBox(NULL, L"Process Staitc:Table Full", L"LPK", MB_OK | MB_ICONWARNING);//throw "Table Full";
	if (FindProt(val) != UINT_MAX)return;
	size_t pos = val % PID_MAX;
	while (protPID[pos] != 0 && protPID[pos] != UINT_MAX) {
		++pos; if (pos >= PID_MAX)pos = 1;
	}
	protPID[pos] = val;
	++protPID[0];
}

DLLEXP size_t FindProt(const DWORD val)
{
	size_t cnt = 0;
	size_t pos = val % PID_MAX;
	while (protPID[pos] != val) {
		if (cnt >= PID_MAX)return UINT_MAX;;
		if (protPID[pos] == 0)return UINT_MAX;
		++pos; if (pos >= PID_MAX)pos = 1;
		++cnt;
	}
	return pos;
}

DLLEXP void RemoveProt(const DWORD val)
{
	size_t pos = FindProt(val);
	if (pos != UINT_MAX)protPID[pos] = UINT_MAX;
	if(protPID[0]>0)--protPID[0];
	//Sometimes double remove occur, but I don't know why
}

DLLEXP size_t CountProt()
{
	return protPID[0];
}



typedef HANDLE(CALLBACK* OpenProcess_t)(DWORD, BOOL, DWORD);

OpenProcess_t orinOpenProcess = OpenProcess;

HANDLE CALLBACK hOpenProcess(DWORD dwDesiredAccess, BOOL  bInheritHandle, DWORD dwProcessId)
{
	if (FindProt(dwProcessId) != UINT_MAX) {
		SetLastError(5);
		return NULL;
	}
	else return orinOpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
}

//Install() has been move to line 250


void Remove()
{
	DetourTransactionBegin();
	DetourDetach(&orinOpenProcess, hOpenProcess);
	DetourTransactionCommit();
}

LRESULT CALLBACK MsgHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (onUinstall)Remove();
	return CallNextHookEx(hHook, code, wParam, lParam);
}

HMODULE WINAPI ModuleFromAddress(PVOID pv) {
	MEMORY_BASIC_INFORMATION mbi;
	if (::VirtualQuery(pv, &mbi, sizeof(mbi)) != 0)
		return (HMODULE)mbi.AllocationBase;
	else
		return NULL;
}


DLLEXP BOOL InstallGlobal()
{
	hHook = SetWindowsHookEx(WH_GETMESSAGE, MsgHookProc, ModuleFromAddress(MsgHookProc), 0);
	return hHook != NULL;
}

DLLEXP void RemoveGlobal()
{
	InterlockedExchange((LONG*)(&onUinstall), TRUE);
}

DLLEXP void RemoveGlobalImmediately()
{
	InterlockedExchange((LONG*)(&onUinstall), TRUE);
	PostMessage(HWND_BROADCAST, WM_KILLFOCUS, 0, 0);
	int cnt = 0;
	while (CountInst() && cnt <= 100) {
		Sleep(10);
		++cnt;
	}
}

typedef BOOL (WINAPI* CreateProcessW_t)(
	 LPCWSTR lpApplicationName,
	 LPWSTR lpCommandLine,
	 LPSECURITY_ATTRIBUTES lpProcessAttributes,
	 LPSECURITY_ATTRIBUTES lpThreadAttributes,
	 BOOL bInheritHandles,
	 DWORD dwCreationFlags,
	 LPVOID lpEnvironment,
	 LPCWSTR lpCurrentDirectory,
	 LPSTARTUPINFOW lpStartupInfo,
	 LPPROCESS_INFORMATION lpProcessInformation
);

CreateProcessW_t orinCreateProcessW = CreateProcessW;

BOOL WINAPI hCreateProcessW(
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
) {
	Beep(523, 100);
	if (descendantMode)
		return DetourCreateProcessWithDllExW(lpApplicationName, lpCommandLine, lpProcessAttributes,
			lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
			lpProcessInformation, "LPK64.dll", orinCreateProcessW);
	else
		return orinCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes,
			lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
			lpProcessInformation);
}

typedef BOOL (WINAPI *CreateProcessA_t)(
	LPCSTR lpApplicationName,
	LPSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCSTR lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
);

CreateProcessA_t orinCreateProcessA = CreateProcessA;

BOOL WINAPI hCreateProcessA(
	LPCSTR lpApplicationName,
	LPSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCSTR lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
) {
	if (descendantMode)
		return DetourCreateProcessWithDllExA(lpApplicationName, lpCommandLine, lpProcessAttributes,
			lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
			lpProcessInformation, "LPK64.dll", orinCreateProcessA);
	else
		return orinCreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes,
			lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
			lpProcessInformation);
}


void Install()
{
	DetourTransactionBegin();
	DetourAttach(&orinOpenProcess, hOpenProcess);
	DetourTransactionCommit();
	if (descendantMode) {
		DetourTransactionBegin();
		DetourAttach(&orinCreateProcessA, hCreateProcessA);
		DetourAttach(&orinCreateProcessW, hCreateProcessW);
		DetourTransactionCommit();
	}
}

DLLEXP void EnableDescendant()//挂钩CreateProcess到WithDllEx
{
	descendantMode = true;
}

DLLEXP void DisableDescendant()
{
	descendantMode = false;
}

DLLEXP void SetFlag(int val)
{
	flag = val;
}

#include <psapi.h>
#pragma comment(lib, "psapi.lib")
bool IsCurrentProcessExplorer()
{
    WCHAR szProcessPath[MAX_PATH] = {0};
    if (GetModuleFileNameExW(GetCurrentProcess(), NULL, szProcessPath, MAX_PATH))
    {
        const WCHAR* p = wcsrchr(szProcessPath, L'\\');
        return p && _wcsicmp(p + 1, L"explorer.exe") == 0;
    }
    return false;
}

void InjectProcess();

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	DWORD pid =GetCurrentProcessId();
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		if (IsCurrentProcessExplorer()&&flag==1) {
			if(hHook==NULL)
				InstallGlobal();
		}
		g_hInstance = hModule;
		Install();
		InsertInst(pid);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (FindInst(pid)) {
			Remove();
			RemoveInst(pid);
		}
		if(FindProt(pid))
			RemoveProt(pid);
		break;
	}
	return TRUE;
}

