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

#ifndef GSLPP_VECTOR_HH
#define GSLPP_VECTOR_HH

#include <gsl++/gsldefines.hh>
#include <gsl++/vector_template.hh>

namespace gsl {

typedef TVector<double>  DoubleVector; 
typedef TVector<float>   FloatVector; 

typedef TVector<long>    LongVector; 
typedef TVector<int>     IntVector; 
typedef TVector<short>   ShortVector; 
typedef TVector<char>    CharVector; 

typedef TVector<ulong>   ULongVector; 
typedef TVector<uint>    UIntVector; 
typedef TVector<ushort>  UShortVector; 
typedef TVector<uchar>   UCharVector; 

extern template class EXPORT_GSL TVector<double>; 

typedef DoubleVector Vector; 



class EXPORT_GSL VectorView :public Vector {
public: 
	VectorView(gsl_vector_view vv): m_view(vv) {
		reset_holder(&m_view.vector); 
	}; 
private: 
	gsl_vector_view m_view; 
}; 

class EXPORT_GSL ConstVectorView :private  Vector {
public: 
	ConstVectorView(gsl_vector_const_view vv):m_view(vv) {
		reset_holder(&m_view.vector); 
	}; 
	
		const_iterator begin()const{
			return Vector::begin(); 
		}
		
		const_iterator end()const{
			return Vector::end(); 
		}
		
		size_type size() const {
			return Vector::size(); 
		}
		
		value_type operator[](size_t i)const {
			
			return Vector::operator[](i); 
		}; 
		
		/// read only vector pointer type operator to enable transparent calls to the GSL APL
		operator const vector_type *() const {
			return Vector::operator const vector_type *(); 
		}
		
		
		const vector_type * operator  ->() const{
			return Vector::operator ->(); 
		}


private: 
	gsl_vector_const_view m_view; 
}; 

}

#endif


