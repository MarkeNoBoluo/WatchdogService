@echo off
setlocal enabledelayedexpansion

echo 看门狗服务构建脚本
echo.

REM 检查构建类型
if "%1"=="" (
    set BUILD_TYPE=Release
) else (
    set BUILD_TYPE=%1
)

REM 创建构建目录
if not exist "build\%BUILD_TYPE%" mkdir "build\%BUILD_TYPE%"

REM 配置
echo 正在配置%BUILD_TYPE%版本...
cd build\%BUILD_TYPE%
cmake ../.. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -G "Visual Studio 17 2022" -A x64

REM 构建
echo 正在构建...
cmake --build . --config %BUILD_TYPE%

echo 构建完成！输出文件在：build/%BUILD_TYPE%/bin/%BUILD_TYPE%/
pause