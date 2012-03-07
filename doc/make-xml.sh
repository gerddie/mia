#!/bin/sh 

name="$1"
ldpath="$2"
MIA_PLUGIN_PATH="$3"
outdir="$4"

if [ "x$LD_LIBRARY_PATH" = "x" ] ; then 
    LD_LIBRARY_PATH=${ldpath}
else
    LD_LIBRARY_PATH=${ldpath}:$LD_LIBRARY_PATH
fi

./mia-${name} --help-xml >${outdir}/mia-${name}.xml
