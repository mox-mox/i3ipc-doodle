#!/bin/bash

if [ -d "build" ]; then
	echo "Build directory already exists. Aborting..."
	exit 1
fi

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
