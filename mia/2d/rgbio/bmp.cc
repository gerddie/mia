/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
#include <mia/2d/rgbimageio.hh>

NS_BEGIN(RGBIMAGEIO_2D_BMP)

NS_MIA_USE
using namespace std;
using namespace boost;


class CBMPRGB2DImageIO: public C2DRGBImageIOPlugin {
public:
	CBMPRGB2DImageIO();

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
	bool do_save(const string& fname, const CRGB2DImage& data) const;
	const string do_get_descr() const;
	bool write_header(COutputFile& f, const C2DBounds& size) const; 
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


CBMPRGB2DImageIO::CBMPRGB2DImageIO():
	C2DRGBImageIOPlugin("bmp")
{
	add_supported_type(it_ushort);
	add_supported_type(it_ubyte);
	add_supported_type(it_bit);
	add_suffix(".bmp");
	add_suffix(".BMP");
}


#ifdef WORDS_BIGENDIAN
void endian_adapt_header(CBMPRGB2DImageIO::BMPHeader& header)
{
	ENDIANADAPT(header.size);
	ENDIANADAPT(header.offset);
}

void endian_adapt_info_header(CBMPRGB2DImageIO::BMPInfoHeader& header)
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

static PRGB2DImage read_8bit_pixels_uc(CFile& image, unsigned int width, unsigned int height)
{
	PRGB2DImage result(new CRGB2DImage(C2DBounds(width, height))); 

	unsigned load_width = ((3 * width + 3) /4) * 4; 
	vector< unsigned char> buffer(load_width); 
	unsigned char *pixel =  result->pixel();

	cvdebug() << "Loading " << load_width << "\n"; 
	
	for (int y = height-1; y >= 0; --y) {
		if (fread( &buffer[0], 1, load_width, image) != load_width){
			throw runtime_error("BPM::Load: incomplete image");
		}

		for (unsigned x = 0; x < width; ++x) {

			unsigned char *p = &pixel[3*(x + y * width)];
			p[0] = buffer[3*x+2];
			p[1] = buffer[3*x+1];
			p[2] = buffer[3*x+0]; 
		}
	}
	
	return result;
}


CBMPRGB2DImageIO::PData CBMPRGB2DImageIO::do_load(string const& MIA_PARAM_UNUSED(filename))const
{
	int read = 0;
	CInputFile f(filename);
	if (!f)
		return PData();

	BMPHeader header;
	BMPInfoHeader info_header;

	cvdebug() << "CBMPRGB2DImageIO::load\n";

	read = fread(&header, sizeof(BMPHeader), 1, f);
	if (header.magic[0] != 'B' || header.magic[1] != 'M' || read != 1)
		return PData();

#ifdef WORDS_BIGENDIAN
	endian_adapt_header(header);
#endif

	cvdebug() << "read header\n";

	read = fread(&info_header, sizeof(BMPInfoHeader), 1, f);
	if (read != 1) {
                throw create_exception<runtime_error>("CBMPRGB2DImageIO::load: unable to read info header from '", filename, "'");
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
		throw create_exception<runtime_error>("CBMPRGB2DImageIO::load: Image has unsupported dimensions", 
						      " width=", info_header.width, ", height=", 
						      info_header.height);
	
  
	// this is actually a non-sense test but it should silence the Coverty warning about 
	// the tainted variables. 
	size_t h = info_header.height; 
	size_t w = info_header.width; 
	if (h > numeric_limits<unsigned>::max() || w > numeric_limits<unsigned>::max()) 
		throw create_exception<runtime_error>("CBMPRGB2DImageIO::load: '", filename,
						      "' Image has too big - width=",
						      info_header.width, ", height=", 
						      info_header.height);
	
	if (fseek(f, header.offset, SEEK_SET) != 0) {
		throw create_exception<runtime_error>("CBMP2DImageIO::load: '", filename,
						      "' ", strerror(errno)); 
	}


	if (!info_header.compression) {

		switch (info_header.bits) {
		case 24: return read_8bit_pixels_uc(f, info_header.width, info_header.height);
		default: {
			stringstream errmsg;
			errmsg << "CBMP2DImageIO::load: unsupported pixel size: " << info_header.bits;
			throw runtime_error(errmsg.str());
		}
		} // end switch
	}else{
		throw create_exception<runtime_error>("CBMPRGB2DImageIO::load: compressed RGB images with ",
						      info_header.bits, " bits per pixel not supported");
	}
}

bool CBMPRGB2DImageIO::write_header(COutputFile& f, const C2DBounds& size) const
{
	cvdebug() << "CImageSaver::write_header("<< size<< ")\n";
	CBMPRGB2DImageIO::BMPHeader header;

	header.magic[0] = 'B';
	header.magic[1] = 'M';
	header.reserved1 = header.reserved2 = 0;
	header.offset = sizeof(CBMPRGB2DImageIO::BMPHeader) + sizeof(CBMPRGB2DImageIO::BMPInfoHeader);
	header.size = header.offset + size.y * ( ((size.x + 3) & 0xFFFFFFE0) / 8);

	CBMPRGB2DImageIO::BMPInfoHeader info_header;

	info_header.size = sizeof(CBMPRGB2DImageIO::BMPInfoHeader);
	info_header.width = size.x;
	info_header.height = size.y;
	info_header.planes = 1;
	info_header.bits = 24;
	info_header.compression = 0;
	info_header.imagesize = 0;
	info_header.xresolution = 1;
	info_header.yresolution = 1;
	info_header.ncolours = 0;
	info_header.importantcolours = 0;

#ifdef WORDS_BIGENDIAN
	endian_adapt_header(header);
	endian_adapt_info_header(info_header);
#endif

	if (fwrite(&header, sizeof(CBMPRGB2DImageIO::BMPHeader), 1, f) != 1)
		return false;
	if (fwrite(&info_header, sizeof(CBMPRGB2DImageIO::BMPInfoHeader), 1, f) != 1)
		return false;
	return true;
}

bool CBMPRGB2DImageIO::do_save(string const& filename, const CRGB2DImage& data) const
{
	cvdebug() << "CBMPRGB2DImageIO::save begin\n";

	COutputFile f(filename);
	if (!f) {
		cverr() << "CBMPRGB2DImageIO::save:unable to open output file:" << filename << "\n";
		return false;
	}

	bool good = true; 
	const size_t dx = 3 * data.get_size().x; 
	write_header(f, data.get_size()); 
	struct BGR {
		unsigned char b,g,r; 
	}; 
	vector<BGR> buffer(data.get_size().x);
	const unsigned  fill = (data.get_size().x * 3 + 3)/4 * 4 - 3 * data.get_size().x; 
	
	for (int i = data.get_size().y - 1; i >= 0 && good; --i) {
		auto p = data.pixel() +  i * dx; 
		for (auto b = buffer.begin(); b != buffer.end(); ++b) {
			b->r = *p++; 
			b->g = *p++; 
			b->b = *p++; 
		}

		good &= (dx == fwrite(&buffer[0], 1, dx, f));
		for (unsigned i = 0; i < fill; ++i)
			good &= (fputc(0, f) != EOF);
		
	}
	
	cvdebug() << "CBMPRGB2DImageIO::save end\n";
	return good;
}

const string  CBMPRGB2DImageIO::do_get_descr()const
{
	return string("BMP RGB 2D-image input/output support");
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
		return new CBMPRGB2DImageIO;
}


NS_END
