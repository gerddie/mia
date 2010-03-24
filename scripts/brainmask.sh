#!/bin/sh 
#
# Copyright (c) Madrid 2008, 2009 
# BIT, ETSI Telecomunicacion, UPM
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

#
# call this program like: 
#
#  brainmash.sh <input-image> <output-image>
#

#supported file formats (all only gray scale)
# Analyze 
# inria 
# vff     
# vista   
# vtk 
print_usage() {
    echo Usage: 
    echo "  brainmash.sh <input-image> <output-image>"
    echo 
    exit 1
}

if [ "x$1" = "x" ]; then 
    echo Error: input file missing 
    print_usage
 
fi 

if [ "x$2" = "x" ]; then 
    echo Error: output file missing 
    print_usage
fi 

infile=$1 

outfile=$2 

#white matter probability \in [0,1]
wmprob=0.7

#erode shape for initial wm mask, 
#     possible values: 6n 18n 26n, [sphere:r=N] (N > 0)  
erode_shape=6n

#growmask shape, possible values see erode_shape
shape=18n

#intensity threshhold for mask growing 
thresh=30 

#final closing shape, possible values see erode_shape
close_shape=[sphere:r=3]

#final opening shape, possible values see erode_shape
open_shape=[sphere:r=3]

# segment the input image into 3 classes 
mia-fuzzysegment3d -i $infile -o b0.v -c cls.v -n 3 -V message && \
    mia-3dimageselect -i cls.v -o wmprob.v -n 2 && \
    mia-3dimagefilter -i wmprob.v -o $outfile \
       -V message \
       binarize:min=$wmprob \
       erode:shape=${erode_shape} \
       label \
       selectbig \
       growmask:ref=b0.v,min=${thresh},shape=${shape} \
       close:shape=${close_shape}\
       open:shape=${open_shape}\
       mask:input=b0.v || \
    echo "Error processing '$infile'"

#rm -f b0.v cls.v wmprob.v

   
