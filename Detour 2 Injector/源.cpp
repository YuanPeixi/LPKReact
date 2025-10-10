#include <windows.h>
#include <iostream>
#include <string>
#include <tchar.h>

#include "LoadLPK.h"

//参考文献 https://www.marxcbr.cn/archives/1dd9aa1

void TestProcessInterception();
void TestProcessManagement();
void TestHookFunctions();

int main2()
{

    // 测试进程拦截功能
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
    if (hProcess) {
        std::cout << "  √ 成功打开进程(未被拦截)" << std::endl;
        CloseHandle(hProcess);
    }
    else {
        std::cout << "  × 打开进程失败" << std::endl;
    }

    // 加载DLL
    //hDll = LoadLibrary(_T("LPK.dll"));
    if (!InitSimple()) {
        return 1;
    }
    else {
        std::cout << "Dll 加载成功" << std::endl;
    }

    std::cout << "=== 开始测试DLL功能 ===" << std::endl;



    // 测试进程管理功能
    TestProcessManagement();

    // 测试进程拦截功能
    TestProcessInterception();

    std::cout << "为当期进程添加保护" << std::endl;
    InsertProt(GetCurrentProcessId());

    // 测试进程拦截功能
    TestProcessInterception();

    // 测试钩子功能
    TestHookFunctions();

    std::cout << "=== 测试完成 ===" << std::endl;

    FreeLibrary(hDll);
    getchar();
    return 0;
}

void TestProcessManagement()
{
    std::cout << "\n[测试进程管理功能]" << std::endl;

    DWORD currentPid = GetCurrentProcessId();

    // 测试插入保护进程
    std::cout << "1. 插入当前进程到保护列表..." << std::endl;
    InsertProt(currentPid);

    // 验证插入结果
    if (FindProt(currentPid) != UINT_MAX) {
        std::cout << "  √ 成功插入进程ID: " << currentPid << std::endl;
    }
    else {
        std::cout << "  × 插入进程失败" << std::endl;
    }

    // 测试计数功能
    std::cout << "2. 保护进程计数: " << CountProt() << std::endl;

    // 测试移除功能
    std::cout << "3. 从保护列表移除当前进程..." << std::endl;
    RemoveProt(currentPid);

    if (FindProt(currentPid) == UINT_MAX) {
        std::cout << "  √ 成功移除进程ID: " << currentPid << std::endl;
    }
    else {
        std::cout << "  × 移除进程失败" << std::endl;
    }
}

void TestProcessInterception()
{
    std::cout << "\n[测试进程拦截功能]" << std::endl;

    DWORD currentPid = GetCurrentProcessId();

    // 测试OpenProcess拦截
    std::cout << "1. 测试OpenProcess拦截..." << std::endl;

    // 首先确保当前进程不在拦截列表中
    if (FindProt(currentPid) == UINT_MAX) {
        std::cout << "  √ 当前进程未被拦截" << std::endl;
    }
    else {
        std::cout << "  × 当前进程已被拦截" << std::endl;
    }

    // 尝试打开当前进程
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, currentPid);
    if (hProcess) {
        std::cout << "  √ 成功打开进程(未被拦截)" << std::endl;
        CloseHandle(hProcess);
    }
    else {
        std::cout << "  × 打开进程失败" << std::endl;
    }
}

void TestHookFunctions()
{
    std::cout << "\n[测试钩子功能]" << std::endl;

    // 测试全局钩子安装
    std::cout << "1. 测试全局钩子安装..." << std::endl;

    // 注意: 实际测试需要DLL实现InstallGlobal和RemoveGlobal的导出
    // 这里假设这些函数已导出

    
    // 如果DLL导出这些函数，可以这样测试:
    typedef BOOL (*InstallGlobal_t)();
    typedef void (*RemoveGlobal_t)();

    InstallGlobal_t pInstallGlobal = (InstallGlobal_t)GetProcAddress(hDll, "InstallGlobal");
    RemoveGlobal_t pRemoveGlobal = (RemoveGlobal_t)GetProcAddress(hDll, "RemoveGlobal");

    if (pInstallGlobal && pRemoveGlobal) {
        if (pInstallGlobal()) {
            std::cout << "  √ 成功安装全局钩子" << std::endl;

            std::cout << "已加载的进程计数=" << CountInst() << std::endl;

            // 测试钩子移除
            pRemoveGlobal();
            std::cout << "  √ 成功移除全局钩子" << std::endl;
        } else {
            std::cout << "  × 安装全局钩子失败" << std::endl;
        }
    } else {
        std::cout << "  × 无法获取钩子函数" << std::endl;
    }
    

    std::cout << "  ! 钩子功能测试需要DLL导出InstallGlobal和RemoveGlobal函数" << std::endl;
}
