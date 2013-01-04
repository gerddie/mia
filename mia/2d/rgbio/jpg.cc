/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <cstdio>
#include <cstdlib>
#include <jpeglib.h>
#include <sstream>
#include <cassert>
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/rgbimageio.hh>

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

METHODDEF(void) mia_jpeg_error_exit (j_common_ptr /*cinfo*/)
{
	throw create_exception<runtime_error>("Jpeg::load: error reading from input file"); 
}


C2DRGBImageIOPlugin::PData CJpegRGB2DImageIOPlugin::do_load(const string& fname) const
{

	CInputFile f(fname);
	if (!f)
		throw create_exception<runtime_error>("CPNG2DImageIO::save:unable to open input file '", fname, "'");

	struct jpeg_decompress_struct cdecompress_info;
	struct jpeg_error_mgr jerr;
	
	cdecompress_info.err = jpeg_std_error(&jerr);

	jerr.error_exit = mia_jpeg_error_exit;
 
	jpeg_create_decompress(&cdecompress_info);
	
	jpeg_stdio_src(&cdecompress_info, f);
	
	// is it a jpeg image? 
	if (jpeg_read_header(&cdecompress_info, TRUE) != JPEG_HEADER_OK) 
		return C2DRGBImageIOPlugin::PData(); 

	(void) jpeg_start_decompress(&cdecompress_info);
	// only one component? 
	if (cdecompress_info.output_components != 3) {
		jpeg_destroy_decompress(&cdecompress_info);
		throw create_exception<runtime_error>(":MIA this plugin only supports RGB images, but got an image with ", 
					    cdecompress_info.output_components, " color components.");
	}


	int row_stride = cdecompress_info.output_width * cdecompress_info.output_components * 3;
	vector<JSAMPLE> buf(row_stride); 
	
	JSAMPROW buffer[1]; 
	buffer[0] = &buf[0]; 

	CRGB2DImage *result = new CRGB2DImage(C2DBounds(cdecompress_info.output_width, cdecompress_info.output_height)); 
	PRGB2DImage presult(result); 
	
	while (cdecompress_info.output_scanline < cdecompress_info.output_height) {
		(void) jpeg_read_scanlines(&cdecompress_info, buffer, 1);
		copy(buf.begin(), buf.end(), 
		     result->pixel() + ((cdecompress_info.output_scanline - 1) * row_stride));  
	}
	
	jpeg_destroy_decompress(&cdecompress_info);
	return presult; 
}

bool CJpegRGB2DImageIOPlugin::do_save_jpeg(FILE *f, const CRGB2DImage& image) const 
{

	struct jpeg_compress_struct cinfo; 
	struct jpeg_error_mgr       jerr; 

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	
	cinfo.image_width = image.get_size().x; 
	cinfo.image_height = image.get_size().y; 
	
	vector<JSAMPLE> samples(image.get_size().x * 3); 
	JSAMPROW scanline[1]; 
	scanline[0] = &samples[0]; 

	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB; 

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo,80,1);

	jpeg_stdio_dest(&cinfo,f);
	jpeg_start_compress(&cinfo,TRUE);
	int dx = image.get_size().x * 3; 

	for (unsigned int y = 0; y < image.get_size().y; ++y) {
		copy(image.pixel() + y * dx, image.pixel() + (y + 1) * dx, scanline[0]); 
		jpeg_write_scanlines(&cinfo,scanline,1); 
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
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
