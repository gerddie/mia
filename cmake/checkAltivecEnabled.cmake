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
#
#

#
# check if the build settings support ALTIVEC
#

CHECK_CXX_SOURCE_COMPILES(
"
#ifdef __POWERPC__
#include <altivec.h>
#else
#error Altivec not supported
#endif

int main(int argc, char *args[]) 
{
       return 0; 
}
" BUILD_HAS_ALTIVEC_ENABLED)
