/*  -*- mia-c++ -*-

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

// $Id: tiff_2dimage_io.cc,v 1.14 2006-06-13 10:20:54 wollny Exp $

/*! \brief plugin function to handle vista- format IO

\file inria_3dimage_io.cc
\author Gert Wollny <wollny@cbs.mpg.de>

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <sstream>
#include <tiffio.h>
#include <cassert>

#undef max
#undef min

#include <limits>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/2dimageio.hh>

#ifdef WIN32
#define snprintf _snprintf
#endif

NS_BEGIN(IMAGEIO_2D_TIFF)
NS_MIA_USE
using namespace std;
using namespace boost;

enum EDataType {dt_8bit, dt_16bit, dt_32bit};

class CTiff2DImageIO: public C2DImageIOPlugin {
public:
	CTiff2DImageIO();
private:
	void do_add_suffixes(multimap<string, string>& map) const;
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;

};


CTiff2DImageIO::CTiff2DImageIO():
	C2DImageIOPlugin("tif")
{
	add_supported_type(it_bit);
	add_supported_type(it_ushort);
	add_supported_type(it_ubyte);
}

void CTiff2DImageIO::do_add_suffixes(multimap<string, string>& map) const
{
	map.insert(pair<string,string>(".tif", get_name()));
	map.insert(pair<string,string>(".tiff", get_name()));
	map.insert(pair<string,string>(".TIF", get_name()));
	map.insert(pair<string,string>(".TIFF", get_name()));
}

typedef TIFF * PTIFF;

struct CTiffFile {
	CTiffFile(const char *name, const char *flags):
		handle(TIFFOpen(name, flags))
	{
	}

	~CTiffFile()
	{
		if (handle)
			TIFFClose(handle);
	}
	operator PTIFF() {
		return handle;
	}
private:
	TIFF *handle;
};

void MyErrorHandler(const char *module, const char *fmt,  va_list ap)
{
	char buf[16384];
	snprintf(buf,16384, fmt, ap);

	cverr() << module << ":" << buf << "\n";
	exit(1);
}


struct CErrorHandlerReplacer {
	CErrorHandlerReplacer():
		_M_old_handler(TIFFSetErrorHandler(MyErrorHandler))
	{
	}
	~CErrorHandlerReplacer() {
		TIFFSetErrorHandler(_M_old_handler);
	}
private:
	TIFFErrorHandler _M_old_handler;
};

void bits2bytes_msb(unsigned char in, C2DBitImage::iterator& out, int max_bit)
{
	if (in != 0) {
		int mask = 0x0080;
		for (int i = 0; i < max_bit; ++i, mask >>= 1, ++out) {
			*out = (mask & in) != 0;
		}
	}else
		for (int i = 0; i < max_bit; ++i, ++out)
			*out = 0;

}

void bits2bytes_lsb(unsigned char in, C2DBitImage::iterator& out, int max_bit)
{
	char mask = 0x1;
	for (int i = 0; i < max_bit; ++i, mask <<= 1, ++out)
		*out = (mask & in) == mask;
}


P2DImage  read_bit_pixels(CTiffFile& tif, unsigned int width,
				      unsigned int height, bool invert)
{
	uint32 nstrips = TIFFNumberOfStrips(tif);
	cvdebug() << "TIFFNumberOfStrips: " << nstrips << "\n";

	uint32 strip_size = TIFFStripSize(tif);
	cvdebug() << "TIFFStripSize: " << strip_size << "\n";

	C2DBitImage *image = new C2DBitImage(C2DBounds(width, height));
	P2DImage  result(image);

	if (!image) {
		stringstream errmsg;
		errmsg << "unable to allocate space for image of size " << width * height / 8;
		throw runtime_error(errmsg.str());
	}

	uint16 fillorder = 0;

	TIFFGetField(tif, TIFFTAG_FILLORDER, &fillorder);

	C2DBitImage::iterator ir = image->begin();
	vector<unsigned char> buf(strip_size);

	long read_size = -1;
	if (nstrips == 1)
		read_size = strip_size;

	for (size_t i = 0 ; i < nstrips; ++i) {
		uint32 strip_s = TIFFReadEncodedStrip(tif, i, &buf[0],  read_size);
		vector<unsigned char>::const_iterator inp = buf.begin();
		uint32 pos = 0;
		while ( pos < strip_s) {
			int xi = width;
			while (xi > 0 && pos < strip_s){
				if (fillorder == FILLORDER_MSB2LSB)
					bits2bytes_msb(*inp, ir, xi < 8 ? xi : 8);
				else
					bits2bytes_lsb(*inp, ir, xi < 8 ? xi : 8);
				xi -= 8;
				++inp;
				++pos;
			}
		}

	}

	// deal with image photometric interpretion
	if (invert) {
		C2DBitImage::iterator ir = image->begin();
		C2DBitImage::iterator ie = image->end();
		while (ir != ie) {
			*ir = !*ir;
			++ir;
		}
	}

	return result;
}

template <typename T>
P2DImage read_strip_pixels(CTiffFile& tif, unsigned int width,
				       unsigned int height, bool invert)
{

	uint32 nstrips = TIFFNumberOfStrips(tif);
	cvdebug() << "TIFFNumberOfStrips: " << nstrips << "\n";

	uint32 strip_size = TIFFStripSize(tif);
	cvdebug() << "TIFFStripSize: " << strip_size << "\n";

	T2DImage<T> *result = new T2DImage<T>(C2DBounds(width, height));
	P2DImage presult(result);
	if (!result) {
		stringstream errmsg;
		errmsg << "unable to allocate space for image of size " << width * height * sizeof(T);
		throw runtime_error(errmsg.str());
	}

	uint32 	row = 0;

	long read_size = -1;
	if (nstrips == 1)
		read_size = strip_size;

	for (size_t i = 0 ; i < nstrips; ++i) {
		uint32 strip_s = TIFFReadEncodedStrip(tif, i,  &(*result)(0,row),  read_size);
		row +=  strip_s / (sizeof(T) * width);
	}

	// deal with image photometric interpretion
	if (invert) {
		typename T2DImage<T>::iterator ii = result->begin();
		typename T2DImage<T>::iterator ie = result->end();

		while (ii != ie) {
			*ii = numeric_limits<T>::max( ) - *ii;
			++ii;
		}
	}

	return presult;
}


CTiff2DImageIO::PData CTiff2DImageIO::do_load(string const& filename)const
{
	CTiffFile tif(filename.c_str(), "r");
	if (!tif)
		return PData();

	// load that stuff
	uint32 height;
	uint32 width;
	uint16 bbs;
	uint16 spp;
	uint16 photo;

	PData result(new C2DImageVector());


	do {

		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		cvdebug() << "TIFFTAG_IMAGELENGTH:" << height << "\n";

		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		cvdebug() << "TIFFTAG_IMAGEWIDTH:" << width << "\n";

		TIFFGetField(tif,TIFFTAG_BITSPERSAMPLE, &bbs);
		cvdebug() << "TIFFTAG_BITSPERSAMPLE:" << bbs << "\n";

		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
		cvdebug() << "TIFFTAG_SAMPLESPERPIXEL:" << spp << "\n";

		if (spp != 1) {
			stringstream errmsg;
			errmsg << "CTiff2DImageIO::load: support only one sample per pixel.";
			throw runtime_error(errmsg.str());
		}

		TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photo);
		if (photo !=PHOTOMETRIC_MINISWHITE && photo != PHOTOMETRIC_MINISBLACK && photo != PHOTOMETRIC_PALETTE) {
			stringstream errmsg;
			errmsg << "CTiff2DImageIO::load: Support only gray scale images.";
			throw runtime_error(errmsg.str());
		}
		if (photo == PHOTOMETRIC_PALETTE)
			cvwarn() << "CTiff2DImageIO::load: Palette will be ignored, loading image as gray scale\n";

		// check if we have a tiled image
		if ( TIFFIsTiled(tif) ) {
			cvdebug() << "tiled image!\n";
			throw runtime_error("CTiff2DImageIO::load: currently no support for tiled images");
		}

		switch (bbs) {
		case  1: result->push_back(read_bit_pixels(tif, width, height, photo == 0)); break;
		case  8: result->push_back(read_strip_pixels<C2DUBImage::value_type>(tif, width, height, photo == 0));
			break;
		case 16: result->push_back(read_strip_pixels<C2DUSImage::value_type>(tif, width, height, photo == 0));
			break;
		case 32: result->push_back(read_strip_pixels<C2DUIImage::value_type>(tif, width, height, photo == 0));
			break;
		default: {
			stringstream errmsg;
			errmsg << "CTiff2DImageIO::load: unsupported BitPerPixel " << bbs;
			throw runtime_error(errmsg.str());
		}
		}
	}while (TIFFReadDirectory(tif));

	return result;
}

template <typename Data2D>
struct __dispatch_saver {
	static uint32 get_bits_per_sample() {
		return sizeof(typename Data2D::value_type) * 8;
	}
	static uint32 get_slice_length(uint32 width) {
		uint32 rows_per_strip = 8192 / (width * sizeof(typename Data2D::value_type));
		return (rows_per_strip > 4) ? rows_per_strip : 4;
	}
	static void write(CTiffFile& tif, const Data2D& image, uint32 rows_per_strip) {

		TIFFSetField(tif, TIFFTAG_COMPRESSION,  COMPRESSION_NONE);
		cvdebug() << "__dispatch_saver::write begin\n";
		cvdebug() << "rows per strip: " << rows_per_strip << "\n";

		vector<typename Data2D::value_type> buf(rows_per_strip * image.get_size().x);

		typename Data2D::const_iterator cbuf_begin = image.begin();
		typename Data2D::const_iterator cbuf_end = cbuf_begin;

		int nstrips = (image.get_size().y + rows_per_strip - 1) / rows_per_strip;

		for (int i = 0; i < nstrips; ++i) {

			uint32 cur_rows = rows_per_strip;

			// last strip might be shorter
			if (i == nstrips - 1)
				cur_rows = image.get_size().y - i * rows_per_strip;

			uint32 samples_to_push = cur_rows * image.get_size().x;

			// copy samples to temporary buffer
			advance(cbuf_end, samples_to_push);
			copy(cbuf_begin, cbuf_end, buf.begin());
			cbuf_begin = cbuf_end;

			// write the stuff
			TIFFWriteEncodedStrip(tif, i, &buf[0], samples_to_push * sizeof(typename Data2D::value_type));

		}

		cvdebug() << "__dispatch_saver::write end\n";
	}

};

template <>
struct __dispatch_saver<C2DBitImage> {
	static uint32  get_bits_per_sample() {
		return 1;
	}
	static uint32 get_slice_length(uint32 width) {
		if (!width) {
			assert(!"try to save an image of width 0");
		}
		uint32 rows_per_strip = 8192 / ( ( width +7 ) / 8);
		return (rows_per_strip > 4) ? rows_per_strip : 4;
	}
	static void write(CTiffFile& tif, const C2DBitImage& image, uint32 rows_per_strip) {

		TIFFSetField(tif,  TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
		TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

		uint32 linelength = (image.get_size().x + 7)/ 8;

		vector<unsigned char> buf(rows_per_strip * linelength);
		C2DBitImage::const_iterator cbuf_begin = image.begin();

		// then push out the slices
		// evaluate the number of strips:
		int nstrips = (image.get_size().y + rows_per_strip - 1) / rows_per_strip;

		for (int i = 0; i < nstrips; ++i) {
			uint32 cur_rows = i < nstrips - 1 ? rows_per_strip : image.get_size().y - i * rows_per_strip;

			uint32 samples_to_push = cur_rows * linelength;

			fill(buf.begin(), buf.end(), false);
			vector<unsigned char>::iterator ibuf = buf.begin();

			for (size_t row = 0; row < cur_rows; ++row) {
				size_t x = 0;
				while ( x < image.get_size().x) {
					int mask = 0x80;
					for (size_t bit = 0; bit < 8 && x < image.get_size().x; ++bit, mask>>=1, ++cbuf_begin, ++x) {
						if (*cbuf_begin)
							*ibuf |= mask;
					}
					++ibuf;
				}
			}

			// write the stuff
			TIFFWriteEncodedStrip(tif, i, &buf[0], samples_to_push);
		}
	}
};


struct CTiffImageSaver {
	typedef void result_type;

	CTiffImageSaver(CTiffFile& tif, int nimages):
		_M_tif(tif),
		_M_nimages(nimages),
		_M_counter(0)
	{
	}

	template <typename Data2D>
	result_type operator ()(const Data2D& image)const
	{
		typedef __dispatch_saver<Data2D> dispatcher;

		cvdebug() << "CImageSaver::() begin\n";

		TIFFSetField(_M_tif, TIFFTAG_IMAGEWIDTH, image.get_size().x);
		TIFFSetField(_M_tif, TIFFTAG_IMAGELENGTH, image.get_size().y);

		uint32 rows_per_strip = dispatcher::get_slice_length(image.get_size().x);
		TIFFSetField(_M_tif, TIFFTAG_BITSPERSAMPLE, dispatcher::get_bits_per_sample());
		TIFFSetField(_M_tif, TIFFTAG_SAMPLESPERPIXEL, 1);
		TIFFSetField(_M_tif, TIFFTAG_ROWSPERSTRIP, rows_per_strip);
		TIFFSetField(_M_tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		TIFFSetField(_M_tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(_M_tif, TIFFTAG_XRESOLUTION, 120.0);
		TIFFSetField(_M_tif, TIFFTAG_YRESOLUTION, 120.0);
		TIFFSetField(_M_tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

		if (_M_nimages != 1) {
			/* We are writing single page of the multipage file */
			TIFFSetField(_M_tif, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
			/* Set the page number */
			TIFFSetField(_M_tif, TIFFTAG_PAGENUMBER, _M_counter++, _M_nimages);
		}
		dispatcher::write(_M_tif, image, rows_per_strip);

		TIFFWriteDirectory(_M_tif);
		cvdebug() << "CImageSaver::() end\n";
	}
private:
	CTiffFile& _M_tif;
	int _M_nimages;
	mutable int _M_counter;
};


bool CTiff2DImageIO::do_save(string const& filename, const C2DImageVector& data)const
{
	cvdebug() << "CTiff2DImageIO::save begin\n";

	CErrorHandlerReplacer error_handing;

	CTiffFile tif(filename.c_str(), "w");
	if (!tif) {
		cverr() << "CTiff2DImageIO::save:unable to open output file:" << filename << "\n";
		return false;
	}

	CTiffImageSaver saver(tif, data.size());

	for (C2DImageVector::const_iterator iimg = data.begin(); iimg != data.end(); ++iimg)
		filter(saver, **iimg);

	cvdebug() << "CTiff2DImageIO::save end\n";
	return true;
}

const string  CTiff2DImageIO::do_get_descr()const
{
	return string("TIFF 2D-image input/output support");
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
		return new CTiff2DImageIO;
}

NS_END

