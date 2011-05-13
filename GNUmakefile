.PHONY: Default Release Debug 

CXXFLAGS=-std=c++0x

all: Default

Default: build build/CMakeCache.txt
	cd build && make -j 7 

build/CMakeCache.txt: build 
	cd build && cmake -DCMAKE_CXXFLAGS="$(CXXFLAGS)" ..

build:
	[ -d build ] || mkdir -p build 	

Release: build-Release build-Release/CMakeCache.txt
	cd build-Release
	make -j 7 


Debug: build-Debug 
	cd build-Debug
	make -j 7 


build-Debug:
	mkdir -p $@
	cd $@ 
	CXXFLAGS=$(CXXFLAGS) cmake -DCMAKE_BUILD_TYPE=Debug .. 


build-Release:
	mkdir -p $@
	cd $@ 
	CXXFLAGS=$(CXXFLAGS) cmake -DCMAKE_BUILD_TYPE=Release .. 

