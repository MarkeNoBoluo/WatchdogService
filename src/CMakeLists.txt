# 包含目录设置
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include  # 用于生成的头文件
)

# 源文件分组
set(SOURCES
    source/watchdog_service.cpp
    source/service_install.cpp

)

set(HEADERS
    include/watchdog_service.h
    include/service_install.h

)

# 添加可执行目标
add_executable(WatchdogService ${SOURCES} ${HEADERS})

# 设置目标属性
set_target_properties(WatchdogService PROPERTIES
    OUTPUT_NAME "WatchdogService"
    WIN32_EXECUTABLE TRUE        # Windows GUI程序（虽然是服务，但需要控制台调试）
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Windows服务需要Advapi32库
target_link_libraries(WatchdogService
    PRIVATE
        Advapi32     # Windows服务API (RegisterServiceCtrlHandler等)
        User32       # Windows用户界面API（可选，用于消息框等）
        Shell32      # Shell API（如果需要Shell操作）
)

# MSVC特定链接选项
if(MSVC)
    target_link_options(WatchdogService
        PRIVATE
            /SUBSYSTEM:CONSOLE   # 先使用控制台子系统便于调试
            /MANIFEST:NO         # 我们稍后手动配置manifest
    )

    # 设置manifest文件（用于UAC和兼容性）
    set_target_properties(WatchdogService PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        VS_USER_PROPS "${CMAKE_CURRENT_SOURCE_DIR}/WatchdogService.props"
    )
endif()

