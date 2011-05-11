#!/bin/sh 
#
# Copyright (c) Leipzig, Madrid 2005-2011
#
# Max-Planck-Institute of Evolutionary Anthropologie
# BIT, ETSI Telecomunicacion, UPM
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
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
# $1 input path 
# $2 output path

mkdir -p $2
cd $2

number_pattern=`mia-filenumberpattern -i $1`


if [ ! -e bg_thresh.done ] ; then 
    thresh=`mia-multihist -i $1  -o pre_histo.dat`
    echo $thresh 
    mkdir -p bg_thresh 
    echo bg_thresh
    mia-2dimagefilterstack -i $1 -t png -o bg_thresh/bgt thresh:thresh=$thresh -V message && touch bg_thresh.done
fi

if [ ! -e filtered.done ] ; then 
    mkdir -p filtered 
    echo stack-process
    mia-2dstackfilter -t png -i bg_thresh/bgt${number_pattern}.png -o filtered/f median:w=3 mlv:w=4 -V message && touch filtered.done
fi

if [ ! -e histo.dat ] ; then 
    mia-multihist -i filtered/f${number_pattern}.png -o histo.dat -V message
fi

if [ ! -e cmeans.done ] ; then 
    mia-cmeans -i histo.dat -c 0,120,150 -V message -k 0.01 -o cmeans.txt && touch cmeans.done
fi

if [ ! -e enamel.done ] ; then 
    mkdir -p enamel
    echo regingrow
    mia-2dstackfilter -t png -i filtered/f${number_pattern}.png -o enamel/enamel regiongrow:map=cmeans.txt,seed=0.99,low=0.65,class=2 -V message && touch enamel.done
fi

if [ ! -e background.done ] ; then 
    mkdir -p background
    echo regingrow
    mia-2dstackfilter -t png -i filtered/f${number_pattern}.png -o background/bg regiongrow:map=cmeans.txt,seed=0.99,low=0.65,class=0 -V message && touch background.done
fi

if [ ! -e dentine.done ] ; then 
    mkdir dentine
    cd enamel
    filenames=`ls *.png`
    cd ..
    for f in $filenames ; do 
	    fbase=`echo $f | sed -s "s/enamel//"`
	    mia-2dbinarycombine -p nor -1 enamel/enamel${fbase} -2 background/bg${fbase} -o dentine/dentine${fbase}
    done
    touch dentine.done
fi
   
