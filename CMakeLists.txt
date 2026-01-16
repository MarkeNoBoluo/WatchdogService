cmake_minimum_required(VERSION 3.16.1) # 最低支持CMAKE版本
project(WatchDogService # 项目基础信息
    VERSION 1.0
    DESCRIPTION "看门狗系统服务"
    LANGUAGES CXX)
# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 项目全局配置
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)

# 设置系统环境
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PLATFORM_FLODER "windows_x64")
    else()
        set(PLATFORM_FLODER "windows_x86")
    endif()
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_FLODER "kylin_v10_sp1")
else()
    message(FATAL_ERROR "Platform detection failed! Please check platform conditions.")
endif()

# 设置输出路径
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OUT_DIR "${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FLODER}/Debug")
else()
    set(OUT_DIR "${CMAKE_SOURCE_DIR}/bin/${PLATFORM_FLODER}/Release")
endif()

# 输出目录配置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUT_DIR})


# 包含自定义模块
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 根据平台配置
if(WIN32)
    # Windows SDK版本控制（可选）
        # set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.22621.0")

        # Unicode支持
        add_definitions(-DUNICODE -D_UNICODE)

        # 安全编译选项
        add_compile_options(
            "$<$<CXX_COMPILER_ID:MSVC>:/W4>"           # 警告级别4
            "$<$<CXX_COMPILER_ID:MSVC>:/WX>"           # 视警告为错误
            "$<$<CXX_COMPILER_ID:MSVC>:/sdl>"          # 安全开发生命周期检查
            "$<$<CXX_COMPILER_ID:MSVC>:/permissive->"  # 严格标准符合
            "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>"  # GCC/Clang所有警告
            "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra>"
            "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>"
        )

        # MSVC特定设置
        if(MSVC)
            # 运行时库配置
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

            # 调试信息格式
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                add_compile_options(/Zi /DEBUG)  # 生成PDB文件
            endif()

            # 关闭特定警告
            add_compile_options(
                /wd4251  # class 'type' needs to have dll-interface...
                /wd4275  # non dll-interface class used as base...
                /wd4996  # 不安全函数警告（使用strsafe.h替代）
            )
        endif()
else()
    # Linux/macOS设置
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    add_compile_options(-fPIC)
endif()


# 添加子目录
add_subdirectory(src)

# 测试（可选）
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 安装规则
include(GNUInstallDirs)
install(TARGETS ${PROJCET_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# 安装文档和示例
install(DIRECTORY docs/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY examples/ DESTINATION ${CMAKE_INSTALL_DATADIR}/examples)
