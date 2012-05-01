#!/bin/sh
export PLATFORM=mingw32
export PREFIX=i486-mingw32
export CC=${PREFIX}-gcc
export WINDRES=${PREFIX}-windres
exec make $*
