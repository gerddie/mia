/* -*- mia-c++  -*-
 * Copyright (c) 2004-2007
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology
 * Gert Wollny <gert.wollny at web.de> 
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

#include <algorithm>
#include <climits>

#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/history.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/3dimageiotest.hh>
#include <mia/core/filter.hh>

NS_MIA_BEGIN
using namespace boost; 
using namespace std; 
using namespace boost::unit_test;
namespace bfs=::boost::filesystem; 

#ifdef WIN32

inline double drand48() {
	return double(rand()) / RAND_MAX; 
}

#ifdef _MSV_VER
#undef max
#undef min 
#endif

#endif

class CImageCompare: public TFilter<bool>  {
public: 
	template <typename  T>
	bool operator()(const T3DImage<T>& a, const T3DImage<T>& b) const 
	{
		return equal(a.begin(), a.end(), b.begin());
	}
};

static bool is_equal(const C3DImage& a, const C3DImage& b)
{
	if (a.get_pixel_type() != b.get_pixel_type()) {
		cvfail() << "types differ: " <<a.get_pixel_type() << " vs. " << b.get_pixel_type()<< endl; 
		return false; 
	}
	
	if (a.get_size() != b.get_size()) {
		cvfail()  << "size differs" << endl; 
		return false; 
	}
	
	CImageCompare compare;
	
	return filter_equal(compare, a, b);
}


template <typename T>
static SHARED_PTR(C3DImage) create_image() 
{
	const T med = numeric_limits<T>::max() / 2; 
	
	T3DImage<T> *r = new T3DImage<T>(C3DBounds(3,5,8)); 
	for(typename T3DImage<T>::iterator i = r->begin(); i != r->end(); ++i)
		*i = T(numeric_limits<T>::max() * drand48() - med); 

	r->set_voxel_size(C3DFVector(2.0,1.0,0.5)); 
	r->set_attribute("some", PAttribute(new TAttribute<string>("some text")));

	return  SHARED_PTR(C3DImage) (r); 
}

template <>
SHARED_PTR(C3DImage) create_image<bool>() 
{
	C3DBitImage *r = new C3DBitImage(C3DBounds(3,5,8)); 
	for(C3DBitImage::iterator i = r->begin(); i != r->end(); ++i)
#ifdef WIN32
		*i =  rand() > RAND_MAX/2; 
#else
		*i =  drand48() > 0.5; 
#endif

	r->set_voxel_size(C3DFVector(2.0,1.0,0.5)); 
	r->set_attribute("some", PAttribute(new TAttribute<string>("some other text")));

	return  SHARED_PTR(C3DImage) (r); 
}

static SHARED_PTR(C3DImage) create_image_of_type(EPixelType type) 
{
	switch (type) {
	case it_bit:    return create_image<bool>(); 
	case it_sbyte:  return create_image<signed char>(); 
	case it_ubyte:  return create_image<unsigned char>(); 
	case it_sshort: return create_image<signed short>(); 
	case it_ushort: return create_image<unsigned short>(); 
	case it_sint:   return create_image<signed int>(); 
	case it_uint:   return create_image<unsigned int>(); 
#ifdef HAVE_INT64
	case it_slong:  return create_image<mia_int64>(); 
	case it_ulong:  return create_image<mia_uint64>(); 
#endif
	case it_float:  return create_image<float>(); 
	case it_double: return create_image<double>(); 
	default:
		cvfail() << "IO plugin supports more types then we know of\n"; 
		return SHARED_PTR(C3DImage) (); 
	}
}

#if 0
static bool check_entry(const string& key, const string& val, const CHistoryRecord& hr) 
{
	CHistoryRecord::const_iterator i = hr.find(key);
	if (i == hr.end()) 
		return false; 
	
	return i->second == val; 
}
#endif


static void test_attributes_equal(const C3DImage& a, const C3DImage& b)
{
	const PAttributeMap ad = a.get_attribute_list(); 
	const PAttributeMap bd = b.get_attribute_list(); 

	BOOST_CHECK_EQUAL(ad->size(), bd->size()); 
	
	for (CAttributeMap::const_iterator ai = ad->begin(); ai != ad->end(); ++ai) {
		CAttributeMap::const_iterator bi = bd->find(ai->first); 
		if (bi == bd->end()) {
			BOOST_FAIL(ai->first); 
			continue; 
		}
		cvdebug() << ai->first << "\n"; 
		BOOST_CHECK(ai->second->is_equal(*bi->second)); 
	}
}

static void check_save_load(const C3DImageVector& images, const C3DImageIOPlugin& imgio, bool /*test_history_split*/)
{
	const string format = imgio.get_name(); 
	const string tmp_name = string("imgtest_") + format; 
	
	CHistory::instance().clear();
#if 0
	if (imgio.has_history()){
		// append some history
		popt::COptions options; 
		CHistory::instance().append("test_imageio", revision, options);
	}
#endif
	
	BOOST_REQUIRE(imgio.save(tmp_name.c_str(), images));
	
	cvdebug() << format << " saved to :" << tmp_name <<std::endl;
	
	// clear the history
	CHistory::instance().clear();
	
	SHARED_PTR(C3DImageVector) reread(imgio.load(tmp_name.c_str()));
	
	BOOST_REQUIRE(reread.get()); 
	
	BOOST_REQUIRE(images.size() == reread->size()); 
	
	C3DImageVector::const_iterator in_i = images.begin();
	C3DImageVector::const_iterator in_e = images.end();
	
	C3DImageVector::const_iterator lo_i = reread->begin();
	
	for (; in_i != in_e; ++in_i, ++lo_i) {
		BOOST_CHECK(is_equal(**in_i, **lo_i)); 
		if (imgio.has_property(io_plugin_property_has_attributes)) {
			test_attributes_equal(**in_i, **lo_i);
			BOOST_CHECK_EQUAL(C3DFVector(2.0,1.0,0.5), (*in_i)->get_voxel_size()); 
		}
	}
#if 0	
	if (imgio.has_history()) {
		// now check the history
		if (CHistory::instance().size() != 1) {
			cvfail() <<  format <<  " history failed, History size = " << 
				CHistory::instance().size() << endl; 
			return false; 
		}
		
		if (test_history_split) {
			CHistory::const_iterator i = CHistory::instance().begin();
			
			if (!check_entry("+PROGRAM", "test_imageio", i->second))
				return false; 
			if (!check_entry("+LIBMIA_VERSION", PACKAGE_VERSION, i->second))
				return false; 
			if (!check_entry("+USER", getenv("USER"), i->second))
				return false; 
			if (!check_entry("+VERSION", revision, i->second))
				return false; 
			if (!check_entry("+LIBMIA_REVISION", get_revision(), i->second))
				return false;
		}
	}
#endif
	unlink(tmp_name.c_str());

}



static void test_imageio_plugin_multi(const C3DImageIOPlugin& plugin)
{
	C3DImageVector src_list; 
	
	for(set<EPixelType>::const_iterator i = plugin.supported_pixel_types().begin(); 
	    i != plugin.supported_pixel_types().end(); ++i) {
		src_list.push_back(create_image_of_type(*i)); 
	}
	check_save_load(src_list, plugin, plugin.has_property(io_plugin_property_history_split)); 
}


static void test_imageio_plugin_single(const C3DImageIOPlugin& plugin)
{
	
	for(set<EPixelType>::const_iterator i = plugin.supported_pixel_types().begin(); 
	    i != plugin.supported_pixel_types().end(); ++i) {
		C3DImageVector src_list; 
		src_list.push_back(create_image_of_type(*i)); 
		check_save_load(src_list, plugin, plugin.has_property(io_plugin_property_history_split)); 
	}
	
}

static void test_3dimageio_plugins(const C3DImageIOPluginHandler::value_type& i)
{
	if (i.second->has_property(io_plugin_property_multi_record))
		test_imageio_plugin_multi(*i.second); 
	else 
		test_imageio_plugin_single(*i.second); 
}

void EXPORT_3DTEST add_3dimageio_plugin_tests( boost::unit_test::test_suite* suite)
{
	suite->add( BOOST_PARAM_TEST_CASE(&test_3dimageio_plugins, 
					  C3DImageIOPluginHandler::instance().begin(), 
					  C3DImageIOPluginHandler::instance().end()
					  )); 
}

NS_MIA_END
