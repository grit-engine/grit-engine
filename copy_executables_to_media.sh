#!/bin/bash

if [ ! -d media ] ; then
    echo 'You first need to clone the game directory (media) from SVN:' >&2
    echo 'svn clone https://svn.code.sf.net/p/gritengine/code media' >&2
    exit 1
fi

STRIP="strip -v --strip-all"

${STRIP} grit -o media/Grit.linux.x86_64
${STRIP} gsl -o media/Tools/gsl.linux.x86_64
${STRIP} extract -o media/Tools/extract.linux.x86_64
${STRIP} grit_col_conv -o media/Tools/extract.linux.x86_64
${STRIP} GritXMLConverter -o media/Tools/GritXMLConverter.x86_64
