#!/bin/sh 

geninfo . -b ../
for f in $(find . -name "*.info"); do lcov --remove $f "/usr*" -o $f ; done
rm $(find src -name "*.info")
genhtml -o test-coverage -t "Mia coverage" --num-spaces 2 $(find . -name "*.info")


