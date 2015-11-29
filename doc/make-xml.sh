#!/bin/sh 
#
# Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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


name="$1"
EXE_PREFIX="$2"
outdir="$4"

MIA_PLUGIN_TESTPATH="$3" ./mia-${name} --help-xml >${outdir}/mia-${name}.xml.help && \
   mv  ${outdir}/mia-${name}.xml.help ${outdir}/mia-${name}.xml

