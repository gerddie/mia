/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2011
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


#include <mia/internal/autotest.hh>
#include <mia.hh>
#include <config.h>


using namespace std; 

struct PluginTestFixture {
	
	void test(const set< string >& plugins, const set<string>& test_data) const; 
}; 



BOOST_FIXTURE_TEST_CASE(test_C3DFullCostPluginHandler,PluginTestFixture) 
{ 
	set<string> test_data = { "divcurl", "image"}; 

	test(C3DFullCostPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C3DImageCostPluginHandler,PluginTestFixture) 
{ 
	set<string> test_data = { "ngf", "ssd"}; 
	test(C3DImageCostPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C3DFilterPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"binarize", "bandpass", "convert","close", "crop", "dilate", "downscale", "erode", "gauss", "growmask", 
		"invert", "kmeans",  "label", "mask", "median", "mlv", "open",  "reorient", 
		"sandp", "scale", "selectbig", "sepconv" 
	}; 
	test(C3DFilterPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C3DImageCombinerPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"labelxmap"
	}; 
	test(C3DImageCombinerPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C3DImageIOPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"analyze",  
		"inria", 
		"vff", 
		"vtk", 
		"dicom", 
		"vista", 
		"datapool"
	}; 
	test(C3DImageIOPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C3DTransformCreatorHandler,PluginTestFixture) 
{
	set<string> test_data = { 
		"affine", "rigid", "spline", "translate"
	}; 
	test(C3DTransformCreatorHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C3DFatImageCostPluginHandler,PluginTestFixture) 
{
	set<string> test_data = { 
		"ssd", "ngf"
	}; 
	test(C3DFatImageCostPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C3DRegTimeStepPluginHandler,PluginTestFixture) 
{
	set<string> test_data = { 
		"direct", "fluid"
	}; 
	test(C3DRegTimeStepPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C3DShapePluginHandler,PluginTestFixture) 
{
	set<string> test_data = { 
		"6n", "18n", "26n", "sphere"
	}; 
	test(C3DShapePluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C3DVFIOPluginHandler,PluginTestFixture) 
{
	set<string> test_data = { 
		"cvista", "vista", "vtk", "datapool"
	}; 
	test(C3DVFIOPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C2DFifoFilterPluginHandler,PluginTestFixture) 
{
	set<string> test_data = { 
		"byslice", "gauss", "label", "median", "mlv", 
		"dilate", "erode", "open", "close", "regiongrow"
	}; 
	test(C2DFifoFilterPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C2DStackDistanceTransformIOPluginHandler,PluginTestFixture) 
{
	set<string> test_data {
		"datapool"
	}; 
	test(C2DStackDistanceTransformIOPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C3DTransformationIOPluginHandler,PluginTestFixture) 
{
	set<string> test_data = { 
		"datapool", "bbs", "vista", "xml"
	}; 
	test(C3DTransformationIOPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C3DRegModelPluginHandler,PluginTestFixture) 
{
	set<string> test_data = { 
		"navier", "naviera", "naviersse", "navierpsse"
	}; 
	test(C3DRegModelPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_CMeshIOPluginHandler,PluginTestFixture) 
{
	set<string> test_data = { 
		"datapool", "off", "vista", "ply", "stl"
	}; 

	test(CMeshIOPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_CMinimizerPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"gsl", "gdsq", "nlopt"
	}; 
	test(CMinimizerPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_CSplineKernelPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"bspline", "omoms"
	}; 
	test(CSplineKernelPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_CNoiseGeneratorPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"gauss", "uniform"
	}; 
	test(CNoiseGeneratorPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C1DSpacialKernelPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"gauss"
	}; 
	test(C1DSpacialKernelPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C2DVFIOPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"datapool", "vista", "exr"
	}; 
	test(C2DVFIOPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C2DFullCostPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"image", "divcurl"
	}; 
	
	test(C2DFullCostPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C2DImageCostPluginHandler,PluginTestFixture) 
{
	set<string> test_data =  {
		"lsd", "ssd", "ngf" 
	}; 
	test(C2DImageCostPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C2DImageIOPluginHandler,PluginTestFixture) 
{
	set<string> test_data =  {
		"bmp", "exr", "datapool", "dicom", "png", "raw", "tif", "vista"
	}; 
	test(C2DImageIOPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_CFFT2DKernelPluginHandler,PluginTestFixture) 
{
	set<string> test_data; 
	test(CFFT2DKernelPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C2DTransformCreatorHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"4n", "8n", "sphere"
	}; 

	test(C2DShapePluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C2DFilterPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"adaptmed", "admean", "aniso", "bandpass", "binarize", 
		"convert", "close", "crop", "cst", "dilate", "distance", 
		"downscale", "erode", "fft", "gauss", "gradnorm", "invert", "kmeans", 
		"label", "labelmap", "mask", "mean", "median", "mlv", 
		"ngfnorm", "noise", "open", "sandp", "scale", "selectbig", 
		"sepconv", "thresh"
	}; 
	test(C2DFilterPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C2DImageCombinerPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"absdiff", "add", "div", "mul", "sub"
	}; 
	test(C2DImageCombinerPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C2DTransformationIOPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"vista", "datapool", "bbs", "pbs", "xml"
	}; 
	test(C2DTransformationIOPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_C2DRegModelPluginHandler,PluginTestFixture) 
{
	set<string> test_data = {
		"identity", "navier", "naviera"
	}; 
	test(C2DRegModelPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_CCST2DVectorKernelPluginHandler,PluginTestFixture) 
{
	set<string> test_data; 
	test(CCST2DVectorKernelPluginHandler::instance().get_set(), test_data); 
}

BOOST_FIXTURE_TEST_CASE(test_CCST2DImgKernelPluginHandler,PluginTestFixture) 
{
	set<string> test_data; 
	test(CCST2DImgKernelPluginHandler::instance().get_set(), test_data); 
}


void PluginTestFixture::test(const set< string >& plugins, const set<string>& test_data) const
{
	BOOST_CHECK_EQUAL(plugins.size(), test_data.size()); 
	for (auto p = plugins.begin(); p != plugins.end(); ++p) {
		BOOST_CHECK_MESSAGE(test_data.find(*p) != test_data.end(), "unexpected plugin '" << *p << "' found"); 
	}
	
	for (auto p = test_data.begin(); p != test_data.end(); ++p)
		BOOST_CHECK_MESSAGE(plugins.find(*p) != plugins.end(), "expected plugin '" << *p << "' not found"); 
}
