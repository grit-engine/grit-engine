#!/bin/bash

if [ ! -d media ] ; then
    echo 'You first need to clone the game directory (media) from SVN:' >&2
    echo 'svn clone https://svn.code.sf.net/p/gritengine/code media' >&2
    exit 1
fi

cp -v grit media/Grit.linux.x86_64
cp -v gsl media/Tools/gsl.linux.x86_64
cp -v extract media/Tools/extract.linux.x86_64
cp -v grit_col_conv media/Tools/extract.linux.x86_64
cp -v GritXMLConverter media/Tools/GritXMLConverter.x86_64
