/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <iostream>

#include <mia/core.hh>
#include <mia/2d/image.hh>


NS_MIA_USE

class CCopyFilter: public TFilter<P2DImage >  {
public:

	template <typename T>
	result_type operator () (const T2DImage<T>& image) const  {
		return result_type(new T2DImage<T>(image));
	}
};


template <typename T>
static void create_and_check(EPixelType type)
{
	const T init_data[4] = {0,  1,  0,  1	};

	P2DImage image(new T2DImage<T>(C2DBounds(2,2), init_data));
	BOOST_CHECK(image->get_pixel_type() == type);

	CCopyFilter copy_filter;
	P2DImage result = filter(copy_filter, *image);
	BOOST_CHECK(result->get_pixel_type() == type);

	T2DImage<T> *img = dynamic_cast<T2DImage<T> *>(image.get());
	BOOST_CHECK(img);
	BOOST_CHECK_EQUAL(img->size(), 4u);

	P2DImage cloned(image->clone());
	BOOST_CHECK_EQUAL(cloned->get_pixel_type(), type);

	img = dynamic_cast< T2DImage<T> * >(cloned.get());
	BOOST_REQUIRE(img);
	BOOST_CHECK_EQUAL(img->size(),4u);

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

typedef boost::mpl::vector<bool,
		     int8_t,
		     uint8_t,
		     int16_t,
		     uint16_t,
		     int32_t,
		     uint32_t,
		     int64_t,
		     uint64_t,
		     float,
		     double
		     > test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE( check_data_types, T, test_types)
{
	create_and_check<T>(pixel_type<T>::value);
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
		C2DFVector( 0.0f, 2.5f ), C2DFVector( 0.5f, 2.0f ), C2DFVector( 1.0f, 1.5f ), C2DFVector( 0.0f, 1.5f ),
		C2DFVector( 0.0f, 1.5f ), C2DFVector(-1.0f, 2.0f ), C2DFVector(-0.5f, 2.0f ), C2DFVector( 0.0f, 2.0f ),
		C2DFVector( 0.0f, 0.0f ), C2DFVector( 1.0f, 0.0f ), C2DFVector( 1.0f, 0.0f ), C2DFVector( 0.0f, 0.0f )
	};

	C2DFImage image(C2DBounds(4,4), init_data);
	C2DFVectorfield gradient = get_gradient(image);

	BOOST_CHECK(equal(gradient.begin(), gradient.end(), test_data));

	C2DFVector g = image.get_gradient(C2DFVector(1.25,1.75));
	cvdebug() << g << "vs." << C2DFVector( -0.5f, 1.96875f ) << "\n";
	BOOST_CHECK( g == C2DFVector( -0.5f, 1.96875f ));

}

BOOST_AUTO_TEST_CASE( check_comparison )
{
	C2DBounds size(1,2); 
	C2DFImage fimage(size, {1.0f, 2.0f});
	C2DUBImage ubimage(size);
	C2DUBImage ubimage2(C2DBounds(2,2));
	C2DFImage fimage2(fimage);

	BOOST_CHECK(fimage == fimage2);
	
	fimage2(0,0) = 4.0;
	BOOST_CHECK(fimage != fimage2);

	BOOST_CHECK(fimage != ubimage);
	BOOST_CHECK(ubimage != ubimage2);

	
}

BOOST_AUTO_TEST_CASE (test_move_semantics)
{
	std::string attr("test_string"); 
	C2DBounds size1(2,3); 
	C2DUBImage image1(size1);
	image1(0,0) = 1.0;
	image1.set_attribute("test", attr);
	
	C2DBounds size2(1,1); 
	C2DUBImage image2(size2);
	
	BOOST_CHECK_EQUAL(image2.get_size(), size2);


	C2DUBImage image1_moved(std::move(image1));

	BOOST_CHECK_EQUAL(image1_moved.get_size(), size1);
	BOOST_CHECK_EQUAL(image1_moved(0,0), 1.0);
	BOOST_CHECK_EQUAL(image1_moved.get_attribute_as_string("test"), attr);

	
	BOOST_CHECK_EQUAL(image1.get_size(), C2DBounds::_0);

	image2 =(std::move(image1_moved));
	BOOST_CHECK_EQUAL(image2.get_size(), size1);
	BOOST_CHECK_EQUAL(image2(0,0), 1.0);
	BOOST_CHECK_EQUAL(image2.get_attribute_as_string("test"), attr);

	BOOST_CHECK_EQUAL(image1_moved.get_size(), C2DBounds::_0);
}
