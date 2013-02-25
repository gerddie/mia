#!/bin/sh 
#
#
# This file is part of MIA - a toolbox for medical image analysis 
# Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#
# extract the boundary between masks 
# params: tissue_1_files  tissue_2_files outfile
# example: 
#       extract_boundary.sh  enamel/e000.png dentine/d000.png de-boundary
#


TMPDIR="/tmp/segment-$$"
mkdir -p $TMPDIR
mkdir -p $TMPDIR/a
mkdir -p $TMPDIR/b

mia-2dstackfilter -i $1 -o $TMPDIR/a/ -t png dilate:shape=6n -V message
mia-2dstackfilter -i $2 -o $TMPDIR/b/ -t png dilate:shape=6n -V message

pushd $TMPDIR/a
files=`ls *` 
popd

for f in $files; do 
    echo $f 
    mia-2dbinarycombine -1 $TMPDIR/a/$f -2 $TMPDIR/b/$f -o $3$f -p and 
done 

rm -fr $TMPDIR
