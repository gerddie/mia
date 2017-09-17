#!/bin/sh

if [ -e Makefile ]; then 
	make -j8 && make test
elif [-e rules.ninja ]; then 
	ninja && ninja test
fi

if [ "x$?" = "x0" ]; then 

    lcov --base-directory . --directory . -c -o mia.info

    # remove system library files 
    lcov --remove mia.info "/usr*" -o mia.info
    lcov --remove mia.info "mia2/src*" -o mia.info

    # generate the html report, note that genhtml may have some problems with a few files
    # that will currently need to be removed manually
    rm -rf test-coverage

    genhtml -o test-coverage -t "Mia coverage" --num-spaces 2 mia.info
else
    echo build or test failed 
fi
