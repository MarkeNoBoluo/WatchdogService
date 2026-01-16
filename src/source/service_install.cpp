#include "service_install.h"
#include "watchdog_service.h"
#include <tchar.h>
#include <stdio.h>
#include <shellapi.h>

/*
📖 知识点：Windows服务安装的关键步骤
1. 打开SCM管理器（OpenSCManager）
2. 创建服务（CreateService）
3. 配置服务参数（启动类型、账户等）
4. 关闭句柄
*/
BOOL InstallService() {
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	TCHAR szPath[MAX_PATH];
	DWORD dwError = 0;

	// 获取当前可执行文件路径
	if(!GetModuleFileName(NULL, szPath, MAX_PATH)) {
		dwError = GetLastError();
        // //LOG_ERROR(L"获取模块文件名失败，错误码: %d\n", dwError);
		return FALSE;
	}
	// 步骤1: 打开SCM管理器
	// 📖 关键点：需要管理员权限才能打开SCM
	schSCManager = OpenSCManager(
		NULL,						// 本地计算机
		NULL,						// SERVICES_ACTIVE_DATABASE
		SC_MANAGER_CREATE_SERVICE	// 创建服务权限
	);
	if(schSCManager == NULL) {
		dwError = GetLastError();
		if(dwError == ERROR_ACCESS_DENIED) {
            //LOG_ERROR(L"打开SCM失败，权限被拒绝。请以管理员身份运行此程序。\n");
		} else {
            //LOG_ERROR(L"打开SCM失败，错误码: %d\n", dwError);
		}
		return FALSE;
	}
    //LOG_DEBUG(L"成功打开SCM管理器,正在创建服务。。。\n");

	// 步骤2: 创建服务
	schService = CreateService(
		schSCManager,               // SCM句柄
		SERVICE_NAME,               // 服务名称
		SERVICE_DISPLAY_NAME,       // 显示名称
		SERVICE_ALL_ACCESS,         // 完全访问权限
		SERVICE_WIN32_OWN_PROCESS,  // 服务类型
		SERVICE_DEMAND_START,       // 启动类型：按需启动
		SERVICE_ERROR_NORMAL,       // 错误控制类型
		szPath,                     // 可执行文件路径
		NULL,                       // 无加载顺序组
		NULL,                       // 无标签标识符
		NULL,                       // 使用默认依赖项
		NULL,                       // 以本地系统账户运行
		NULL                        // 无密码
	);

	if(schService == NULL) {
		dwError = GetLastError();
		if(dwError == ERROR_SERVICE_EXISTS) {
            //LOG_ERROR(L"服务已存在。\n");
		} else {
            //LOG_ERROR(L"创建服务失败，错误码: %d\n", dwError);
			CloseServiceHandle(schSCManager);
			return FALSE;
		}
	}else {
        //LOG_INFO(L"服务创建成功！\n");
	}

	// 步骤3: 关闭句柄
	if(schService) {
		CloseServiceHandle(schService);
	}
	if(schSCManager) {
		CloseServiceHandle(schSCManager);
	}	

    //LOG_INFO(L"服务安装完成。\n");
    //LOG_INFO(L"您可以通过服务管理器启动服务，或使用命令：net start %s\n", SERVICE_NAME);

	return TRUE;
	
}

BOOL UninstallService() {
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ssStatus;
	BOOL bSuccess = FALSE;

    //LOG_INFO(L"正在卸载服务...\n");

	// 步骤1: 打开SCM管理器
	schSCManager = OpenSCManager(
		NULL,                       // 本地计算机
		NULL,                       // SERVICES_ACTIVE_DATABASE
		SC_MANAGER_CONNECT          // 连接权限
	);

	if(schSCManager == NULL) {
        //LOG_ERROR(L"打开SCM失败，错误码: %d\n", GetLastError());
		return FALSE;
	}
	// 步骤2: 打开服务
	schService = OpenService(
		schSCManager,               // SCM句柄
		SERVICE_NAME,               // 服务名称
		DELETE | SERVICE_STOP | SERVICE_QUERY_STATUS // 所需权限
	);
	if(schService == NULL) {
		DWORD dwError = GetLastError();
		if (dwError == ERROR_SERVICE_DOES_NOT_EXIST) {
            //LOG_ERROR(L"服务不存在。\n");
			CloseServiceHandle(schSCManager);
			return TRUE;  // 算是成功，本来就不存在
		}
        //LOG_ERROR(L"打开服务失败！错误: %lu\n", dwError);
		CloseServiceHandle(schSCManager);
		return FALSE;
	}

	// 步骤3: 停止服务（如果正在运行）
	if(ControlService(schService, SERVICE_CONTROL_STOP, &ssStatus)) {
        //LOG_INFO(L"正在停止服务...\n");
		Sleep(1000);
		// 等待服务停止
		while (QueryServiceStatus(schService, &ssStatus)) {
			if (ssStatus.dwCurrentState == SERVICE_STOPPED) {
                //LOG_INFO(L"服务已停止。\n");
				break;
			}
            //LOG_INFO(L"等待服务停止...\n");
			Sleep(1000);
		}
	} else {
		DWORD dwError = GetLastError();
		if (dwError == ERROR_SERVICE_NOT_ACTIVE) {
            //LOG_ERROR(L"服务未运行。\n");
		} else {
            //LOG_ERROR(L"发送停止命令失败！错误: %lu\n", dwError);
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return FALSE;
		}
	}

	// 步骤4: 删除服务
	if(DeleteService(schService)) {
        //LOG_INFO(L"服务已成功卸载。\n");
		bSuccess = TRUE;
	} else {
		DWORD dwError = GetLastError();
		if (dwError == ERROR_SERVICE_MARKED_FOR_DELETE) {
            //LOG_ERROR(L"服务已标记为删除，将在重启后移除。\n");
			bSuccess = TRUE;
		}
		else {
            //LOG_ERROR(L"删除服务失败！错误: %lu\n", dwError);
		}
	}

	// 步骤5: 关闭句柄
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return bSuccess;
}

void ShowServiceStatus() {
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwBytesNeeded;

	schSCManager = OpenSCManager(
		NULL,
		NULL,
		SC_MANAGER_CONNECT
	);
	if(schSCManager == NULL) {
		DWORD dwError = GetLastError();
		if (dwError == ERROR_SERVICE_DOES_NOT_EXIST) {
            //LOG_ERROR(L"服务未安装。\n");
		}
		else {
            //LOG_ERROR(L"打开服务失败。\n");
		}
		CloseServiceHandle(schSCManager);
		return;
	}

	if(QueryServiceStatusEx(
		schService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssStatus,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded
	)) {
        //LOG_INFO(L"服务状态:\n");
        //LOG_INFO(L"  显示名称: %s\n", SERVICE_DISPLAY_NAME);
        //LOG_INFO(L"  当前状态: %s\n",
            // (ssStatus.dwCurrentState == SERVICE_RUNNING) ? "运行中" :
            // (ssStatus.dwCurrentState == SERVICE_STOPPED) ? "已停止" :
            // (ssStatus.dwCurrentState == SERVICE_START_PENDING) ? "启动中" :
            // (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) ? "停止中" :
            // (ssStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ? "暂停中" :
            // (ssStatus.dwCurrentState == SERVICE_PAUSED) ? "已暂停" : "未知状态");
        //LOG_INFO(L"  进程ID: %lu\n", ssStatus.dwProcessId);
        //LOG_INFO(L"  检查点: %lu\n", ssStatus.dwCheckPoint);
        //LOG_INFO(L"  等待提示: %lu\n", ssStatus.dwWaitHint);
	} else {
        //LOG_ERROR(L"查询服务状态失败。\n");
	}
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}
