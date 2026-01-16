# 编译器警告配置模块
function(set_target_warnings target)
    option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
    
    set(MSVC_WARNINGS
        /W4     # 警告级别4
        /w14242 # 转换可能丢失数据
        /w14254 # 浮点转换可能丢失数据
        /w14263 # 成员函数不覆盖基类虚函数
        /w14265 # 类有虚函数但析构函数非虚
        /w14287 # 无符号/有符号比较
        /we4289 # 非标准扩展使用
        /w14296 # 表达式总是true/false
        /w14311 # 指针截断
        /w14545 # 表达式前有未求值的表达式
        /w14546 # 函数调用前有未求值的函数
        /w14547 # 运算符前有未求值的运算符
        /w14549 # 运算符前有未求值的表达式
        /w14555 # 表达式没有副作用
        /w14619 # pragma warning: 警告编号
        /w14640 # 启用线程安全检查
        /w14826 # 转换到更大类型，可能符号扩展
        /w14905 # 宽字符串文字到char*
        /w14906 # 字符串文字到char*
        /w14928 # 非法的复制初始化
    )
    
    set(CLANG_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )
    
    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )
    
    if(WARNINGS_AS_ERRORS)
        set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
        set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
        set(GCC_WARNINGS ${GCC_WARNINGS} -Werror)
    endif()
    
    if(MSVC)
        target_compile_options(${target} PRIVATE ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        target_compile_options(${target} PRIVATE ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE ${GCC_WARNINGS})
    endif()
endfunction()