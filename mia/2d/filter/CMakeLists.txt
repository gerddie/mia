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

IF (FFTWF_FOUND AND WITH_FFTWF) 
  SET(filters2d_fftNew cst fft)
ENDIF (FFTWF_FOUND AND WITH_FFTWF) 

SET (NOTHING
  classmap
  frequency
  harmmean
  ianiso
  kuwahara
  lnfft
  midpoint
  rgg
  variation
  wmean
  ws
)

SET(filters2dNew_base 
  adaptmed
  admean
  aniso
  bandpass
  binarize
  combiner
  convert
  crop
  distance
  downscale
  gradnorm
  invert
  kmeans
  label 
  labelmap
  labelscale
  load
  mask
  mean 
  median
  mlv
  morphological
  ngfnorm
  noise
  regiongrow
  scale
  seededwatershed
  selectbig
  sepconv
  shaped_mean
  sortlabel
  tee
  thinning 
  thresh
  tmean
  transform
  watershed
)

SET(filters2dNew ${filters2dNew_base} ${filters2d_fftNew})

PLUGINGROUP_WITH_TEST_AND_PREFIX2("2dimage" "filter" "${filters2dNew}" 
  "${MIA2DLIBS}" TESTLIBS  mia2dtest)

