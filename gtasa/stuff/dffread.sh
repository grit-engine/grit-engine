#!/bin/sh

if [ "$1" = "" ] ; then
        echo "Usage: $0 <ides.txt> ..."
        exit 1
fi

IDES=`cat "$1" | sed "s/^\\(.*\\)$/-i '\1'/g"`


shift 1

for i in "$@" ; do
        IDES="$IDES '$i'" 
done
 
eval dffread $IDES


