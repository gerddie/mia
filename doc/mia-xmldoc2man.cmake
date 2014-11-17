#!/bin/sh 

miadoctoolroot=@MIA_DOCTOOLS_ROOT@

$miadoctoolroot/miaxml2man.py $1 >$2
