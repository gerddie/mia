#!/bin/sh 

name="$1"
TEST_PREFIX="$2"
outdir="$4"

if [ "x$LD_LIBRARY_PATH" = "x" ] ; then 
    LD_LIBRARY_PATH=${TEST_PREFIX}/lib
else 
    LD_LIBRARY_PATH=${TEST_PREFIX}/lib:$LD_LIBRARY_PATH
fi
MIA_PLUGIN_TESTPATH="$3" ${TEST_PREFIX}/bin/mia-${name} --help-xml >${outdir}/mia-${name}.xml.help && \
   mv  ${outdir}/mia-${name}.xml.help ${outdir}/mia-${name}.xml

