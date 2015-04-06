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


#ifndef mia_template_combiner_hh
#define mia_template_combiner_hh

#include <mia/core/filter.hh>
#include <mia/core/import_handler.hh>

NS_MIA_BEGIN

template <typename Image> 
class EXPORT_HANDLER TImageCombiner : public TFilter< typename Image::Pointer > ,
				   public CProductBase {
public:
	/// data type for plug-in serachpath component 
	typedef Image plugin_data; 
	/// plug-in type for plug-in serachpath component 
	typedef combiner_type plugin_type; 
	
        typedef typename TFilter< typename Image::Pointer >::result_type result_type; 

	/**
	   Combine two images by a given operator 
	   @param a 
	   @param b 
	   @returns combined image 
	   
	 */
	result_type combine( const Image& a, const Image& b) const;
private:
	virtual result_type do_combine( const Image& a, const Image& b) const = 0;
};


NS_MIA_END

#endif 
