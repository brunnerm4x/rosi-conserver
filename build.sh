#!/bin/bash

echo "INFO: This script expects qmake executable path set in qmake variable"

mkdir -p build

if [ ! -f build/.gitignore ]; then
  printf "build/\nbin/" | tee .gitignore
fi

cd build/
$qmake ../rosiConserver.pro
make

cd ../
mkdir -p bin
if [ -f bin/rosiConserver ]; then
	rm /bin/rosiConserver
fi

ln build/rosiConserver bin/rosiConserver

echo "Built and linked to bin/rosiConserver."
