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

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <cassert>
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/rgbimageio.hh>

#include "jpeg-common.hh"

NS_BEGIN(RGBIMAGEIO_2D_JPG)

using namespace mia;
using namespace std;
using namespace boost;

class CJpegRGB2DImageIOPlugin : public C2DRGBImageIOPlugin {
public:
	CJpegRGB2DImageIOPlugin();
private:
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const CRGB2DImage& data) const;
	const string do_get_descr() const;

	bool do_save_jpeg(FILE *f, const CRGB2DImage& image) const; 
};

CJpegRGB2DImageIOPlugin::CJpegRGB2DImageIOPlugin():
	C2DRGBImageIOPlugin("jpg")
{
	add_supported_type(it_ubyte);
	add_suffix(".jpg");
	add_suffix(".JPG");
	add_suffix(".jpeg");
	add_suffix(".JPEG");
}

METHODDEF(void) mia_jpeg_load_error_exit (j_common_ptr /*cinfo*/)
{
	throw create_exception<runtime_error>("JpegRGB::load: error reading from input file"); 
}

METHODDEF(void) mia_jpeg_save_error_exit (j_common_ptr /*cinfo*/)
{
	throw create_exception<runtime_error>("JpegRGB::save: error reading from input file"); 
}


C2DRGBImageIOPlugin::PData CJpegRGB2DImageIOPlugin::do_load(const string& fname) const
{

	CInputFile f(fname);
	if (!f)
		throw create_exception<runtime_error>("CJpegRGB2DImageIO::save:unable to open input file '", fname, "'");

	miajpeg::JpegDecompress decompress; 
	decompress.err.error_exit = mia_jpeg_load_error_exit;
	
	jpeg_stdio_src(&decompress.info, f);
	
	// is it a jpeg image? 
	if (jpeg_read_header(&decompress.info, TRUE) != JPEG_HEADER_OK) 
		return C2DRGBImageIOPlugin::PData(); 

	jpeg_start_decompress(&decompress.info);
	// only one component? 
	if (decompress.info.output_components != 3) {
		throw create_exception<runtime_error>(":MIA this plugin only supports RGB images, but got an image with ", 
					    decompress.info.output_components, " color components.");
	}


	int row_stride = decompress.info.output_width * decompress.info.output_components * 3;
	vector<JSAMPLE> buf(row_stride); 
	
	JSAMPROW buffer[1]; 
	buffer[0] = &buf[0]; 

	CRGB2DImage *result = new CRGB2DImage(C2DBounds(decompress.info.output_width, decompress.info.output_height)); 
	PRGB2DImage presult(result); 
	
	while (decompress.info.output_scanline < decompress.info.output_height) {
		(void) jpeg_read_scanlines(&decompress.info, buffer, 1);
		copy(buf.begin(), buf.end(), 
		     result->pixel() + ((decompress.info.output_scanline - 1) * row_stride));  
	}
	
	return presult; 
}

bool CJpegRGB2DImageIOPlugin::do_save_jpeg(FILE *f, const CRGB2DImage& image) const 
{

	miajpeg::JpegCompress compress; 

	jpeg_create_compress(&compress.info);
	compress.err.error_exit = mia_jpeg_save_error_exit;
	
	compress.info.image_width = image.get_size().x; 
	compress.info.image_height = image.get_size().y; 
	
	vector<JSAMPLE> samples(image.get_size().x * 3); 
	JSAMPROW scanline[1]; 
	scanline[0] = &samples[0]; 

	compress.info.input_components = 3;
	compress.info.in_color_space = JCS_RGB; 

	jpeg_set_defaults(&compress.info);
	jpeg_set_quality(&compress.info,80,1);

	jpeg_stdio_dest(&compress.info,f);
	jpeg_start_compress(&compress.info,TRUE);
	int dx = image.get_size().x * 3; 

	for (unsigned int y = 0; y < image.get_size().y; ++y) {
		copy(image.pixel() + y * dx, image.pixel() + (y + 1) * dx, scanline[0]); 
		jpeg_write_scanlines(&compress.info,scanline,1); 
	}
	jpeg_finish_compress(&compress.info);
	return true; 
}

bool CJpegRGB2DImageIOPlugin::do_save(const string& fname, const CRGB2DImage& image) const
{
	TRACE_FUNCTION; 
	COutputFile f(fname);
	if (!f)
		throw create_exception<runtime_error>("CJpegRGB2DImageIO::save:unable to open output file '", fname, "'");
	
	return do_save_jpeg(f, image);
	
	
}

const string CJpegRGB2DImageIOPlugin::do_get_descr() const
{
	return "A 2dimage io plugin for jpeg RGB images, Saving is et to 80% quality.";
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	return new CJpegRGB2DImageIOPlugin();
}



NS_END
