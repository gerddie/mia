/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <config.h>


#include <mia/internal/autotest.hh>

#include <mia/2d/imageio.hh>


using namespace std; 
using namespace mia; 
PrepareTestPluginPath g_prepare_pluginpath; 


template <typename Handler> 
struct PluginMapFixture {
	typedef vector<pair<string, string>> Expectmap; 

	void test_plugin_from_name(const Expectmap& map) const; 
	void test_suffix_from_name_or_suffix(const Expectmap& map) const; 
	
}; 

BOOST_FIXTURE_TEST_CASE( test_2d_plugin_mapping, PluginMapFixture<C2DImageIOPluginHandler> )
{
	
	Expectmap test_filenames;
	Expectmap test_prefsuffix;

	test_filenames.push_back(make_pair("test.BMP", "bmp")); 
	test_filenames.push_back(make_pair("test.bmp", "bmp")); 

	test_prefsuffix.push_back(make_pair("bmp", "bmp")); 
	
#ifdef HAVE_DCMTK 
	test_filenames.push_back(make_pair("test.dcm", "dicom")); 
	test_filenames.push_back(make_pair("test.DCM", "dicom")); 

	test_prefsuffix.push_back(make_pair("dicom", "dcm")); 

	test_prefsuffix.push_back(make_pair("dcm", "dcm")); 

#endif 

#ifdef HAVE_VISTAIO
	test_filenames.push_back(make_pair("test.v", "vista")); 
	test_filenames.push_back(make_pair("test.V", "vista")); 
	test_filenames.push_back(make_pair("test.vista", "vista")); 
	test_filenames.push_back(make_pair("test.VISTA", "vista")); 

	test_prefsuffix.push_back(make_pair("vista", "v")); 
	test_prefsuffix.push_back(make_pair("v", "v")); 
#endif 

#ifdef HAVE_OPENEXR
	test_filenames.push_back(make_pair("test.exr", "exr")); 
	test_filenames.push_back(make_pair("test.EXR", "exr")); 

	test_prefsuffix.push_back(make_pair("exr", "exr")); 
#endif 

#ifdef HAVE_PNG
	test_filenames.push_back(make_pair("test.png", "png")); 
	test_filenames.push_back(make_pair("test.PNG", "png")); 

	test_prefsuffix.push_back(make_pair("png", "png")); 
#endif 

#ifdef HAVE_TIFF
	test_filenames.push_back(make_pair("test.tif", "tif")); 
	test_filenames.push_back(make_pair("test.TIF","tif")); 
	test_filenames.push_back(make_pair("test.tiff", "tif")); 
	test_filenames.push_back(make_pair("test.TIFF","tif")); 

	test_prefsuffix.push_back(make_pair("tif", "tif")); 
	test_prefsuffix.push_back(make_pair("tiff", "tiff")); 
#endif 

#ifdef HAVE_JPG
	test_filenames.push_back(make_pair("test.jpg", "jpg")); 
	test_filenames.push_back(make_pair("test.JPG", "jpg")); 
	test_filenames.push_back(make_pair("test.jpeg", "jpg")); 
	test_filenames.push_back(make_pair("test.JPEG", "jpg")); 

	test_prefsuffix.push_back(make_pair("jpg", "jpg")); 
	test_prefsuffix.push_back(make_pair("jpeg", "jpeg")); 
#endif 

	test_plugin_from_name(test_filenames); 
	test_suffix_from_name_or_suffix(test_prefsuffix); 

}

template <typename Handler> 
void PluginMapFixture<Handler>::test_plugin_from_name(const Expectmap& map) const
{
	const auto& io = Handler::instance(); 
	for_each(map.begin(), map.end(), 
		 [&io](const pair<string, string>& p) {
			 const auto& plugin = io.preferred_plugin(p.first); 
			 BOOST_CHECK_EQUAL(string(plugin.get_name()), p.second); 
		 }); 
	
}

template <typename Handler> 
void PluginMapFixture<Handler>::test_suffix_from_name_or_suffix(const Expectmap& map) const
{
	const auto& io = Handler::instance(); 
	for_each(map.begin(), map.end(), 
		[&io](const pair<string, string>& p) {
			 cvdebug() << "Test '" << p.first << "\n"; 
			 BOOST_CHECK_EQUAL(io.get_preferred_suffix(p.first), p.second); 
		 });

}
