#!/bin/bash

g++ -Wall -Wextra -std=c++11 -I /usr/local/include -L /usr/local/lib glsl_check.cpp -o glsl_check -lGL -lglfw3 -lX11 -lXcursor -lXrandr -lXi -lXxf86vm -lpthread -ldl -lXinerama
