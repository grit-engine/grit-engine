#!/bin/bash

makedepend -Y -f ../src/common.mk ../src/{,linux,win32}/*.c++

sed -i 's_^[.][.]/src/\(linux\|win32\)\?/__g' ../src/common.mk

