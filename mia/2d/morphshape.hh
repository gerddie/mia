/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_2d_morphshape_hh
#define mia_2d_morphshape_hh

#include <mia/2d/shape.hh>

NS_MIA_BEGIN


/** 
    @ingroup misc 
    \brief a class for advanced morphological filter masks

    This class provides the base for masks that support \a foregound, \a background and 
    \a don't \a care pixels. Such masks are used in more advanced morphological filters 
    like thinning, pruning, corner detection, and the likes.
*/

class EXPORT_2D C2DMorphShape : public CProductBase{
public: 
	/// helper type for plug-in handling 
	typedef C2DImage plugin_data; 
	
	/// helper type for plug-in handling 
	typedef C2DMorphShape plugin_type; 

	/// helper string for plug-in handling 
	static const char *type_descr;

	/// The pointer type of the class 
	typedef std::shared_ptr<C2DMorphShape> Pointer; 

	/// the value type based on the shape representation 
	typedef C2DShape::value_type value_type; 

	/// default constructor
	C2DMorphShape(); 


	/**
	   Constructor to create the mask from given shapes for background and foreground 
	   mask locations. Note, that these two masks must not overlap. 
	   \param foreground_mask shape describing the foreground pixles
	   \param background_mask shape describing the background pixles
	   
	 */
	C2DMorphShape(P2DShape foreground_mask, P2DShape background_mask); 
	

	/**
	   Add a pixel to either the foreground or the background mask
	   \param pixel the pixel location 
	   \param foreground set to true if the pixel goes to the foreground mask 
	   and to false if the pixel belongs to the background.
	 */
	void add_pixel(const value_type& pixel, bool foreground); 

	/**
	   Add a pixel to either the foreground or the background mask
	   \param x x-coordinate of the pixel location 
	   \param y y-coordinate of the pixel location 
	   \param foreground set to true if the pixel goes to the foreground mask 
	   and to false if the pixel belongs to the background.
	 */
	void add_pixel(int x, int y, bool foreground); 
	

	/// \returns the foreground mask 
	const C2DShape& get_foreground_mask() const; 

	/// \returns the background mask 
	const C2DShape& get_background_mask() const;

	/// \returns the combined mask rotate by 90 degree 
	C2DMorphShape rotate_by_90() const;
private: 

	P2DShape m_foreground_mask; 
	P2DShape m_background_mask; 
}; 

/// The pointer type for the morp shape class. 
typedef C2DMorphShape::Pointer P2DMorphShape; 


/**
   Implements the hit-and-miss transformation on a binary image with a given 
   morphshape. 
   \param[in, out] target output bitmap, the image must be allocated and of the same size like the input image
   \param[in] source input image 
   \param[in] shape structuring element to be used in the operation 
   \returns number of set pixels
 */
size_t EXPORT_2D morph_hit_and_miss_2d(C2DBitImage& target, const C2DBitImage& source, const C2DMorphShape& shape);

/**
   Implements the thinning transformation on a binary image with a given 
   morphshape. 
   \param[in, out] target output bitmap, the image must be allocated and of the same size like the input image
   \param[in] source input image 
   \param[in] shape structuring element to be used in the operation 
   \returns number of set pixels
 */
size_t EXPORT_2D morph_thinning_2d(C2DBitImage& target, const C2DBitImage& source, const C2DMorphShape& shape);


NS_MIA_END

#endif

