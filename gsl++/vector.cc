/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010 Gert Wollny 
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <gsl++/vector.hh>
#include <gsl++/vector_template.cxx>

namespace gsl {

	template class TVector<double>; 
	template class TVector<float>; 
	
	template class TVector<long>; 
	template class TVector<int>; 
	template class TVector<short>; 
	template class TVector<char>; 
	
	template class TVector<ulong>; 
	template class TVector<uint>; 
	template class TVector<ushort>; 
	template class TVector<uchar>; 

}
