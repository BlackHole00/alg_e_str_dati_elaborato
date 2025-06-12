#!/bin/sh

set -e

if [ ! -d build ]; then
	mkdir build

	cd build
	cmake .. -DCMAKE_BUILD_TYPE=Release
	cd ..
fi

cmake --build build --config Release

./build/counting_sort
./build/quick_sort
./build/quick_sort_3way
./build/intro_sort

platform_specific_dir="$(whoami)_$(uname)_$(uname -m)"
if [ ! -d "./results/$platform_specific_dir" ]; then
	mkdir "./results/$platform_specific_dir"
fi
cp ./results/*.csv ./results/$platform_specific_dir

