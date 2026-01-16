#ifndef WATCHDOG_SERVICE_H
#define WATCHDOG_SERVICE_H

// windows服务开发必需的头文件
#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <iostream>

/*
📖 知识点：为什么需要这些头文件？
- windows.h: Windows API的核心头文件
- tchar.h:   Unicode/ANSI字符串处理的兼容头文件
- stdio.h:   标准输入输出（用于调试日志）(strsafe.h中已包含，删除)
- strsafe.h: 安全的字符串处理函数
*/

// 服务配置常量
#define SERVICE_NAME _T("WatchdogService") // 服务内部名称
#define SERVICE_DISPLAY_NAME _T("智慧会务看门狗服务") // 服务显示名称
#define SERVICE_DESCRIPTION _T("监控智慧会务程序") // 服务注释

// 服务状态全局变量
extern SERVICE_STATUS g_ServiceStatus;
extern SERVICE_STATUS_HANDLE g_ServiceHandle;
extern HANDLE g_ServiceStopEvent;

// 服务控制函数声明
VOID WINAPI ServiceMain(DWORD argc,LPTSTR* argv);
VOID WINAPI ServiceCtrlHandle(DWORD dwControl);


// 服务工作函数
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
void ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
void StartLogger();// 启动日志
void StopLogger();// 停止日志



#endif // WATCHDOG_SERVICE_H
