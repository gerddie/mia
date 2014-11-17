#!/bin/sh 

miadoctoolroot=@MIA_DOCTOOLS_ROOT@

$miadoctoolroot/miaxml2nipype.py -i $1 -o $2
