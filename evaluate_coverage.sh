#!/bin/sh 

mkdir build-coverage
cd build-coverage

cmake -DALWAYS_CREATE_DOC=OFF -DSTRICT_DEPENDECIES=ON \
      -DMIA_CREATE_MANPAGES=OFF -DMIA_CREATE_NIPYPE_INTERFACES=OFF \
      -DENABLE_COVERAGE=ON -DDISABLE_PROGRAMS=ON -DUSE_MATHJAX=YES \
      -G Ninja ..

ninja

lcov --base-directory . --directory . --zerocounters -q

ninja test

lcov --base-directory . --directory . -c -o mia.info

# remove system library files 
lcov --remove mia.info "/usr*" -o mia.info

# generate the html report, note that genhtml may have some problems with a few files
# that will currently need to be removed manually
genhtml -o test-coverage -t "Mia coverage" --num-spaces 2 mia.info




