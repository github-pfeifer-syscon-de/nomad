#!/bin/bash
# 
# only the wia_lh.h is used
widl -Oi --win64 -m64 -I ${MINGW_PREFIX}/include/ wia_lh.idl
#cp wia_lh.h ${MINGW_PREFIX}/include
#cp wia_lh.idl ${MINGW_PREFIX}/include/
