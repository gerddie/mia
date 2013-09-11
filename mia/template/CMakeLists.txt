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

SET(MIA_TEMPLATES
bandpass.cxx
bandpass.hh
binarize.cxx
binarize.hh
combiner.hh
combiner_filter.hh
convert.cxx
convert.hh
cvd_io_trait.hh
dimtrait.hh
divcurl.cxx
divcurl.hh
filter_chain.hh
fullcost.cxx
fullcost.hh
invert.cxx
invert.hh
lsd.hh
mi.hh
multicost.cxx
multicost.hh
nonrigidregister.cxx
nonrigidregister.hh
normalize.hh
seededwatershed.hh
similarity_profile.cxx
similarity_profile.hh
ssd.hh
transformfactory.cxx
transformfactory.hh
trackpoint.hh
trackpoint.cxx
watershed.hh)

INSTALL(FILES ${MIA_TEMPLATES} DESTINATION ${INCLUDE_INSTALL_PATH}/mia/template)

