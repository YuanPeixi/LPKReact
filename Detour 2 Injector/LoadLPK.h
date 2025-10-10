#pragma once
#include <windows.h>
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

    // 定义函数指针类型
    typedef size_t(*FindInst_t)(DWORD);
    typedef size_t(*CountInst_t)(void);
    typedef void (*InsertProt_t)(DWORD);
    typedef size_t(*FindProt_t)(DWORD);
    typedef void (*RemoveProt_t)(DWORD);
    typedef size_t(*CountProt_t)(void);
    typedef BOOL(*InstallGlobal_t)(void);
    typedef void (*RemoveGlobal_t)(void);
    typedef void (*RemoveGlobalImmediately_t)(void);

    // 声明全局函数指针变量
    extern FindInst_t FindInst;
    extern CountInst_t CountInst;
    extern InsertProt_t InsertProt;
    extern FindProt_t FindProt;
    extern RemoveProt_t RemoveProt;
    extern CountProt_t CountProt;
    extern InstallGlobal_t InstallGlobal;
    extern RemoveGlobal_t RemoveGlobal;
    extern RemoveGlobalImmediately_t RemoveGlobalImmediately;

    // 初始化函数
    BOOL InitSimple(LPCSTR dllPath = "LPK.dll");

    // 清理函数
    void CleanupSimple();

#ifdef __cplusplus
}
#endif

FindInst_t FindInst = nullptr;
CountInst_t CountInst = nullptr;
InsertProt_t InsertProt = nullptr;
FindProt_t FindProt = nullptr;
RemoveProt_t RemoveProt = nullptr;
CountProt_t CountProt = nullptr;
InstallGlobal_t InstallGlobal = nullptr;
RemoveGlobal_t RemoveGlobal = nullptr;
RemoveGlobalImmediately_t RemoveGlobalImmediately = nullptr;

static HMODULE hDll = nullptr;

BOOL InitSimple(LPCSTR dllPath)
{
    hDll = LoadLibraryA(dllPath);
    if (!hDll) {
        std::cerr << "无法加载DLL: " << dllPath << std::endl;
        return FALSE;
    }

    // 初始化所有函数指针
    FindInst = (FindInst_t)GetProcAddress(hDll, "FindInst");
    CountInst = (CountInst_t)GetProcAddress(hDll, "CountInst");
    InsertProt = (InsertProt_t)GetProcAddress(hDll, "InsertProt");
    FindProt = (FindProt_t)GetProcAddress(hDll, "FindProt");
    RemoveProt = (RemoveProt_t)GetProcAddress(hDll, "RemoveProt");
    CountProt = (CountProt_t)GetProcAddress(hDll, "CountProt");
    InstallGlobal = (InstallGlobal_t)GetProcAddress(hDll, "InstallGlobal");
    RemoveGlobal = (RemoveGlobal_t)GetProcAddress(hDll, "RemoveGlobal");
    RemoveGlobalImmediately = (RemoveGlobalImmediately_t)GetProcAddress(hDll, "RemoveGlobalImmediately");

    // 检查是否所有必要函数都加载成功
    if (!FindInst || !CountInst || !InsertProt ||
        !FindProt || !RemoveProt || !CountProt ||
        !InstallGlobal || !RemoveGlobal || !RemoveGlobalImmediately) {
        FreeLibrary(hDll);
        hDll = nullptr;
        return FALSE;
    }

    return TRUE;
}

void CleanupSimple()
{
    if (hDll) {
        FreeLibrary(hDll);
        hDll = nullptr;
    }

    // 重置所有函数指针
    FindInst = nullptr;
    CountInst = nullptr;
    InsertProt = nullptr;
    FindProt = nullptr;
    RemoveProt = nullptr;
    CountProt = nullptr;
    InstallGlobal = nullptr;
    RemoveGlobal = nullptr;
    RemoveGlobalImmediately = nullptr;
}