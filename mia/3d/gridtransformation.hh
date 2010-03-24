/* -*- mia-c++  -*-
 *
 * Copyright (c) 2007 Gert Wollny <gert dot wollny at acm dot org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#ifndef mia_3d_gridtransformation_hh
#define mia_3d_gridtransformation_hh


#include <iterator>
#include <mia/3d/transform.hh>


NS_MIA_BEGIN

/**
   3D transformation based on a deformation vector for each grid point. 
 */

class C3DGridTransformation : public C3DTransformation {
public: 

	typedef C3DFVectorfield::iterator field_iterator;  
	typedef C3DFVectorfield::const_iterator const_field_iterator;  
	
	C3DGridTransformation(const C3DBounds& size); 

	operator C3DFVectorfield&(); 

	C3DFVector operator ()(const  C3DFVector& x) const; 

	C3DFVectorfield& field() __attribute__((deprecated)) ; 

	const C3DFVectorfield& field() const __attribute__((deprecated)) ; 

	class const_iterator  {
	public: 
		const_iterator& operator ++(); 
		const_iterator operator ++(int); 
		
		const C3DFVector operator *() const; 

		friend bool operator == (const const_iterator& a, const const_iterator& b); 

		const_iterator(); 
		
	private: 
		friend class C3DGridTransformation; 
		const_iterator(const C3DBounds& pos, const C3DBounds& size, C3DFVectorfield::const_iterator start);

		C3DBounds _M_pos; 
		C3DBounds _M_size; 
	
		C3DFVectorfield::const_iterator _M_current; 

	}; 

	const_iterator begin() const; 
	const_iterator end() const; 

	const_iterator begin_at(size_t x, size_t y, size_t z) const; 

	field_iterator field_begin(); 
	field_iterator field_end(); 

	const_field_iterator field_begin()const; 
	const_field_iterator field_end()const; 


	virtual const C3DBounds& get_size() const; 

private: 

	virtual bool do_save(const std::string& filename, const std::string& type) const; 
	virtual P3DTransformation do_upscale(const C3DBounds& size) const;
	virtual void do_add(const C3DTransformation& a);
	virtual size_t do_degrees_of_freedom() const;   
        virtual P3DImage apply(const C3DImage& image, const C3DInterpolatorFactory& ipf) const; 

	C3DFVectorfield _M_field; 
}; 

inline C3DFVector C3DGridTransformation::operator ()(const  C3DFVector& x) const
{
	return _M_field.get_interpol_val_at(x); 
}

C3DGridTransformation operator + (const C3DGridTransformation& a, const C3DGridTransformation& b); 

NS_MIA_END

namespace std {
	template <>
	struct iterator_traits<mia::C3DGridTransformation::const_iterator> {
		typedef input_iterator_tag iterator_category;
		typedef mia::C3DFVector       value_type;
		typedef mia::C3DBounds        difference_type;
		typedef mia::C3DFVector*      pointer;
		typedef mia::C3DFVector&      reference;
	};
}



#endif
