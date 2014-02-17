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

#include <png.h>
#include <sstream>
#include <cassert>
#include <boost/shared_array.hpp>
#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
//#include <mia/core/errormacro.hh>
#include <mia/2d/imageio.hh>



NS_BEGIN(IMAGEIO_2D_PNG)


using namespace mia;
using namespace std;
using namespace boost;

class CPNG2DImageIOPlugin : public C2DImageIOPlugin {
public:
	CPNG2DImageIOPlugin();
private:
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
};

CPNG2DImageIOPlugin::CPNG2DImageIOPlugin():
	C2DImageIOPlugin("png")
{
	add_supported_type(it_bit);
	add_supported_type(it_ubyte);
	add_supported_type(it_ushort);
	add_property(io_plugin_property_has_attributes);
	add_suffix(".png");
	add_suffix(".PNG");
}

template <typename T>
std::shared_ptr<T2DImage<T>  > read_image(int width, int height, const png_bytep *data, size_t row_length, const CAttributedData& attributes)
{
	cvdebug() << "copy loaded data\n";
	T2DImage<T> *result = new T2DImage<T>(C2DBounds(width, height), attributes);

	if (width * sizeof(typename T2DImage<T>::value_type) < row_length) {
		cvwarn() << "row_length larger then image width, adjusting\n";
		row_length = width * sizeof(typename T2DImage<T>::value_type);
	}

	for (int i = 0; i < height; ++i) {
		memcpy(&*result->begin_at(0,i), data[i], row_length);
	}
	return std::shared_ptr<T2DImage<T>  > (result);
}

template <>
std::shared_ptr<T2DImage<bool>  > read_image(int width, int height, const png_bytep *data, size_t row_length, const CAttributedData& attributes)
{
	cvdebug() << "copy loaded data\n";
	T2DImage<bool> *result = new T2DImage<bool>(C2DBounds(width, height), attributes);

	if (width * sizeof(bool) < row_length) {
		cvwarn() << "row_length larger then image width, adjusting\n";
		row_length = width * sizeof(bool);
	}

	for (int i = 0; i < height; ++i) {
		copy(data[i], data[i] + row_length, result->begin_at(0,i));
	}
	return std::shared_ptr<T2DImage<bool>  > (result);
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

	if (color_type != PNG_COLOR_TYPE_GRAY && color_type != PNG_COLOR_TYPE_PALETTE) {
		png_destroy_read_struct(&png_ptr, &info_ptr,&end_info);
		stringstream errmsg;
		errmsg << "CPNG2DImageIO::load: image type is '";
		switch (color_type) {
		case PNG_COLOR_TYPE_RGB:        errmsg << "RGB"; break;
		case PNG_COLOR_TYPE_RGB_ALPHA:  errmsg << "RGBA";break;
		case PNG_COLOR_TYPE_GRAY_ALPHA: errmsg << "GRAY+ALPHA";break;
		default: assert(!"GRAY and PALETTE should not be rejected");
		}
		errmsg <<  "' but only support gray scale images.";
		throw invalid_argument(errmsg.str());
	}

	if ( color_type == PNG_COLOR_TYPE_PALETTE) {
		cvwarn() << "CPNG2DImageIO::load: Palette will be ignored\n";
	}

	if (bit_depth < 8)
		png_set_packing(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

#ifndef WORDS_BIGENDIAN
	/* swap bytes of 16 bit files to least significant byte first */
	png_set_swap(png_ptr);
#endif

	shared_array<png_bytep> row_pointers(new png_bytep[height]);

	size_t row_length = png_get_rowbytes(png_ptr, info_ptr);

	cvdebug() << "png: image = " << width << "x" << height << "@" << bit_depth << "\n";
	cvdebug() << "png:row_length = " << row_length <<"\n";

	for (png_uint_32 row = 0; row < height; row++) {
		row_pointers[row] = static_cast<png_bytep>(png_malloc(png_ptr, row_length));
	}

	png_textp text;
	int ntext;

	CAttributedData attributes;
	png_get_text(png_ptr, info_ptr, &text, &ntext);
	for (int i = 0; i < ntext; ++i) {
		attributes.set_attribute(text[i].key, CStringAttrTranslatorMap::instance().to_attr(text[i].key, text[i].text));
	}


	png_read_image(png_ptr, row_pointers.get());
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	std::shared_ptr<C2DImageVector > result(new C2DImageVector);

	switch (bit_depth) {
	case 1:  result->push_back(read_image<bool>(width, height, row_pointers.get(), row_length, attributes));
		break;
	case 2:
	case 4:
	case 8:  result->push_back(read_image<unsigned char>(width, height, row_pointers.get(), row_length, attributes));
		break;
	case 16: result->push_back(read_image<unsigned short>(width, height, row_pointers.get(), row_length, attributes));
		break;
	default: {
		stringstream errmsg;
		errmsg << "png::read: unsupported bit depth " << bit_depth;
		throw invalid_argument(errmsg.str());
	}
	}

	for (png_uint_32 row = 0; row < height; row++)
		free(row_pointers[row]);

	return CPNG2DImageIOPlugin::PData(result);

}



struct CPngImageSaver: public TFilter<bool> {
	CPngImageSaver(CFile& f):
		m_f(f)
	{
	}

	template <typename T>
	CPngImageSaver::result_type operator ()(const T2DImage<T>& image)const;
private:

	CFile& m_f;
};

template <typename T>
struct pixel_trait {
	enum { pixel_size = 8 * sizeof(T)};
	enum { supported = 0 };
};

template <>
struct pixel_trait<bool> {
	enum { pixel_size = 1 };
	enum { supported = 1 };
};

template <>
struct pixel_trait<unsigned char> {
	enum { pixel_size = 8 };
	enum { supported = 1 };
};

// we treat signed images just like their unsigned counterparts
template <>
struct pixel_trait<signed char> {
	enum { pixel_size = 8 };
	enum { supported = 1 };
};

template <>
struct pixel_trait<unsigned short> {
	enum { pixel_size = 16 };
	enum { supported = 1 };
};

template <>
struct pixel_trait<signed short> {
	enum { pixel_size = 16 };
	enum { supported = 1 };
};

template <typename T>
struct png_dispatch_write{
	static vector<png_bytep> get_row_pointer(const T2DImage<T>& image) {
		vector<png_bytep> result(image.get_size().y);
		for (size_t y = 0; y < image.get_size().y; ++y)
			result[y] = (png_bytep)&image(0,y);
		return result;
	}
	static void free_row_pointers(vector<png_bytep>& /*ptr*/){
	}
};

template <>
struct png_dispatch_write<bool> {
	static vector<png_bytep> get_row_pointer(const T2DImage<bool>& image) {
		vector<png_bytep> result(image.get_size().y);

		for (size_t y = 0; y < image.get_size().y; ++y) {
			result[y] = (png_bytep)new unsigned char[image.get_size().x];
			copy(image.begin_at(0,y), image.begin_at(0,y) + image.get_size().x, result[y]);
		}
		return result;
	}
	static void free_row_pointers(vector<png_bytep>& ptr){
		for (size_t i = 0; i < ptr.size(); ++i)
			delete[] ptr[i];
	}
};


template <typename T>
CPngImageSaver::result_type CPngImageSaver::operator ()(const T2DImage<T>& image)const
{
	if (!pixel_trait<T>::supported ) {
		throw create_exception<invalid_argument>("Input pixel format '", typeid(T).name(), 
					       "' not supported by png writer");
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

	png_init_io(png_ptr, m_f);


	png_set_IHDR(png_ptr, info_ptr, image.get_size().x, image.get_size().y,
		     pixel_trait<T>::pixel_size, PNG_COLOR_TYPE_GRAY,
		     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_color_8 sig_bit;
	sig_bit.gray = pixel_trait<T>::pixel_size;
	png_set_sBIT(png_ptr, info_ptr, &sig_bit);



	vector<png_text>  text_ptr;
	
	png_text p; 

	p.key = strdup("Creator");
	p.text = strdup("MIA 2.0 png image io plugin");
	p.compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr.push_back(p); 

	for (auto iattr = image.begin_attributes(); iattr != image.end_attributes(); ++iattr) {
		png_text p; 
		p.key = strdup(iattr->first.c_str());
		p.text = strdup(iattr->second->as_string().c_str());
		p.compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr.push_back(p); 
	}

	png_set_text(png_ptr, info_ptr, &text_ptr[0], text_ptr.size());

	png_write_info(png_ptr, info_ptr);
	png_set_packing(png_ptr);

#ifndef WORDS_BIGENDIAN
	png_set_swap(png_ptr);
#endif

	typedef png_dispatch_write<T> png_dispatch;
	vector<png_bytep> row_pointers = png_dispatch::get_row_pointer(image);

	png_write_image(png_ptr, &row_pointers[0]);

	png_dispatch::free_row_pointers(row_pointers);

	png_write_end(png_ptr,info_ptr);

	for (size_t i = 0; i < text_ptr.size(); ++i) {
		free(text_ptr[i].key);
		free(text_ptr[i].text);
	}
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return true;
}

bool CPNG2DImageIOPlugin::do_save(const string& fname, const C2DImageVector& data) const
{
	TRACE_FUNCTION; 

	COutputFile f(fname);
	if (!f) {
		cverr() << "CPNG2DImageIO::save:unable to open output file:" << fname << "\n";
		return false;
	}

	CPngImageSaver saver(f);

	if (data.size() != 1)
		throw invalid_argument("PNG support exactly 1 image per file");


	return filter(saver, **data.begin());
}

const string CPNG2DImageIOPlugin::do_get_descr() const
{
	return "a 2dimage io plugin for png images";
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	return new CPNG2DImageIOPlugin();
}

NS_END
