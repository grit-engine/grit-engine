#!/bin/bash

devg++ -I /usr/local/include -L /usr/local/lib glsl_check.cpp -o glsl_check -lGL -lglfw -lX11 -lXcursor -lXrandr -lXi

