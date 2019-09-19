#!/bin/sh

#初始变量设定
set -x

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-../build}
#上面这句的意思是将建立的目录放在当前目录(-)的上一层目录(..)的build文件下。
BUILD_TYPE=${BUILD_TYPE:-release}

mkdir -p $BUILD_DIR/$BUILD_TYPE \
    && cd $BUILD_DIR/$BUILD_TYPE \
    && cmake \
            -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
            $SOURCE_DIR \
    && make $*

#build.sh是一个shell脚本
