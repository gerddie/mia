/*  -*- mia-c++ -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
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

// $Id: bmp_2dimage_io.cc,v 1.12 2006-03-16 12:48:38 wollny Exp $

/*! \brief plugin function to handle bmp grayscale images

\file bmp.cc
\author Gert Wollny <wollny@cbs.mpg.de>

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <sstream>
#include <stdexcept>
#include <cassert>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/2dimageio.hh>

NS_BEGIN(BMPIO)

NS_MIA_USE
using namespace std;
using namespace boost;


enum EDataType {dt_8bit, dt_16bit, dt_32bit};
enum ECompression {c_none = 0,
		   c_8bit_rle,
		   c_4bit_rle,
		   c_rgb_mask};



class CBMP2DImageIO: public C2DImageIOPlugin {
public:
	CBMP2DImageIO();

	friend struct CImageSaver;
#pragma pack (1)
	typedef struct {
		unsigned char magic[2];
		unsigned int size;
		unsigned short int reserved1, reserved2;
		unsigned int offset;
	} BMPHeader;

	typedef struct {
		unsigned int size;               /* Header size in bytes      */
		int width,height;                /* Width and height of image */
		unsigned short int planes;       /* Number of colour planes   */
		unsigned short int bits;         /* Bits per pixel            */
		unsigned int compression;        /* Compression type          */
		unsigned int imagesize;          /* Image size in bytes       */
		int xresolution,yresolution;     /* Pixels per meter          */
		unsigned int ncolours;           /* Number of colours         */
		unsigned int importantcolours;   /* Important colours         */
	} BMPInfoHeader;
#pragma pack (8)
private:
	void do_add_suffixes(multimap<string, string>& map) const;
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
};


#ifdef WORDS_BIGENDIAN
        inline void ENDIANADAPT(short& x) { x = ((x & 0xFF) << 8)| ((x >> 8) & 0xFF);}
        inline void ENDIANADAPT(unsigned short& x) { x = ((x & 0xFF) << 8)| ((x >> 8) & 0xFF);}
        inline void ENDIANADAPT(int& x) { x = ((x & 0xFF) << 24)|((x & 0xFF00) << 8)|
                        ((x & 0xFF0000) >> 8)| ((x >> 24) & 0xFF);}
        inline void ENDIANADAPT(unsigned int& x) { x = ((x & 0xFF) << 24)|((x & 0xFF00) << 8)|
                        ((x & 0xFF0000) >> 8)| ((x >> 24) & 0xFF);}

#else
#define ENDIANADAPT(x)
#endif


CBMP2DImageIO::CBMP2DImageIO():
	C2DImageIOPlugin("bmp")
{
	add_supported_type(it_ushort);
	add_supported_type(it_ubyte);
	add_supported_type(it_bit);
}

void CBMP2DImageIO::do_add_suffixes(multimap<string, string>& map) const
{
	map.insert(pair<string,string>(".bmp", get_name()));
	map.insert(pair<string,string>(".BMP", get_name()));
}

template <class Iterator>
void bits2bytes_msb(int in, Iterator& out, int imax)
{
	if (in != 0) {
		int mask = 0x0080;
		for (int i = 0; i < imax; ++i, mask >>= 1, ++out) {
			*out = ((mask & in) != 0);
		}
	}else
		for (int i = 0; i < imax; ++i, ++out)
			*out = 0;

}


static P2DImage  read_bit_pixels(CFile& image, unsigned int width, unsigned int height)
{
        TRACE("read_bit_pixels");
        C2DBitImage *result = new C2DBitImage(C2DBounds(width, height));

	for (int y = height-1; y >= 0; --y) {
		int row_count = 0;
		C2DBitImage::iterator  p = result->begin() + width * y;
		for (size_t x = 0; x < width; x += 8, ++row_count) {
			int inbyte = fgetc(image);
			cverb << inbyte << " ";
			bits2bytes_msb(inbyte, p, min(8, int(width - x)));
		}
		// eat the padding bytes
		int remain = (4 - row_count % 4) % 4;
		while (remain--) {
			cverb << fgetc(image) << " ";
		}
	}
        return P2DImage(result);
}

static P2DImage  read_4bit_pixels(CFile& image, unsigned int width, unsigned int height)
{
	C2DBitImage *result = new C2DBitImage(C2DBounds(width, height));
	cvdebug() << "read_4bit_pixels\n";

	for (int y = height-1; y >= 0; --y) {
		int row_count = 0;
		C2DBitImage::iterator p = result->begin() + width * y;
		for (size_t x = 0; x < width; x += 2, ++row_count) {
			int inbyte = fgetc(image);
			*p++ = (inbyte >> 4) & 0xF;
			*p++ = inbyte & 0xF;
		}
		// eat the padding bytes
		int remain = (4 - row_count % 4) % 4;
		while (remain--) {
			fgetc(image);
		}
	}

	return P2DImage(result);
}

static P2DImage  read_8bit_pixels_uc(CFile& image, unsigned int width, unsigned int height)
{
	C2DUBImage *result = new C2DUBImage(C2DBounds(width, height));
	int row_count =  (4 - width % 4) % 4;
	cvdebug() << "read_8bit_pixels_nc\n";

	for (int y = height-1; y >= 0; --y) {
		if (fread( &(*result)(0,y), width, 1, image) != 1){
			throw runtime_error("BPM::Load: incomplete image");
		}

		int remain = row_count;
		while (remain--) {
			fgetc(image);
		}
	}
	cvdebug() << "read_8bit_pixels_nc done\n";
	return P2DImage(result);
}

static P2DImage  read_8bit_pixels_c(CFile& image, unsigned int width, unsigned int height)
{
        TRACE("read_8bit_pixels_c");
        C2DUBImage *result = new C2DUBImage(C2DBounds(width, height));


	int x = 0;
	int y = height-1;
	while ( y >= 0 )  {


		int n = fgetc(image);
		int c = fgetc(image);

		if (n == 0) {
			switch (c) {

			case 0: --y;
				x = 0;
				break;

			case 1: y = -1;
				break;

			case 2:
				x += fgetc(image);
				y -= fgetc(image);
				break;

			default: {
				bool odd = c & 1;
				while ( c-- )
					(*result)(x++,y) = fgetc(image);
				if (odd)
					fgetc(image);
			}
			}
		}else{
			while ( n-- )
				(*result)(x++,y) = c;
		}
	}

	return P2DImage(result);
}


static P2DImage  read_16bit_pixels(CFile& image, unsigned int width, unsigned int height)
{
	C2DUSImage *result = new C2DUSImage(C2DBounds(width, height));
	int row_count = width % 2;
	cvdebug() << "read_16bit_pixels\n";

	for (int y = height-1; y >= 0; --y) {
		unsigned short *p = &(*result)(0,y);
		if (fread( p, width, 2, image) != 2)
			throw runtime_error("BMP::Load: incomplete image");
#ifdef WORDS_BIGENDIAN
		for (size_t x = 0; x < width; ++x, ++p)
			ENDIANADAPT(*p);
#endif
		int remain = row_count;
		while (remain--) {
			fgetc(image);
			fgetc(image);
		}
	}
	return P2DImage(result);
}



static P2DImage  read_24bit_pixels(CFile& image, unsigned int width, unsigned int height)
{
	cvdebug() << "read_24bit_pixels\n";

	C2DUIImage *result = new C2DUIImage(C2DBounds(width, height));

	for (int y = height-1; y >= 0; --y) {
		unsigned int *p = &(*result)(0,y);
		if (fread( p, width, 4, image) != 4)
			throw runtime_error("BMP::Load: incomplete image");
#ifdef WORDS_BIGENDIAN
		for (size_t x = 0; x < width; ++x, ++p)
			ENDIANADAPT(*p);
#endif
	}
	return P2DImage(result);
}

#ifdef WORDS_BIGENDIAN
void endian_adapt_header(CBMP2DImageIO::BMPHeader& header)
{
	ENDIANADAPT(header.size);
	ENDIANADAPT(header.offset);
}

void endian_adapt_info_header(CBMP2DImageIO::BMPInfoHeader& header)
{
	ENDIANADAPT(header.size);
	ENDIANADAPT(header.width);
	ENDIANADAPT(header.height);                /* Width and height of image */
	ENDIANADAPT(header.planes);       /* Number of colour planes   */
	ENDIANADAPT(header.bits);         /* Bits per pixel            */
	ENDIANADAPT(header.compression);        /* Compression type          */
	ENDIANADAPT(header.imagesize);          /* Image size in bytes       */
	ENDIANADAPT(header.xresolution);
	ENDIANADAPT(header.yresolution);     /* Pixels per meter          */
	ENDIANADAPT(header.ncolours);           /* Number of colours         */
	ENDIANADAPT(header.importantcolours);
}

#endif


CBMP2DImageIO::PData CBMP2DImageIO::do_load(string const& filename)const
{
	int read = 0;
	CInputFile f(filename);
	if (!f)
		return PData();

	BMPHeader header;
	BMPInfoHeader info_header;

	cvdebug() << "CBMP2DImageIO::load\n";

	read = fread(&header, sizeof(BMPHeader), 1, f);
	if (header.magic[0] != 'B' || header.magic[1] != 'M' || read != 1)
		return PData();

#ifdef WORDS_BIGENDIAN
	endian_adapt_header(header);
#endif


	cvdebug() << "read header\n";

	read = fread(&info_header, sizeof(BMPInfoHeader), 1, f);
	if (read != 1) {
                THROW(runtime_error, "CBMP2DImageIO::load: unable to read info header from '"
                << filename << "'");
        }

	cvdebug() << "read info header";

#ifdef WORDS_BIGENDIAN
	endian_adapt_info_header(info_header);
#endif


	if (info_header.size != 40)
		throw runtime_error("CBMP2DImageIO::load: incompatible header size");

	cvdebug() << "validated infor header size\n";

	if (info_header.bits <= 8) {
		// eat the palette
		size_t palette_size = (1 << info_header.bits);
		cvdebug() << "Read palette of size " << palette_size << "\n";
		vector<char> buffer(palette_size * 4);
		if (fread(&buffer[0], palette_size, 4, f) != 4) {
			throw runtime_error("bmp: error reading palette");
		}
	}

	PData result = PData(new C2DImageVector());


	if (info_header.compression == c_none ) {

		switch (info_header.bits) {
		case 1: result->push_back(read_bit_pixels(f, info_header.width, info_header.height));
			break;
		case 4: result->push_back(read_4bit_pixels(f, info_header.width, info_header.height));
			break;
		case 8: result->push_back(read_8bit_pixels_uc(f, info_header.width, info_header.height));
			break;
		case 16:result->push_back(read_16bit_pixels(f, info_header.width, info_header.height));
			break;
		case 24:result->push_back(read_24bit_pixels(f, info_header.width, info_header.height));
			break;
		default: {
			stringstream errmsg;
			errmsg << "CBMP2DImageIO::load: unsupported pixel size: " << info_header.bits;
			throw runtime_error(errmsg.str());
		}
		} // end switch
	}else{
		switch (info_header.bits) {
		case 8: result->push_back(read_8bit_pixels_c(f, info_header.width, info_header.height));
			break;
		default:
			throw runtime_error("CBMP2DImageIO::load: compressed images not (yet) supported");
		}
	}
	cvdebug() << "CBMP2DImageIO::load done\n";
	return result;
}

struct CImageSaver {
	typedef bool result_type;

	CImageSaver(CFile& f):
		_M_f(f)
	{
	}

	template <typename Data2D>
	result_type operator ()(const Data2D& image) const;
private:
	bool write_header(int bpp, int compression, const C2DBounds& size)const;

	CFile& _M_f;
};

template <typename Image2D>
struct pixel_trait {
	enum { pixel_size = 8 * sizeof(typename Image2D::value_type)};
	enum { supported = 0 };
};

template <>
struct pixel_trait<C2DBitImage> {
	enum { pixel_size = 1 };
	enum { supported = 1 };
};

template <>
struct pixel_trait<C2DUIImage> {
	enum { pixel_size = 24 };
	enum { supported = 1 };
};

template <>
struct pixel_trait<C2DUBImage> {
	enum { pixel_size = 8 };
	enum { supported = 1 };
};

template <>
struct pixel_trait<C2DUSImage> {
	enum { pixel_size = 16 };
	enum { supported = 1 };
};

template <typename Image2D, int supported>
struct image_writer {
	static bool apply (const Image2D& /*image*/, FILE */*f*/) {
		throw runtime_error("Pixel type not supported by file format");
	}
};

template <typename Image2D>
struct image_writer<Image2D, 1>  {
	static bool apply (const Image2D& image, FILE *f) {
		cvdebug() << "image_writer<Image2D, 1>::apply image size ("<< image.get_size().x << ", " << image.get_size().y<<")\n";

		const unsigned int pixel_size = sizeof(typename Image2D::value_type);
		int remain = (image.get_size().x  *  pixel_size) % 4;
		if (remain)
			remain = 4 - remain;

		vector<char> buf(remain, 0);


		vector<unsigned char> transfer(image.get_size().x  * pixel_size);

		for (int y = image.get_size().y - 1; y >= 0; --y) {

			vector<unsigned char>::iterator out = transfer.begin();
			typename Image2D::const_pointer p = &image(0,y);
			for (size_t x = 0; x <  image.get_size().x; ++x, ++p){
				int mask = 0xFF;
				for (unsigned int n = 0; n < pixel_size; ++n, mask <<= 8, ++out)
					*out = (*p & mask) >> n * 8;
			}

			if (fwrite(&transfer[0], image.get_size().x, pixel_size, f) != pixel_size) {
				cverr() << "unable to write whole image\n";
				return false;
			}

			if (remain)
				if (fwrite(&buf[0],  remain, 1,  f) != 1)
					return false;
		}

		cvdebug() << "image_writer<Image2D, 1>::apply done\n";
		return true;
	}
};

template <>
struct image_writer<C2DBitImage, 1> {
	enum { pixel_size = 1};
	static bool apply (const C2DBitImage& image, FILE *f) {

		vector<unsigned char> buf( 4 * ((image.get_size().x + 31)  / 32));
		fill(buf.begin(), buf.end(), 0);


		for (int y = image.get_size().y-1; y >= 0; --y) {
			C2DBitImage::const_iterator pixel_i = image.begin() + y * image.get_size().x;
			vector<unsigned char>::iterator out = buf.begin();

			size_t x = 0;
			while ( x < image.get_size().x ) {

				unsigned int mask = 0x80;
				*out = 0;

				for (int k = 0; k < 8 && x < image.get_size().x; ++k, ++pixel_i, ++x, mask >>= 1 )
					if (*pixel_i)
						*out |= mask;

				++out;

			}

			if (fwrite(&buf[0], buf.size(), 1, f) != 1)
				return false;


		}
		return true;
	}
};

bool CImageSaver::write_header(int bpp, int compression, const C2DBounds& size) const
{
	cvdebug() << "CImageSaver::write_header("<< bpp << ", "<< compression << ", " << "(" << size.x<< ", "<< size.y<< ") )\n";
	CBMP2DImageIO::BMPHeader header;
	int palette_size = bpp > 8 ? 0 : (1 << bpp) * 4;

	header.magic[0] = 'B';
	header.magic[1] = 'M';
	header.reserved1 = header.reserved2 = 0;
	header.offset = sizeof(CBMP2DImageIO::BMPHeader) + sizeof(CBMP2DImageIO::BMPInfoHeader) + palette_size;
	header.size = header.offset + size.y * ( ((size.x + 3) & 0xFFFFFFE0) / 8);

	CBMP2DImageIO::BMPInfoHeader info_header;

	info_header.size = sizeof(CBMP2DImageIO::BMPInfoHeader);
	info_header.width = size.x;
	info_header.height = size.y;
	info_header.planes = 1;
	info_header.bits = bpp;
	info_header.compression = compression;
	info_header.imagesize = 0;
	info_header.xresolution = 1;
	info_header.yresolution = 1;
	unsigned int ncolours = info_header.ncolours = 1 << bpp;
	info_header.importantcolours = 0;

#ifdef WORDS_BIGENDIAN
	endian_adapt_header(header);
	endian_adapt_info_header(info_header);
#endif

	if (fwrite(&header, sizeof(CBMP2DImageIO::BMPHeader), 1, _M_f) != 1)
		return false;
	if (fwrite(&info_header, sizeof(CBMP2DImageIO::BMPInfoHeader), 1, _M_f) != 1)
		return false;

	if (bpp <= 8) {
		cvdebug() << "write palette of size " << ncolours << "\n";
		vector<int> palette(ncolours);
		int black = 0;
#ifdef WORDS_BIGENDIAN
		int step = bpp == 8 ? 0x01010100 : (bpp == 4 ? 0x11111100 : 0xFFFFFF00);
#else
		int step = bpp == 8 ? 0x010101 : (bpp == 4 ? 0x111111 : 0xFFFFFF);
#endif
		for (size_t i = 0; i < ncolours; ++i, black+=step)
			palette[i] = black;

		if (fwrite(&palette[0], sizeof(int), ncolours , _M_f) != ncolours)
			return false;
	}

	return true;
}

template <typename Image2D>
CImageSaver::result_type
CImageSaver::operator()(const Image2D& image) const
{
	if (!write_header(pixel_trait<Image2D>::pixel_size, 0, image.get_size()))
		return false;

	return image_writer<Image2D, pixel_trait<Image2D>::supported>::apply(image,_M_f);
}



///

bool CBMP2DImageIO::do_save(string const& filename, const C2DImageVector& data) const
{
	cvdebug() << "CBMP2DImageIO::save begin\n";

	COutputFile f(filename);
	if (!f) {
		cverr() << "CBMP2DImageIO::save:unable to open output file:" << filename << "\n";
		return false;
	}

	CImageSaver saver(f);

	for (C2DImageVector::const_iterator iimg = data.begin(); iimg != data.end(); ++iimg)
		filter(saver, **iimg);

	cvdebug() << "CBMP2DImageIO::save end\n";
	return true;
}

const string  CBMP2DImageIO::do_get_descr()const
{
	return string("BMP 2D-image input/output support");
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
		return new CBMP2DImageIO;
}


NS_END
