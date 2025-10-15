// LoadLPK.h - 简化版动态加载LPK64.dll的头文件
//Version 2025-10-5
#pragma once
#include <Windows.h>

// 函数指针类型声明
typedef BOOL(WINAPI* PFN_InstallGlobal)();
typedef void (WINAPI* PFN_RemoveGlobal)();
typedef void (WINAPI* PFN_RemoveGlobalImmediately)();
typedef void (WINAPI* PFN_EnableDescendant)();
typedef void (WINAPI* PFN_DisableDescendant)();
typedef void (WINAPI* PFN_SetFlag)(int val);
typedef void (WINAPI* PFN_SetDLLPath)(const char path[]);
typedef void (WINAPI* PFN_SetDLLPathForce)(const char path[]);
typedef void (WINAPI* PFN_SetWorkProcess)(DWORD pid);
typedef size_t(WINAPI* PFN_FindInst)(DWORD val);
typedef size_t(WINAPI* PFN_CountInst)();
typedef size_t(WINAPI* PFN_FindProt)(DWORD val);
typedef void (WINAPI* PFN_InsertProt)(DWORD val);
typedef void (WINAPI* PFN_RemoveProt)(DWORD val);
typedef size_t(WINAPI* PFN_CountProt)();


// 函数指针变量定义
PFN_InstallGlobal InstallGlobal = NULL;
PFN_RemoveGlobal RemoveGlobal = NULL;
PFN_RemoveGlobalImmediately RemoveGlobalImmediately = NULL;
PFN_EnableDescendant EnableDescendant = NULL;
PFN_DisableDescendant DisableDescendant = NULL;
PFN_SetFlag SetFlag = NULL;
PFN_SetDLLPath SetDLLPath = NULL;
PFN_SetDLLPathForce SetDLLPathForce = NULL;
PFN_SetWorkProcess SetWorkProcess = NULL;
PFN_FindInst FindInst = NULL;
PFN_CountInst CountInst = NULL;
PFN_FindProt FindProt = NULL;
PFN_InsertProt InsertProt = NULL;
PFN_RemoveProt RemoveProt = NULL;
PFN_CountProt CountProt = NULL;

BOOL InitSimple(const char path[]="LPK64.dll")
{
    // 初始化函数指针
    HMODULE hDll = LoadLibraryA(path);
    if (hDll) {
        InstallGlobal = (PFN_InstallGlobal)GetProcAddress(hDll, "InstallGlobal");
        RemoveGlobal = (PFN_RemoveGlobal)GetProcAddress(hDll, "RemoveGlobal");
        RemoveGlobalImmediately = (PFN_RemoveGlobalImmediately)GetProcAddress(hDll, "RemoveGlobalImmediately");
        EnableDescendant = (PFN_EnableDescendant)GetProcAddress(hDll, "EnableDescendant");
        DisableDescendant = (PFN_DisableDescendant)GetProcAddress(hDll, "DisableDescendant");
        SetFlag = (PFN_SetFlag)GetProcAddress(hDll, "SetFlag");
        SetDLLPath = (PFN_SetDLLPath)GetProcAddress(hDll, "SetDLLPath");
        SetDLLPathForce = (PFN_SetDLLPathForce)GetProcAddress(hDll, "SetDLLPathForce");
        SetWorkProcess = (PFN_SetWorkProcess)GetProcAddress(hDll, "SetWorkProcess");
        FindInst = (PFN_FindInst)GetProcAddress(hDll, "FindInst");
        CountInst = (PFN_CountInst)GetProcAddress(hDll, "CountInst");
        FindProt = (PFN_FindProt)GetProcAddress(hDll, "FindProt");
        InsertProt = (PFN_InsertProt)GetProcAddress(hDll, "InsertProt");
        RemoveProt = (PFN_RemoveProt)GetProcAddress(hDll, "RemoveProt");
        CountProt = (PFN_CountProt)GetProcAddress(hDll, "CountProt");
    }
    return InstallGlobal && RemoveGlobal && EnableDescendant && DisableDescendant\
        && SetFlag && SetDLLPath && SetDLLPathForce && SetWorkProcess && \
        FindInst && CountInst && FindProt && CountProt && InsertProt && RemoveProt;
}