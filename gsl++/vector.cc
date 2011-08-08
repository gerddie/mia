/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <gsl++/vector.hh>
#include <gsl++/vector_template.cxx>
#include <gsl++/gsldefines.hh>

namespace gsl {

	template class EXPORT_GSL TVector<double>; 
	template class EXPORT_GSL TVector<float>; 
	
	template class EXPORT_GSL TVector<long>; 
	template class EXPORT_GSL TVector<int>; 
	template class EXPORT_GSL TVector<short>; 
	template class EXPORT_GSL TVector<char>; 
	
	template class EXPORT_GSL TVector<ulong>; 
	template class EXPORT_GSL TVector<uint>; 
	template class EXPORT_GSL TVector<ushort>; 
	template class EXPORT_GSL TVector<uchar>; 

}
