#!/bin/sh

DIR="$( dirname "$(readlink -f "$0")")"
cd $DIR

if [ ! -d "build" ]; then
	mkdir "build"
fi

cd build

	if [ ! -d "linux" ]; then
		mkdir "linux"
	fi

	cd linux

		compileFlags="-Wall -fno-rtti -fno-exceptions -Wno-write-strings -Wno-unused-function -Wno-unused-result -no-pie -pipe -march=x86-64 -o linux_main"

		debugCompileFlags="-O0 -g3 -D_DEBUG"
		releaseCompileFlags="-O3 -g0 -flto -ffast-math -mavx2 -DNDEBUG"
		#releaseCompileFlags="$releaseCompileFlags -fopt-info-vec-optimized"

		compileFlags="$debugCompileFlags $compileFlags"
		#compileFlags="$releaseCompileFlags $compileFlags"

		libs="-lstdc++ -lm -lX11 -lGL -lpthread"

		files="../../src/linux/linux_main.cpp"
		files="$files ../../src/shared/app.cpp"
		files="$files ../../src/shared/xmaths.cpp"

		linkFlags="-Map=build.map -Wno-undef"

		gcc $compileFlags $files $libs -Xlinker $linkFlags