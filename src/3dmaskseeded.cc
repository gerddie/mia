/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#include <iostream>
#include <string>
#include <queue>
#include <stdexcept>
#include <sstream>

#include <mia/3d/3dfilter.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/shape.hh>
#include <mia/core.hh>

using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
	"3D image processing", 
	
	
	"3D image segmentation based on region growing from a seed point. "
	"Neighboring points are added, either when their intensity is "
	"equal or higher than that of the seed point, or of the intensity "
	"is lower or equal then that of the neighoring point. "
	"After region growing is finished, this mask is used to zero out "
	"the region in the original image yielding the resulting image. ", 
	
	"Run a region growing on input.v starting at point <10,23,21> and "
	"use the 18n neighbourhood.", 
	
	"-i image.v -o masked.v -s '<10,23,21>' -n 18n" 
}; 



class FMask : public TFilter <P3DImage> {
public:

	FMask(const C3DBounds& seed_point, P3DShape neigborhood):
		m_seed_point(seed_point),
		m_neigborhood(neigborhood)
	{
	}

	template <typename T>
	P3DImage operator() (const T3DImage<T>& image) const;
private:

	template <typename T>
	struct seed_t {
		seed_t( const C3DBounds& _p, T _v):
			p(_p), value(_v) {};
		C3DBounds p;
		T value;
	};

	C3DBounds m_seed_point;
	P3DShape m_neigborhood;
};

template <typename T>
P3DImage FMask::operator() (const T3DImage<T>& image) const
{
	queue <seed_t<T> > pool;

	if ( m_seed_point < image.get_size() )   {

		// create mask by flood filling starting from the seed
		T thresh = image(m_seed_point);
		C3DBitImage mask(image.get_size());
		pool.push(seed_t<T>(m_seed_point, thresh));

		while (!pool.empty()) {

			seed_t<T> v = pool.front();
			pool.pop();

			if (mask(v.p)) // duplicate?
				continue;

			mask(v.p) = true;

			// add neighboring pixels that fullfill the requirements
			for (C3DShape::const_iterator i = m_neigborhood->begin();
			     i != m_neigborhood->end(); ++i) {

				C3DBounds x( v.p.x + i->x,v.p.y + i->y, v.p.z + i->z);

				if (!(x < image.get_size()))
					continue;

				T tv = image(x);
				if ( ( tv <= v.value || tv >= thresh ) && !mask(x) )
					pool.push(seed_t<T>(x, tv));
			}
		}

		// mask the image
		T3DImage<T> *result = new T3DImage<T>(image);
		typename T3DImage<T>::iterator       rb = result->begin();
		typename T3DImage<T>::iterator       re = result->end();
		C3DBitImage::const_iterator im = mask.begin();

		while (rb != re)  {
			if (*im)
				*rb = T();
			++rb; ++im;
		}
		return P3DImage(result);

	}else
		throw invalid_argument("Seed point outside image domain");
}

int do_main(int argc, char *argv[] )
{
	string in_filename;
	string out_filename;
	C3DBounds seed_point(0,0,0);
	string shape_descr("6n");

	const C3DImageIOPluginHandler::Instance& imageio =
		C3DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i',
				    "input image(s) to be filtered", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o',
				    "output image(s) that have been filtered", CCmdOption::required));
	options.add(make_opt( seed_point, "seed", 's', "seed point"));

	options.add(make_opt( shape_descr, "neighborhood", 'n', "neighborhood shape"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 




	C3DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(in_filename);

	if (in_image_list.get() && in_image_list->size()) {
		P3DShape shape(C3DShapePluginHandler::instance().produce(shape_descr.c_str()));
		FMask mask(seed_point, shape);

		for (C3DImageIOPluginHandler::Instance::Data::iterator i = in_image_list->begin();
		     i != in_image_list->end(); ++i)
			*i = mia::filter(mask,**i);
		if ( !imageio.save(out_filename, *in_image_list) ){
			string not_save = ("unable to save result to ") + out_filename;
			throw runtime_error(not_save);
		};
	}
	return EXIT_SUCCESS;
}


#include <mia/internal/main.hh>
MIA_MAIN(do_main)
