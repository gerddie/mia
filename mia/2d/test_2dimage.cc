/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <iostream>

#include <mia/core.hh>
#include <mia/2d/2DImage.hh>


NS_MIA_USE
using namespace boost;


class CCopyFilter: public TFilter<P2DImage >  {
public:

	template <typename T>
	result_type operator () (const T2DImage<T>& image) const  {
		return result_type(new T2DImage<T>(image));
	}
};


template <typename T>
static void creat_and_check(EPixelType type)
{
	const T init_data[4] = {0,  1,  0,  1	};

	P2DImage image(new T2DImage<T>(C2DBounds(2,2), init_data));
	BOOST_CHECK(image->get_pixel_type() == type);

	CCopyFilter copy_filter;
	P2DImage result = filter(copy_filter, *image);
	BOOST_CHECK(result->get_pixel_type() == type);

	T2DImage<T> *img = dynamic_cast<T2DImage<T> *>(image.get());
	BOOST_CHECK(img);
	BOOST_CHECK_EQUAL(img->size(), 4);

	P2DImage cloned(image->clone());
	BOOST_CHECK_EQUAL(cloned->get_pixel_type(), type);

	img = dynamic_cast< T2DImage<T> * >(cloned.get());
	BOOST_REQUIRE(img);
	BOOST_CHECK_EQUAL(img->size(),4);

	C2DFVector pixel_size(2.3f, 7.8f);
        img->set_pixel_size(pixel_size);
        BOOST_CHECK_EQUAL(img->get_pixel_size(), pixel_size);

	T2DImage<T> data2(*img);
	BOOST_CHECK_EQUAL(data2.get_pixel_size(), pixel_size);

	C2DFVector pixel_size2(3.1f, 8.1f);
	data2.set_pixel_size(pixel_size2);
	BOOST_CHECK_EQUAL(data2.get_pixel_size(), pixel_size2);
	BOOST_CHECK_EQUAL(img->get_pixel_size(), pixel_size);

	*img = data2;
	BOOST_CHECK_EQUAL(img->get_pixel_size(), pixel_size2);
}

BOOST_AUTO_TEST_CASE( check_data_types )
{
	creat_and_check<bool>(it_bit);
	creat_and_check<unsigned char>(it_ubyte);
	creat_and_check<signed char>(it_sbyte);
	creat_and_check<unsigned short>(it_ushort);
	creat_and_check<signed short>(it_sshort);
	creat_and_check<unsigned int>(it_uint);
	creat_and_check<signed int>(it_sint);
#ifdef HAVE_INT64
	creat_and_check<mia_uint64>(it_ulong);
	creat_and_check<mia_int64>(it_slong);
#endif
	creat_and_check<float>(it_float);
	creat_and_check<double>(it_double);
}

BOOST_AUTO_TEST_CASE( check_gradient )
{
	float init_data[16] = {
		0, 0, 0, 0,
		1, 1, 2, 3,
		5, 4, 3, 3,
		4, 5, 6, 7
	};

	C2DFVector test_data[16] = {
		C2DFVector( 0.0f, 0.0f ), C2DFVector( 0.0f, 0.0f ), C2DFVector( 0.0f, 0.0f ), C2DFVector( 0.0f, 0.0f ),
		C2DFVector( 0.0f, 0.0f ), C2DFVector( 0.5f, 2.0f ), C2DFVector( 1.0f, 1.5f ), C2DFVector( 0.0f, 0.0f ),
		C2DFVector( 0.0f, 0.0f ), C2DFVector(-1.0f, 2.0f ), C2DFVector(-0.5f, 2.0f ), C2DFVector( 0.0f, 0.0f ),
		C2DFVector( 0.0f, 0.0f ), C2DFVector( 0.0f, 0.0f ), C2DFVector( 0.0f, 0.0f ), C2DFVector( 0.0f, 0.0f )
	};

	C2DFImage image(C2DBounds(4,4), init_data);
	C2DFVectorfield gradient = get_gradient(image);

	BOOST_CHECK(equal(gradient.begin(), gradient.end(), test_data));

	C2DFVector g = image.get_gradient(C2DFVector(1.25,1.75));
	cvdebug() << g << "vs." << C2DFVector( -0.5f, 1.96875f ) << endl;
	BOOST_CHECK( g == C2DFVector( -0.5f, 1.96875f ));

}

