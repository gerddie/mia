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
# extract the boundary between masks 
# params: tissue_1_files  tissue_2_files outfile
# example: 
#       extract_boundary.sh  enamel/e000.png dentine/d000.png de-boundary
#


TMPDIR="/tmp/segment-$$"
mkdir -p $TMPDIR
mkdir -p $TMPDIR/a
mkdir -p $TMPDIR/b

mia-2dstackfilter -i $1 -o $TMPDIR/a/ -t png dilate:shape=n6 -V message
mia-2dstackfilter -i $2 -o $TMPDIR/b/ -t png dilate:shape=n6 -V message

pushd $TMPDIR/a
files=`ls *` 
popd

for f in $files; do 
    echo $f 
    mia-2dbinarycombine -1 $TMPDIR/a/$f -2 $TMPDIR/b/$f -o $3 -p and 
done 

rm -fr $TMPDIR
