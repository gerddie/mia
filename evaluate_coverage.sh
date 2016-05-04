#!/bin/sh 

geninfo . -b ../

# remove system library files 
for f in $(find . -name "*.info"); do lcov --remove $f "/usr*" -o $f ; done

# remove the tools tree because it is not under test (yet) 
rm $(find src -name "*.info")

# generate the html report, note that genhtml may have some problems with a few files
# that will currently need to be removed manually
genhtml -o test-coverage -t "Mia coverage" --num-spaces 2 $(find . -name "*.info")



