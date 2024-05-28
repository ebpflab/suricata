#!/bin/bash

LIB_NAME=edpi
LIB_DIR=`pwd`/$LIB_NAME
VERSION=1.0.1
DEBUG_VER=
#"--enable-debug"
#DBG_CFLAGS+="gcc -g -O0"
rm $LIB_NAME -rf
mkdir $LIB_NAME

./autogen.sh
./configure --enable-isolated --enable-rdkafka --enable-unix-socket=no  --libdir=$LIB_DIR/lib --includedir=$LIB_DIR/include $DEBUG_VER

cd libhtp && ./configure CFLAGS="-DHAVE_STRLCAT -DHAVE_STRLCPY" $DEBUG_VER  && cd ..

make
make install-lib

strip $LIB_DIR/lib/libedpi.so.$VERSION
mkdir -p $LIB_DIR/include
cd src && cp edpi.h $LIB_DIR/include -rf && cd ..

cp $LIB_DIR/lib/libedpi.so /lib/x86_64-linux-gnu/
cp $LIB_DIR/lib/libedpi.so.$VERSION /lib/x86_64-linux-gnu/
