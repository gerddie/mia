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


CBMPRGB2DImageIO::PData CBMPRGB2DImageIO::do_load(string const& MIA_PARAM_UNUSED(filename))const
{
	assert(0 && "loading of RGB image not yet supported");
	return PData();
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
	for (int i = data.get_size().y - 1; i >= 0 && good; --i) {
		auto p = data.pixel() +  i * dx; 
		for (auto b = buffer.begin(); b != buffer.end(); ++b) {
			b->r = *p++; 
			b->g = *p++; 
			b->b = *p++; 
		}

		good = (dx == fwrite(&buffer[0], 1, dx, f));
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
