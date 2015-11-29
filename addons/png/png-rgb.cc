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

#include <png.h>
#include <sstream>
#include <cassert>
#include <boost/shared_array.hpp>
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
//#include <mia/core/errormacro.hh>
#include <mia/2d/rgbimageio.hh>



NS_BEGIN(RGBIMAGEIO_2D_PNG)


using namespace mia;
using namespace std;
using namespace boost;

class CPNG2DImageIOPlugin : public C2DRGBImageIOPlugin {
public:
	CPNG2DImageIOPlugin();
private:
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
};

CPNG2DImageIOPlugin::CPNG2DImageIOPlugin():
	C2DRGBImageIOPlugin("png")
{
	add_supported_type(it_ubyte);
	add_property(io_plugin_property_has_attributes);
	add_suffix(".png");
	add_suffix(".PNG");

}

CPNG2DImageIOPlugin::PData  CPNG2DImageIOPlugin::do_load(const string& fname) const
{
	CInputFile f(fname);
	if (!f) {
		cvdebug() << fname << " not found\n";
		return CPNG2DImageIOPlugin::PData();
	}

	const size_t sig_number = 8;
	png_byte header[sig_number];

	if (fread(header, 1, sig_number, f) != sig_number) {
		return CPNG2DImageIOPlugin::PData();
	}

	if (png_sig_cmp(header, 0, sig_number)) {
		cvdebug() << "png: signature not found\n";
		return CPNG2DImageIOPlugin::PData();
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,NULL, NULL);
	if (!png_ptr)
		throw runtime_error("unable to allocate png read_struct");

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr){
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		throw runtime_error("unable to allocate png info ptr");
	}

	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		throw runtime_error("unable to allocate png end info ptr");
	}


	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr,&end_info);
		throw runtime_error("Error reading png file");
	}

	png_init_io(png_ptr, f);
	png_set_sig_bytes(png_ptr, sig_number);
	png_set_packing(png_ptr);
	png_read_info(png_ptr, info_ptr);

	png_uint_32 width;
	png_uint_32 height;
	int bit_depth;
	int color_type;
	int interlace_type;

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		     &interlace_type, NULL, NULL);

	if (color_type != PNG_COLOR_TYPE_RGB) {
		png_destroy_read_struct(&png_ptr, &info_ptr,&end_info);
		stringstream errmsg;
		errmsg << "CPNG2DImageIO::load: image type is '";
		switch (color_type) {
		case PNG_COLOR_TYPE_GRAY:       errmsg << "GRAY"; break;
		case PNG_COLOR_TYPE_RGB_ALPHA:  errmsg << "RGBA";break;
		case PNG_COLOR_TYPE_GRAY_ALPHA: errmsg << "GRAY+ALPHA";break;
		case PNG_COLOR_TYPE_PALETTE: errmsg << "COLOR+PALETTE";break;
		default: assert(!"RGB and PALETTE should not be rejected");
		}
		errmsg <<  "' but only support RGB images are supported.";
		throw invalid_argument(errmsg.str());
	}

	if (bit_depth != 24)
		throw create_exception<invalid_argument>("PNG-RGB: Only 24-bit images are supported, but got ",  bit_depth, " bit");

	png_read_update_info(png_ptr, info_ptr);
	
	shared_array<png_bytep> row_pointers(new png_bytep[height]);

	size_t row_length = png_get_rowbytes(png_ptr, info_ptr);

	cvdebug() << "png: image = " << width << "x" << height << "@" << bit_depth << "\n";
	cvdebug() << "png:row_length = " << row_length <<"\n";

	for (png_uint_32 row = 0; row < height; row++) {
		row_pointers[row] = static_cast<png_bytep>(png_malloc(png_ptr, row_length));
	}

	png_read_image(png_ptr, row_pointers.get());
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	C2DBounds size(width, height); 
	CRGB2DImage *result = new CRGB2DImage(size); 
	auto p = result->pixel(); 
	
	for (png_uint_32 row = 0; row < height; row++, p+=row_length) {
		copy(row_pointers[row], row_pointers[row] + row_length, p); 
		free(row_pointers[row]);
	}

	return CPNG2DImageIOPlugin::PData(result);

}





bool CPNG2DImageIOPlugin::do_save(const string& fname, const CRGB2DImage& image) const
{
	TRACE_FUNCTION; 

	COutputFile f(fname);
	if (!f) {
		cverr() << "CPNG2DImageIO::save:unable to open output file:" << fname << "\n";
		return false;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
						      NULL, NULL, NULL);
	
	if (!png_ptr)
		throw runtime_error("unable to create png write struct");
	
	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr,  NULL);
		throw runtime_error("unable to create png info struct");
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		/* If we get here, we had a problem reading the file */
		png_destroy_write_struct(&png_ptr, &info_ptr);
		throw runtime_error("Error writing png file");
	}

	png_init_io(png_ptr, f);


	png_set_IHDR(png_ptr, info_ptr, image.get_size().x, image.get_size().y,
		     8, PNG_COLOR_TYPE_RGB,
		     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);


	png_write_info(png_ptr,info_ptr);

	png_set_packing(png_ptr);
	
	int line_stride = image.get_size().x * 3; 
	auto p = image.pixel(); 

	vector<unsigned char> buffer(line_stride);
	for (auto i = 0u; i < image.get_size().y; i++, p += line_stride) {
		copy(p, p + line_stride, buffer.begin()); 
		png_write_row(png_ptr, &buffer[0]);
	}
	
	png_write_end(png_ptr,info_ptr);
	
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return true; 
}

const string CPNG2DImageIOPlugin::do_get_descr() const
{
	return "a 2d RGB image io plugin for png images";
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	return new CPNG2DImageIOPlugin();
}

NS_END
