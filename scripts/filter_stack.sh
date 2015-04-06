#!/bin/sh 
#
#
# This file is part of MIA - a toolbox for medical image analysis 
# Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
#
# MIA is free software; you can redistribute it and/or modify
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
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#

# $1 input path 
# $2 output path

median_width=3
mlv_width=4

mkdir -p $2
cd $2

number_pattern=`eva-filenumber-pattern $1`

if [ ! -e thresh.done ] ; then 
    thresh=`eva-multihisto -i $1 -V message -o pre_histo.dat`
    mkdir -p bg_thresh
    echo bg_thresh
    eva-2dimagefilterstack -i $1 -t png -o bg_thresh/bg thresh:min=$thresh -V message && touch bg_thresh.done
fi

if [ ! -e filtered.done ] ; then 
    mkdir -p filtered 
    echo stack-process
    eva-2dstackfilter -i bg_thresh/bg${number_pattern}.png -o filtered/f median:w=$median_width mlv:w=$mlv_width -V message && touch filtered.done
fi
