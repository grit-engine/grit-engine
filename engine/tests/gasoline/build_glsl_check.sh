#!/bin/bash

g++ -Wall -Wextra -std=c++11 glsl_check.cpp -o glsl_check -lGL `pkg-config --cflags --libs glfw3` -lX11 -lXcursor -lXrandr -lXi -lXxf86vm -lpthread -ldl -lXinerama
