CC=clang

INCLUDE=/opt/homebrew/Cellar/sdl2/2.32.0/include
LIB=/opt/homebrew/Cellar/sdl2/2.32.0/lib
SDL=SDL2

RELEASE=exec
SRC=main.c

$CC -o $RELEASE $SRC -I $INCLUDE -L $LIB -l $SDL

./$RELEASE