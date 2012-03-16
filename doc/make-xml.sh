#!/bin/sh 

name="$1"
EXE_PREFIX="$2"
outdir="$4"

MIA_PLUGIN_TESTPATH="$3" ./mia-${name} --help-xml >${outdir}/mia-${name}.xml.help && \
   mv  ${outdir}/mia-${name}.xml.help ${outdir}/mia-${name}.xml

