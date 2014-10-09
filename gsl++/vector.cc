/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gsl++/vector.hh>
#include <gsl++/vector_template.cxx>

namespace gsl {

template class TVector<double>; 
template class EXPORT_GSL TVector<float>; 

template class EXPORT_GSL TVector<long>; 
template class EXPORT_GSL TVector<int>; 
template class EXPORT_GSL TVector<short>; 
template class EXPORT_GSL TVector<char>; 

template class EXPORT_GSL TVector<ulong>; 
template class EXPORT_GSL TVector<uint>; 
template class EXPORT_GSL TVector<ushort>; 
template class EXPORT_GSL TVector<uchar>; 

#ifdef NDEBUG 
	class CTurnOffErrorHandler {
	public: 
		CTurnOffErrorHandler(); 
	}; 


	CTurnOffErrorHandler::CTurnOffErrorHandler() 
	{
		gsl_set_error_handler_off (); 
	}
		
	const CTurnOffErrorHandler gsl_turn_off_error_handler; 		
#endif 

}
