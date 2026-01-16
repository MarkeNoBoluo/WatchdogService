#include "watchdog_service.h"
#include "service_install.h"
#include <ShlObj.h>
#include "Logger.h"
#include <stdio.h>

// 全局变量定义
SERVICE_STATUS        g_ServiceStatus = {};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = NULL;

/*
📖 知识点：理解这三个关键全局变量
1. g_ServiceStatus: 存储服务的当前状态（已停止、启动中、停止中、运行中、继续中、暂停中、暂停）
2. g_StatusHandle: 服务状态句柄，用于向SCM报告状态
3. g_ServiceStopEvent: 事件对象，用于线程间通信，通知服务停止
*/

/*
📖 知识点：ReportServiceStatus - 服务与SCM的"心跳机制"
SCM（服务控制管理器）需要定期知道服务的状态。
如果不及时报告，SCM会认为服务"无响应"。
*/
void ReportServiceStatus(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint) {

    static DWORD dwCheckPoint = 1;

    // 填充服务状态结构体
    g_ServiceStatus.dwCurrentState = dwCurrentState;
    g_ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
    g_ServiceStatus.dwWaitHint = dwWaitHint;

    // 根据状态设置其他字段
    if (dwCurrentState == SERVICE_START_PENDING) { // 启动服务中
        g_ServiceStatus.dwControlsAccepted = 0; // 启动中不接受控制
    } else {
        g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP; // 接受停止命令
    }

    // 设置检查点（用于跟踪长时间操作）
    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED)) {
        g_ServiceStatus.dwCheckPoint = 0;
    } else {
        g_ServiceStatus.dwCheckPoint = dwCheckPoint++;
    }

    // 📖 关键API：SetServiceStatus - 向SCM报告当前状态
    // 这是服务必须定期调用的最重要的API之一
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    LOG_INFO("报告服务状态: %d, 检查点: %d\n",
               dwCurrentState, g_ServiceStatus.dwCheckPoint);
}

/*
📖 知识点：ServiceCtrlHandler - 服务的"遥控器"
当用户通过服务管理器点击"停止"、"暂停"等按钮时，
SCM会调用这个函数。你必须在这里处理控制请求。
*/
VOID WINAPI ServiceCtrlHandler(DWORD dwControl) {
    LOG_INFO("接收到控制请求: %d\n", dwControl);

    switch (dwControl) {
    case SERVICE_CONTROL_STOP:
        LOG_INFO("正在停止服务...\n");

        // 报告"正在停止"状态
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

        // 设置停止事件，通知工作线程退出
        SetEvent(g_ServiceStopEvent);

        // 报告最终状态
        ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
        break;

    case SERVICE_CONTROL_INTERROGATE:
        // 只是查询状态，直接报告当前状态即可
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        break;

    default:
        LOG_INFO("未知控制码: %d\n", dwControl);
        break;
    }
}

/*
📖 知识点：服务工作线程 - 服务的"大脑"
这是服务实际工作的地方。对于看门狗服务，
这里将实现监控其他服务状态的循环。
*/
DWORD WINAPI ServiceWorkerThread(PVOID lpParam) {
    (void)lpParam; // 避免C4100未引用参数警告

    LOG_INFO("服务工作线程启动");

    int iteration = 0;
    while (WaitForSingleObject(g_ServiceStopEvent, 5000) != WAIT_OBJECT_0) {
        iteration++;
        LOG_DEBUG("看门狗服务运行中... 迭代次数: %d", iteration);

        // 📌 TODO: 这里将添加监控其他服务状态的逻辑
        // 示例：每10次迭代记录一次INFO日志
        if (iteration % 10 == 0) {
            LOG_INFO("服务持续运行中，已迭代 %d 次", iteration);
        }
    }

    LOG_INFO("服务工作线程退出");
    return ERROR_SUCCESS;
}

/*
📖 知识点：ServiceMain - 服务的"main()函数"
这是服务的入口点，由SCM调用。注意参数格式是固定的。
*/
VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
    (void)argc;(void)argv; // 避免C4100未引用参数警告
    // 启动日志
    StartLogger();

    // 步骤1: 立即注册服务控制处理器
    // 📖 关键API：RegisterServiceCtrlHandler - 告诉SCM我们的控制处理器是谁
    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (!g_StatusHandle) {
        HANDLE hEventLog = RegisterEventSourceW(NULL, L"Application");
        if (hEventLog) {
            const wchar_t* strings[] = {
                L"WatchdogService: Failed to register service control handler"
            };
            ReportEventW(hEventLog, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, strings, NULL);
            DeregisterEventSource(hEventLog);
        }
        return;
    }

    // 步骤2: 报告"服务正在启动"
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // 步骤3: 创建停止事件（用于线程同步）
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!g_ServiceStopEvent) {
        //LOG_ERROR("创建停止事件失败! 错误: %d\n", GetLastError());
        ReportServiceStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    // 步骤4: 报告"服务正在运行"
    ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

    // 步骤5: 创建工作线程执行实际任务 进入工作线程循环
    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);
    if (!hThread) {
        //LOG_ERROR("创建工作线程失败! 错误: %d\n", GetLastError());
        ReportServiceStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    // 步骤6: 等待停止事件（阻塞直到收到停止信号）
    //LOG_INFO("服务主线程等待停止事件...\n");
    WaitForSingleObject(g_ServiceStopEvent, INFINITE);

    // 步骤7: 清理资源
    //LOG_INFO("开始清理资源...\n");
    CloseHandle(hThread);
    CloseHandle(g_ServiceStopEvent);

    // 步骤8: 报告服务已停止
    ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);

    // 停止日志
    StopLogger();
}

/*
📖 知识点：main/wmain - 程序的入口点判断
Windows服务程序需要判断是以服务模式还是控制台模式运行。
这是通过命令行参数实现的。
*/
int _tmain(int argc, TCHAR* argv[]) {
    //LOG_INFO("程序启动，参数个数: %d\n", argc);

    // 如果有参数，检查是否是安装/卸载命令
    if (argc > 1) {
        if (_tcsicmp(argv[1], _T("install")) == 0) {
            LOG_INFO("安装模式\n");
            LOG_INFO("=== 看门狗服务安装 ===\n");

            // 检查管理员权限（建议但不是必须）
            if (!IsUserAnAdmin()) {
                LOG_ERROR("警告：建议以管理员身份运行安装。\n");
            }

            if (InstallService()) {
                LOG_INFO("\n安装成功！\n");
                LOG_INFO("接下来可以:\n");
                LOG_INFO("1. 在服务管理器中启动服务\n");
                LOG_INFO("2. 使用命令行: sc start WatchdogService\n");
                LOG_INFO("3. 重启计算机，服务将自动启动\n");
            }
            else {
                LOG_ERROR("\n安装失败！\n");
            }
            return 0;
        }
        else if (_tcsicmp(argv[1], _T("uninstall")) == 0) {
            LOG_INFO("卸载模式\n");
            LOG_INFO("=== 看门狗服务卸载 ===\n");

            if (IsUserAnAdmin()) {
                if (UninstallService()) {
                    LOG_INFO("\n卸载成功！\n");
                }
                else {
                    LOG_INFO("\n卸载失败！\n");
                }
            }
            else {
                LOG_ERROR("请以管理员身份运行卸载命令。\n");
            }
            return 0;
        }
        else if (_tcsicmp(argv[1], _T("status")) == 0) {
            LOG_INFO("状态查询模式\n");
            LOG_INFO("=== 看门狗服务状态 ===\n");
            ShowServiceStatus();
            return 0;
        }
        else if (_tcsicmp(argv[1], _T("debug")) == 0) {
            LOG_INFO("调试模式 - 以控制台程序运行\n");
            LOG_INFO("=== 看门狗服务调试模式 ===\n");
            LOG_INFO("按Ctrl+C停止程序\n\n");

            // 模拟SCM调用ServiceMain
            ServiceMain(0, NULL);
            return 0;
        }
        else if (_tcsicmp(argv[1], _T("run")) == 0) {
            LOG_INFO("直接运行模式（测试SCM启动）\n");
            // 直接进入服务模式（模拟被SCM启动）
            // 这仍然会失败，但错误信息更清晰
        }
        else {
            LOG_INFO("未知参数: %s\n", reinterpret_cast<char*>(argv[1]));
            LOG_INFO("可用参数:\n");
            LOG_INFO("  install     - 安装服务\n");
            LOG_INFO("  uninstall   - 卸载服务\n");
            LOG_INFO("  status      - 查看服务状态\n");
            LOG_INFO("  debug       - 调试模式运行\n");
            LOG_INFO("  run         - 直接运行（测试）\n");
            return 1;
        }
    }

    // 如果没有参数或以服务模式启动
    LOG_INFO("服务模式启动\n");

    // 服务入口点表
    SERVICE_TABLE_ENTRY ServiceTable[] = {
        {(PWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
        {NULL, NULL}
    };

    /*
    📖 关键API：StartServiceCtrlDispatcher - 服务的"报到"函数
    这个调用告诉SCM："我是服务程序，这是我的入口函数表"。
    调用后，程序进入服务模式，SCM会在适当时候调用ServiceMain。

    这是整个服务程序中最关键的调用，没有它程序就是普通控制台程序。
    */
    if (!StartServiceCtrlDispatcher(ServiceTable)) {
        DWORD error = GetLastError();
        LOG_ERROR("StartServiceCtrlDispatcher失败! 错误: %d\n", error);

        // 常见错误：不是从SCM启动（例如直接双击运行）
        if (error == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
            LOG_ERROR("错误：请以服务方式启动本程序\n");
            LOG_ERROR("使用方法:\n");
            LOG_ERROR("  安装服务: WatchdogService.exe install\n");
            LOG_ERROR("  调试运行: WatchdogService.exe debug\n");
            LOG_ERROR("  卸载服务: WatchdogService.exe uninstall\n");
        }

        return error;
    }

    return 0;
}

void StartLogger()
{
    // 初始化日志系统
    Logger:: Init("./logs", "info");

    // 使用日志
    LOG_INFO("WatchdogService started");
    LOG_DEBUG("Debug message example");
    LOG_WARN("Warning message example");
}

void StopLogger()
{
    // 关闭日志系统
    Logger::Shutdown();
}
