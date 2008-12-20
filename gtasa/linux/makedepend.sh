#!/bin/bash

makedepend -Y -f ../src/common.mk -I ../../grit_core/src ../src/*.c++

sed -i 's_^[.][.]/src/__g' ../src/common.mk
