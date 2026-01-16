# 自动生成版本信息
function(generate_version_info target)
    # 从Git获取版本信息
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --always
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else()
        set(GIT_VERSION "unknown")
        set(GIT_BRANCH "unknown")
    endif()
    
    # 生成版本头文件
    configure_file(
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../include/version.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/include/version.h
        @ONLY
    )
    
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
endfunction()