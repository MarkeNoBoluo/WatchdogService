#!/bin/bash

# 看门狗服务构建脚本

BUILD_TYPE=${1:-Release}
BUILD_DIR="build/${BUILD_TYPE}"

echo "看门狗服务构建脚本"
echo "构建类型: ${BUILD_TYPE}"
echo

# 创建构建目录
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}" || exit 1

# 配置
echo "正在配置 ${BUILD_TYPE} 版本..."
cmake ../.. -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

# 构建
echo "正在构建..."
cmake --build . --config "${BUILD_TYPE}"

echo "构建完成！"
echo "输出文件在: ${BUILD_DIR}/bin/"