/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <mia/2d/imageio.hh>

NS_BEGIN(IMAGEIO_2D_BMP)

NS_MIA_USE
using namespace std;
using namespace boost;


enum ECompression {c_none = 0,
		   c_8bit_rle,
		   c_4bit_rle,
		   c_rgb_mask};



class CBMP2DImageIO: public C2DImageIOPlugin {
public:
	CBMP2DImageIO();

	friend struct C2DBMPImageSaver;
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
	add_supported_type(it_ubyte);
	add_supported_type(it_bit);
	add_suffix(".bmp");
	add_suffix(".BMP");

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
	P2DImage presult(result); 
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
			if (fgetc(image) == EOF) 
				throw runtime_error("BPM::Load: incomplete image");
			
		}
	}
        return presult;
}

static P2DImage  read_4bit_pixels_c(CFile& image, unsigned int width, unsigned int height)
{
	C2DUBImage *result = new C2DUBImage(C2DBounds(width, height));
	P2DImage presult(result); 
	cvdebug() << "read_4bit_pixels compressed\n";

	
	int y = height-1;
	int x = 0; 
	while (y >= 0) {
		int inbyte_1 = fgetc(image);
		
		//rle encoded pixels
		if (inbyte_1 > 0) {
			int inbyte_2 = fgetc(image);
			if (inbyte_2 == EOF) 
				throw runtime_error("BPM::Load: incomplete image");
			char hi = (inbyte_2 >> 4) & 0xF;
			char lo = inbyte_2 & 0xF;
			
			while (inbyte_1--) {
				(*result)(x,y) = hi;
				cvdebug() << inbyte_1 << " x=" << x << ", y=" << y << " -> " << (int)hi << "\n"; 
				swap(hi, lo);
				++x;
			}
		}else{
			int inbyte_2 = fgetc(image);

			// literal pixels 
			if ( inbyte_2 > 2){
				vector<char> buffer( (inbyte_2 + 3) / 2);
				if (fread(&buffer[0], 1, buffer.size(), image) != buffer.size())
					throw runtime_error("BPM::Load: incomplete image");
				for (auto c = buffer.begin(); c != buffer.end() && inbyte_2 > 0; ++c) {
					(*result)(x,y) = (*c >> 4) & 0xF;
					++x; 
					inbyte_2--;
					if (inbyte_2--) {
						(*result)(x,y) = *c & 0xF;
						++x; 
					}
				}
			}else if (inbyte_2 == 0){
				cvdebug() << "End of row\n"; 
				--y;
				x = 0;
			}else if (inbyte_2 == 1){
				cvdebug() << "End of image\n"; 
				// end of image 
				break;
			}else { // if inbyte_2 == 2
				// offset
				int dx = fgetc(image);
				int dy = fgetc(image);

				if (dx == EOF || dy == EOF) {
					throw runtime_error("BPM::Load: incomplete image");
				}
				y -= dy;
				x += dx;
			}
		}
	}
	
	return presult;
}

static P2DImage  read_4bit_pixels(CFile& image, unsigned int width, unsigned int height)
{
	C2DUBImage *result = new C2DUBImage(C2DBounds(width, height));
	P2DImage presult(result); 
	cvdebug() << "read_4bit_pixels\n";

	for (int y = height-1; y >= 0; --y) {
		int row_count = 0;
		C2DBitImage::iterator p = result->begin() + width * y;
		for (size_t x = 0; x < width; x += 2, ++row_count) {
			int inbyte = fgetc(image);
			if (inbyte == EOF) 
				throw runtime_error("BPM::Load: incomplete image");
			*p++ = (inbyte >> 4) & 0xF;
			*p++ = inbyte & 0xF;
		}
		// eat the padding bytes
		int remain = (4 - row_count % 4) % 4;
		while (remain--) {
			if (fgetc(image) == EOF) 
				throw runtime_error("BPM::Load: incomplete image");
		}
	}

	return presult;
}

static P2DImage  read_8bit_pixels_uc(CFile& image, unsigned int width, unsigned int height)
{
	C2DUBImage *result = new C2DUBImage(C2DBounds(width, height));
	P2DImage presult(result); 
	int row_count =  (4 - width % 4) % 4;
	cvdebug() << "read_8bit_pixels_nc\n";

	for (int y = height-1; y >= 0; --y) {
		if (fread( &(*result)(0,y), width, 1, image) != 1){
			throw runtime_error("BPM::Load: incomplete image");
		}

		int remain = row_count;
		while (remain--) {
			if (fgetc(image) == EOF) 
				throw runtime_error("BPM::Load: incomplete image");
		}
	}
	cvdebug() << "read_8bit_pixels_nc done\n";
	return presult;
}

static P2DImage  read_8bit_pixels_c(CFile& image, unsigned int width, unsigned int height)
{
        TRACE("read_8bit_pixels_c");
        C2DUBImage *result = new C2DUBImage(C2DBounds(width, height));
	P2DImage presult(result); 

	int x = 0;
	int y = height-1;
	while ( y >= 0 )  {


		int n = fgetc(image);
		int c = fgetc(image);
		if (n == EOF || c == EOF) 
			throw runtime_error("BMP::Load: incomplete image");


		if (n == 0) {
			switch (c) {

			case 0: --y;
				x = 0;
				break;

			case 1: y = -1;
				break;

			case 2: {
				const int dx = fgetc(image);
				const int dy = fgetc(image);
				if (dx == EOF || dy == EOF) 
					throw runtime_error("BMP::Load: incomplete image");
				x += dx; 
				y -= dy; 
			} break;

			default: {
				bool odd = c & 1;
				while ( c-- ) {
					const int cc = fgetc(image);
					if (cc == EOF) 
						throw runtime_error("BMP::Load: incomplete image");
					(*result)(x++,y) = cc; 
				}
				if (odd) {
					if (fgetc(image) == EOF) 
						throw runtime_error("BMP::Load: incomplete image");
				}
			}
			}
		}else{
			while ( n-- )
				(*result)(x++,y) = c;
		}
	}

	return presult;
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
                throw create_exception<runtime_error>("CBMP2DImageIO::load: unable to read info header from '", filename, "'");
        }

	cvdebug() << "read info header";

#ifdef WORDS_BIGENDIAN
	endian_adapt_info_header(info_header);
#endif
	switch (info_header.size) {
	case 40:cvdebug() << "BMP: Windows NT, 3.1x style bitmap\n"; 
		break;
	case 108: cvdebug() << "BMP: Windows NT 4.0, 95 style bitmap\n"; 
		break;
	case 124: cvdebug() << "BMP: Windows NT 5.0, 98 style bitmap\n";
	default: 
		throw create_exception<runtime_error>("CBMP2DImageIO::load: incompatible header size=", info_header.size);
	}

	cvdebug() << "validated info header size\n";

	if ((info_header.width < 1) || (info_header.height < 1))
		throw create_exception<runtime_error>("CBMP2DImageIO::load: Image has unsupported dimensions", 
						      " width=", info_header.width, ", height=", 
						      info_header.height);
	
  
	// this is actually a non-sense test but it should silence the Coverty warning about 
	// the tainted variables. 
	size_t h = info_header.height; 
	size_t w = info_header.width; 
	if (h > numeric_limits<unsigned>::max() || w > numeric_limits<unsigned>::max()) 
		throw create_exception<runtime_error>("CBMP2DImageIO::load: Image has too big", 
						      " width=", info_header.width, ", height=", 
						      info_header.height);
	if (fseek(f, header.offset, SEEK_SET) != 0) {
		throw create_exception<runtime_error>("CBMP2DImageIO::load: '", filename,
						      "' ", strerror(errno)); 
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
		default: {
			stringstream errmsg;
			errmsg << "CBMP2DImageIO::load: unsupported pixel size: " << info_header.bits;
			throw runtime_error(errmsg.str());
		}
		} // end switch
	}else{
		switch (info_header.bits) {
		case 4: result->push_back(read_4bit_pixels_c(f, info_header.width, info_header.height));
			break;
		case 8: result->push_back(read_8bit_pixels_c(f, info_header.width, info_header.height));
			break;
		default:
			throw create_exception<runtime_error>("CBMP2DImageIO::load: compressed images with ",
							      info_header.bits, " bits per pixel not supported");
		}
	}
	cvdebug() << "CBMP2DImageIO::load done\n";
	return result;
}

struct C2DBMPImageSaver {
	typedef bool result_type;

	C2DBMPImageSaver(CFile& f):
		m_f(f)
	{
	}

	template <typename Data2D>
	result_type operator ()(const Data2D& image) const;
private:
	bool write_header(int bpp, int compression, const C2DBounds& size)const;

	CFile& m_f;
};

template <typename Image2D>
struct TBMPIOPixelTrait
 {
	enum { pixel_size = 8 * sizeof(typename Image2D::value_type)};
	enum { supported = 0 };
};

template <>
struct TBMPIOPixelTrait<C2DBitImage> {
	enum { pixel_size = 1 };
	enum { supported = 1 };
};

template <>
struct TBMPIOPixelTrait<C2DUBImage> {
	enum { pixel_size = 8 };
	enum { supported = 1 };
};

template <typename Image2D, int supported>
struct T2DBMPImageWriter
{
	static bool apply (const Image2D& /*image*/, FILE */*f*/) {
		throw runtime_error("Pixel type not supported by file format");
	}
};

template <typename Image2D>
struct T2DBMPImageWriter<Image2D, 1>  {
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
struct T2DBMPImageWriter<C2DBitImage, 1> {
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

bool C2DBMPImageSaver::write_header(int bpp, int compression, const C2DBounds& size) const
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

	if (fwrite(&header, sizeof(CBMP2DImageIO::BMPHeader), 1, m_f) != 1)
		return false;
	if (fwrite(&info_header, sizeof(CBMP2DImageIO::BMPInfoHeader), 1, m_f) != 1)
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

		if (fwrite(&palette[0], sizeof(int), ncolours , m_f) != ncolours)
			return false;
	}

	return true;
}

template <typename Image2D>
C2DBMPImageSaver::result_type
C2DBMPImageSaver::operator()(const Image2D& image) const
{
	if (!write_header(TBMPIOPixelTrait<Image2D>::pixel_size, 0, image.get_size()))
		return false;

	return T2DBMPImageWriter<Image2D, TBMPIOPixelTrait<Image2D>::supported>::apply(image,m_f);
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

	C2DBMPImageSaver
	saver(f);

	for (C2DImageVector::const_iterator iimg = data.begin(); iimg != data.end(); ++iimg)
		filter(saver, **iimg);

	cvdebug() << "CBMP2DImageIO::save end\n";
	return true;
}

const string  CBMP2DImageIO::do_get_descr()const
{
	return string("BMP 2D-image input/output support. The plug-in supports reading and writing of binary images and "
		      "8-bit gray scale images. read-only support is provided for 4-bit gray scale images. "
		      "The color table is ignored and the pixel values are taken as literal gray scale values.");
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
		return new CBMP2DImageIO;
}


NS_END
