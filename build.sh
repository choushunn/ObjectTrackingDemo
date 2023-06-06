#!/bin/bash

# 判断build文件夹是否存在，不存在则创建
if [ ! -d "build" ]; then
  mkdir build
fi

# 进入build文件夹
cd build

# 执行cmake和make命令
cmake ..
make -j$(nproc)
