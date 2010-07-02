/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * BIT, ETSI Telecomunicacion, UPM
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


// this is for the definition of the 2D image plugin base classes
#include <mia/2d/2dfilter.hh>

// start a namespace to avoid naming conflicts during runtime

NS_BEGIN(selectbig_2dimage_filter)

// the name of the plugin - this is, how it will be called from the command line


/*Some filter specific parameters are defined here:
  the binarize filter converts an input image to a binary image. A pixel becomes 1
  if corresponding pixel in the the input image is in the range [min,max], 0 otherwise
*/


/* This is the work-horse class of the filter plugin. It is initialised with above parameters
   and provides a templated operator () to be able to handle all pixel types.
 */
class C2DSelectBig: public mia::C2DFilter {
public:
	C2DSelectBig();

	template <typename T>
	C2DSelectBig::result_type operator () (const mia::T2DImage<T>& data) const;
private:
	virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;
};

/* The factory class - this is what the application gets first. This factory class is used to
   create the actual filter object. It also provides some filter testing routines.
*/
class C2DSelectBigImageFilterFactory: public mia::C2DFilterPlugin {
public:
	C2DSelectBigImageFilterFactory();
	virtual mia::C2DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const;
	virtual bool do_test()const {return true;};
};

NS_END
