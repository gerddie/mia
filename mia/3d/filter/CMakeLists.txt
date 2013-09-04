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

SET(filters3dNew 	    
  binarize 
  bandpass
  combiner
  convert 
  crop 
  downscale 
  gradnorm
  growmask
  invert
  kmeans 
  label
  load
  mask
  median 
  mlv 
  morphological 
#  reorient 
  resize
  scale
  seededwatershed
  selectbig
  sepconv 
  tee
  thinning
  transform
  watershed
  )

PLUGINGROUP_WITH_TEST_AND_PREFIX2("3dimage" "filter" "${filters3dNew}" 
  "${MIA3DLIBS}" TESTLIBS  mia3dtest
  )


