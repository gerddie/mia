/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#ifndef mia_core_shape_hh
#define mia_core_shape_hh

#include <set>

#include <mia/core/product_base.hh>
#include <mia/core/type_traits.hh>
#include <mia/core/utils.hh>

#ifndef EXPORT_SHAPE
#  ifdef WIN32
#    define EXPORT_SHAPE __declspec(dllimport)
#  else
#    define EXPORT_SHAPE
#  endif
#endif

NS_MIA_BEGIN


struct EXPORT_CORE shape_type {
	static  const char * type_descr;
};



/**
   \ingroup basic 
   \brief a generic class for morphological shapes 
   
   Pixel representation of shapes and masks around the origin.
   Type T is the coordinate class (e.g. T2DVector) and type M is the Mask image
   type, usually an binary image of the same dimensionallity as the coordinate
   class.
   For a specific instanciation, the user must provied a specialisation __adjust<T<int> > of
   the template defined in the shape.cxx, as well as a less_then<T<int> > specialisation.
   The implementation file of an instaciation must include shape.cxx, and must instanciate
   the TShape<T,M> with the user provided types T and M explicitely.
*/

template <template <typename> class T, typename M>
class EXPORT_SHAPE TShape :public CProductBase {
public:
	/// helper type for use of the class in TFactory 
	typedef shape_type plugin_type; 
	
	/// Obtain the data type that is used for the mask image from the template parameters 
	typedef typename M::Super plugin_data; 
	
	/** type to store the mask coordinates in a set to be able to easily iterate over it 
	    and avoid duplicate entries 
	*/ 
	typedef std::set< T<int>, less_then<T<int> > > Flat;
	
	/// the actual value type of the mask coordinates 
	typedef T<int> value_type;
	
	/// iterator over the mask coordinate set 
	typedef typename Flat::const_iterator const_iterator;

	/// the mask image type 
	typedef M Mask;
	
	/// the size type for the mask image 
	typedef T<unsigned int> Size;

	/// standard constructor initialises the shape set
	TShape();

	/// \returns the iterator to the begin of the shape
	const_iterator begin() const;

	/// \returns the iterator to the past end of the shape
	const_iterator end() const;

	/// \returns an coordinate shifted image of the real mask
	Mask get_mask()const;

	/// returns the nD size of the mask
	Size get_size()const;

	/// \returns the number of points in the shape definition 
	typename Flat::size_type size() const; 

	/** insert another pixel coordinate
	    \param p the pixel coordinate to be added 
	 */
	void insert(const T<int>& p);

	/**
	   Test if a certain pixel location is set in the shape
	   \param p the pixel position 
	   \returns true if the pixel location is part of the shape 
	 */
	bool has_location(const T<int>& p) const; 
private:
	Flat m_shape;
	T<int> m_size;
};

NS_MIA_END

#endif
