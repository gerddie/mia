#!/bin/bash 
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
# $3 temp path

temp_dir=/tmp/remove-small-blobs/


in_images="$1"
out_images="$2"
thresh=$3

mkdir -p $temp_dir


number_pattern=$(mia-filenumber-pattern "$1")

# label the images 
mia-2dstackfilter -i "$in_images" -t v -o "$temp_dir/labeled" "label:map=$temp_dir/labelmap.txt" 

#relabel joined labels 
mia-2dimagefilterstack -i "$temp_dir/labeled${number_pattern}.v" -o $temp_dir/relabeled -t v \
                        "relabel:map=$temp_dir/labelmap.txt"

# evaluate the histogram
mia-multihist -i "$temp_dir/relabeled${number_pattern}.v" -o "$temp_dir/labelcount.txt"


# this can be done with awk
echo "MiaLabelmap" > "$temp_dir/labelbinarizationmap.txt"
awk -v thresh=$thresh <"$temp_dir/labelcount.txt" >> "$temp_dir/labelbinarizationmap.txt" '{if ($2 > thresh){ print $1 " 1"}else{ print $1 " 0"} }'

mia-create-labelbinarization -i "$temp_dir/labelcount.txt" -o "$temp_dir/labelbinarizationmap.txt" \
                             --thresh ${min_blobsize}

mia-2dimagefilterstack -i "$temp_dir/relabeled${number_pattern}.v" -o "$out_images" \
                       "relabel:map=$temp_dir/labelbinarizationmap.txt" binarize:min=1

rm -rf $temp_dir


