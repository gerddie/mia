/* -*- mia-c++  -*-
 * Copyright (c) 2009 Gert Wollny <gert at die.upm.es>
 * 
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


#include <mia/3d/3dfilter.hh>
#include <mia/3d/shape.hh>
#include <mia/3d/3dimageio.hh>

NS_BEGIN(growmask_3dimage_filter)



class C3DGrowmask: public mia::C3DFilter {
public:
	C3DGrowmask(const mia::C3DImageDataKey& reference, mia::P3DShape neigborhood, float min);
	
	template <typename T>
	C3DGrowmask::result_type operator () (const mia::T3DImage<T>& data) const;
private: 
	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
	mia::C3DImageDataKey _M_ref; 
	mia::P3DShape _M_neigborhood; 
	float _M_min; 
};

/* The factory class - this is what the application gets first. This factory class is used to 
   create the actual filter object. It also provides some filter testing routines. 
*/
class C3DGrowmaskImageFilterFactory: public mia::C3DFilterPlugin {
public: 
	C3DGrowmaskImageFilterFactory();
	virtual mia::C3DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const; 
	virtual bool do_test()const {return true;}; 
private: 
	std::string _M_ref_filename; 
	std::string _M_shape_descr; 
	float _M_min; 
};
	
NS_END
