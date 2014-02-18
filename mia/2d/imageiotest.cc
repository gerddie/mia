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

#include <climits>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/history.hh>
#include <mia/2d/imageio.hh>
#include <mia/core/filter.hh>


using namespace std;
using namespace boost;
using namespace boost::unit_test;

NS_MIA_BEGIN
typedef C2DImageIOPluginHandler::Instance::Interface Interface;




class CImageCompare: public TFilter<bool>  {
public:
	template <typename  T>
	CImageCompare::result_type operator()(const T2DImage<T>& a, const T2DImage<T>& b) const
	{
		return equal(a.begin(), a.end(), b.begin());
	}
};

static bool is_equal(const C2DImage& a, const C2DImage& b)
{
	BOOST_REQUIRE(a.get_pixel_type() == b.get_pixel_type());
	BOOST_REQUIRE(a.get_size() == b.get_size());

	CImageCompare compare;

	return filter_equal(compare, a, b);
}

static void test_attributes_equal(const CAttributedData& a, const CAttributedData& b)
{
	BOOST_CHECK(a == b);
}

static void check_save_load(const C2DImageVector& images, const Interface& imgio)
{
	const string format = imgio.get_name();
	const string tmp_name = string("imgtest") + string(".") + format;

	cvdebug() << format << ":" << tmp_name << endl;

	BOOST_REQUIRE(imgio.save(tmp_name.c_str(), images));

	cvdebug() << format << "saved" << std::endl;

	std::shared_ptr<C2DImageVector > reread(imgio.load(tmp_name.c_str()));

	BOOST_REQUIRE(reread.get());
	cvdebug() << images.size() << " vs. " << reread->size() << "\n";
	BOOST_REQUIRE(images.size() == reread->size());

	C2DImageVector::const_iterator in_i = images.begin();
	C2DImageVector::const_iterator in_e = images.end();

	C2DImageVector::const_iterator lo_i = reread->begin();

	for (; in_i != in_e; ++in_i, ++lo_i) {
		BOOST_REQUIRE(is_equal(**in_i, **lo_i));
		if (imgio.has_property(io_plugin_property_has_attributes)) {
			test_attributes_equal(**in_i, **lo_i);
		}

	}

	unlink(tmp_name.c_str());
	cvdebug() << tmp_name << " unlinked\n";
}


template <typename T>
static P2DImage create_image()
{
	T2DImage<T> *r = new T2DImage<T>(C2DBounds(13,19));
	int k = 0;
	for(typename T2DImage<T>::iterator i = r->begin(); i != r->end(); ++i, ++k)
		*i = T(k);

	r->set_pixel_size(C2DFVector(1.5f, 4.6f));
	r->set_attribute("some", PAttribute(new TAttribute<string>("some text")));

	return  P2DImage(r);
}

template <>
P2DImage create_image<bool>()
{
	C2DBitImage *r = new C2DBitImage(C2DBounds(13,19));
	for(C2DBitImage::iterator i = r->begin(); i != r->end(); ++i)
#ifdef WIN32
			*i =  rand() > RAND_MAX/2;
#else
			*i =  drand48() > 0.5;
#endif

	r->set_pixel_size(C2DFVector(1.5f, 4.6f));
	r->set_attribute("some", PAttribute(new TAttribute<string>("some other text")));
	return  P2DImage(r);
}

static P2DImage create_image_of_type(EPixelType type)
{
	switch (type) {
	case it_bit:    return create_image<bool>();
	case it_sbyte:  return create_image<signed char>();
	case it_ubyte:  return create_image<unsigned char>();
	case it_sshort: return create_image<signed short>();
	case it_ushort: return create_image<unsigned short>();
	case it_sint:   return create_image<signed int>();
	case it_uint:   return create_image<unsigned int>();
#ifdef LONG_64BIT
	case it_slong:  return create_image<signed long>();
	case it_ulong:  return create_image<unsigned long>();
#endif
	case it_float:  return create_image<float>();
	case it_double: return create_image<double>();
	default:
		BOOST_FAIL("IO plugin supports more types then we know of");
		return P2DImage();
	}
}



static void test_multi(const Interface & plugin)
{
	C2DImageVector src_list;
	Interface::PixelTypeSet pixeltypes = plugin.supported_pixel_types();

	for(set<EPixelType>::const_iterator i = pixeltypes.begin();
	    i != pixeltypes.end(); ++i) {
		src_list.push_back(create_image_of_type(*i));
	}
	check_save_load(src_list, plugin);
}

static void test_single(const Interface & plugin)
{
	cvdebug() << "test_imageio_plugin_single:" << plugin.get_name() << "\n";

	Interface::PixelTypeSet pixeltypes = plugin.supported_pixel_types();

	for(set<EPixelType>::const_iterator i = pixeltypes.begin();
	    i != pixeltypes.end(); ++i) {
		C2DImageVector src_list;
		src_list.push_back(create_image_of_type(*i));
		check_save_load(src_list, plugin);
	}

	cvdebug() << "test_imageio_plugin_single:" << plugin.get_name() << " done\n";
}

void EXPORT_2DTEST test_2dimageio_plugins()
{
	const C2DImageIOPluginHandler::Instance& plugins = C2DImageIOPluginHandler::instance();

	for (C2DImageIOPluginHandler::Instance::const_iterator i = plugins.begin();
	     i != plugins.end(); ++i) {

		if (i->second->has_property(io_plugin_property_multi_record))
			test_multi(*i->second);
		else
			test_single(*i->second);
	}
}

NS_MIA_END
