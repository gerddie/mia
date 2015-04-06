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
# check existence of _Complex type (c++0x)
#

CHECK_CXX_SOURCE_COMPILES(
"#include <complex>
#include <complex.h>
int main(int argc, char *args[]) 
{
	std::complex<double> c(2.0, 1.0); 
	double _Complex *a = reinterpret_cast<double _Complex *>(&c); 
	std::complex<double> b(*a); 
	return 0; 
}
" CXX_HAS_COMPLEX_C_TYPE)