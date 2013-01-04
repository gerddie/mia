/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 David Paster, Gert Wollny
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

#include <mia/2d/trackpoint.hh>
#include <mia/template/cvd_io_trait.hh>
#include <mia/template/trackpoint.cxx>


NS_MIA_BEGIN
using namespace std; 

template <> 
struct NDVectorIOcvd<C2DFVector>{
	static bool read(istream& is, C2DFVector& value) {
		char c; 
		is >> value.x; 
		is >> c; 
		if (c != ';') 
			return false;
		
		is >> value.y; 
		return true; 
	}
	
	static void write(ostream& os, const C2DFVector& value){
		os << value.x << ";" << value.y; 
	}
}; 

template class  TTrackPoint<C2DTransformation>; 

NS_MIA_END
