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


#include <iostream>
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

class EXPORT_GSL ConstVectorView {
public: 
	ConstVectorView(gsl_vector_const_view vv):m_view(vv), 
		m_holder(&m_view.vector)
	{
		
	}; 
	
	Vector::const_iterator begin()const{
		return m_holder.begin(); 
	}
		
	Vector::const_iterator end()const{
		return m_holder.end(); 
	}
	
	Vector::size_type size() const {
		return m_holder.size(); 
	}
	
	Vector::value_type operator[](size_t i)const {
		
		return m_holder[i]; 
	}; 

	operator const Vector&(){
		return m_holder; 
	}
		
	const Vector::vector_type * operator  ->() const{
		return m_holder.operator ->(); 
	}
	
	operator Vector::vector_const_pointer_type () const {
		return m_holder.operator vector_const_pointer_type(); 
	}

private: 
	gsl_vector_const_view m_view; 
	const Vector m_holder; 
}; 

inline std::ostream& operator << (std::ostream& os, const Vector& v) {
	v.print(os); 
	return os; 
} 

}

#endif


