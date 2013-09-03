/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <errno.h>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <istream>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/imageio.hh>


NS_BEGIN(ns_3d_io_vff)
NS_MIA_USE

using namespace std;
using namespace boost;

struct SHeader {
	bool m_valid;
	C3DBounds m_size;
	EPixelType m_pixel_type;
	string m_subject;
	string m_title;
	int m_rank;
	float m_min;
	float m_max;
	float m_elementsize;
	C3DFVector m_spacing;
};

typedef bool (*FDecode)(istream& is, SHeader& header);

static bool decode_valid(istream& is, SHeader& header);
static bool decode_size(istream& is, SHeader& header);
static bool decode_pixel_type(istream& is, SHeader& header);
static bool decode_title(istream& is, SHeader& header);
static bool decode_subject(istream& is, SHeader& header);
static bool decode_min(istream& is, SHeader& header);
static bool decode_rank(istream& is, SHeader& header);
static bool decode_max(istream& is, SHeader& header);
static bool decode_spacing(istream& is, SHeader& header);
static bool decode_elementsize(istream& is, SHeader& header);

struct SDecodeMap {
	const char *key;
	FDecode callback;
};

SDecodeMap decode_map[] = {
	{"ncaa", decode_valid},
	{"subject", decode_subject},
	{"title", decode_title},
	{"rank", decode_rank},
	{"type",  NULL },
	{"size", decode_size},
	{"origin", NULL},
	{"y_bin", NULL},
	{"z_bin", NULL},
	{"bands", NULL},
	{"bits", decode_pixel_type},
	{"format",NULL},
	{"date", NULL},
	{"center_of_rotation",NULL},
	{"central_slice",NULL},
	{"rfan_y", NULL},
	{"rfan_z",NULL},
	{"angle_increment", NULL},
	{"reverse_order", NULL},
	{"min", decode_min},
	{"max", decode_max},
	{"spacing",  decode_spacing},
	{"elementsize",decode_elementsize},
	{"water", NULL},
	{"air", NULL},
	{NULL, NULL}
};


static bool decode_valid(istream& /*is*/, SHeader& /*header*/)
{
	return true;
}

static bool decode_size(istream& is, SHeader& header)
{
	is >> header.m_size.x >> header.m_size.y >> header.m_size.z;
	cvdebug() << "Got size " << header.m_size << '\n';
	return !(is.rdstate() & (ifstream::failbit | ifstream::badbit));
}

static bool decode_pixel_type(istream& is, SHeader& header)
{
	int pixel_size = 0;
	is >> pixel_size;
	if (pixel_size == 16)
		header.m_pixel_type = it_sshort;
	else if (pixel_size == 8)
		header.m_pixel_type = it_ubyte;
	else
		return false;
	cvdebug() << "pixel size = " << pixel_size << " type= " << header.m_pixel_type << "\n";
	return true;
}

#define DECODE(NAME) static bool decode_##NAME(istream& is, SHeader& header) \
	{\
		is >> header.m_##NAME; \
		return !(is.rdstate() & (ifstream::failbit | ifstream::badbit));  \
	}

DECODE(title);
DECODE(subject);
DECODE(min);
DECODE(max);
DECODE(rank);
DECODE(elementsize);

#undef DECODE


static bool decode_spacing(istream& is, SHeader& header)
{
	is >> header.m_spacing.x >> header.m_spacing.y >> header.m_spacing.z;
	return !(is.rdstate() & (ifstream::failbit | ifstream::badbit));
}

class CVFF3DImageIO: public C3DImageIOPlugin {
public:
	CVFF3DImageIO();
private:
	virtual PData do_load(string const&  filename)const;
	virtual bool do_save(string const&  filename, const C3DImageVector& data) const;
	virtual const string do_get_descr() const;
	bool store_info(const char *key, const char *value, SHeader& h) const;
};

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CVFF3DImageIO();
}


CVFF3DImageIO::CVFF3DImageIO():
	C3DImageIOPlugin("vff")
{
//	add_property(io_plugin_property_canpipe);
	add_supported_type(it_sshort);
	add_supported_type(it_ubyte);
	add_suffix(".vff");
}


bool CVFF3DImageIO::store_info(const char *key, const char *value, SHeader& h) const
{
	const SDecodeMap *dm = decode_map;
	while (dm->key) {
		if (!strcmp(dm->key, key)) {
			if( dm->callback ) {
				istringstream s(value);
				return dm->callback(s, h);
			}
			else
				cvdebug() << "Ignoring key '" << key << "' with value '" << value << "\n";
		}
		++dm;
	}
	cvwarn() << "Unknown key '" << key << "' with value '" << value << "\n";
	return true;
}

template <typename T>
static bool load_image(T3DImage<T>& image, C3DFVector voxel, CInputFile& f)
{
	image.set_voxel_size(voxel);

	long load_size =  image.size();

	long read = fread(&image(0,0,0), sizeof(T), load_size, f);
	return read == load_size;
}

static void swap(char& a, char& b)
{
	char c = a;
	a = b;
	b = c;
}


static bool low_endian()
{
	typedef union {
		char c[2];
		short s;
	} helpme;

	helpme test;

	test.s = 0x0001;
	return test.c[0];
}

template <class Iterator>
void handle_endian(Iterator b, Iterator e)
{

	typedef typename iterator_traits<Iterator>::value_type Pixel;
	cvdebug() << "fix endianness\n";


	typedef union  {
		char s[sizeof(Pixel)];
		Pixel v;
	} shuffle;



	switch (sizeof(Pixel)) {
	case 2:
		cvdebug() << "Endian correct 16 bit data\n";
		while (b != e) {
			shuffle s;
			s.v = *b;
			swap(s.s[0], s.s[1]);
			*b = s.v;
			++b;
		}
		break;
	case 4:
		cvdebug() << "Endian correct 32 bit data\n";
		while (b != e) {
			shuffle s;
			s.v = *b;
			swap(s.s[0], s.s[3]);
			swap(s.s[1], s.s[2]);
			*b = s.v;
			++b;
		}
		break;
		cvdebug() << "Endian correct 64 bit data\n";
	case 8:
		while (b != e) {

			shuffle s;
			s.v = *b;

			swap(s.s[0],s.s[7]);
			swap(s.s[1],s.s[6]);
			swap(s.s[2],s.s[5]);
			swap(s.s[3],s.s[4]);

			*b = s.v;

			++b;
		}
		break;
	default:
		cvdebug() << "8 bit data, no correction needed\n";
	}
}

CVFF3DImageIO::PData CVFF3DImageIO::do_load(string const&  filename)const
{
	cvdebug() << "Try vff image type\n";

	CInputFile f(filename);
	if (!f){
		perror(filename.c_str());
		return PData();
	}

	char buffer[256];

	if (fgets(buffer, 256, f) == NULL) {
		perror(filename.c_str());
		return PData();
	}

	if (strncmp(buffer, "ncaa", 4))
		return PData();

	SHeader header;
	header.m_elementsize = 1.0;

	while (fgets(buffer, 256, f)) {
		cvdebug() << "Read: " << buffer << '\n';

		char *split = strchr(buffer, '=');
		if (split) {
			*split = 0;
			++split;
			char *end = strchr(split, ';');
			if (!end)
				throw invalid_argument(string("vff-load:Bogous header line in " + filename));

			*end = 0;

			if (!store_info(buffer, split, header)) {
				throw invalid_argument(string("vff-load:Read component '") + string(split)  + string("' failed"));
			}
		}else{
			if (buffer[0] != 0xC)
				throw invalid_argument(string("vff-load:Bogus header line:") + buffer);
			else
				break;
		}
	}

	if (header.m_rank != 3)
		throw invalid_argument(string("vff-load:Unsupported image type"));

	header.m_spacing *= header.m_elementsize;

	PData result(new C3DImageVector());

	switch (header.m_pixel_type) {
	case it_ubyte:{
		C3DUBImage *image = new C3DUBImage(header.m_size);
		std::shared_ptr<C3DImage > pimage(image);
		if (load_image(*image, header.m_spacing, f))
			result->push_back(pimage);
	}break;
	case it_sshort:{
		C3DSSImage *image = new C3DSSImage(header.m_size);
		std::shared_ptr<C3DImage > pimage(image);
		if (load_image(*image, header.m_spacing, f)) {
			if (low_endian())
				handle_endian(image->begin(), image->end());
			result->push_back(pimage);
		}
	}break;
	default:
		assert(!"input format not implemented");
	}

	if (result->size() > 0)
		return result;

	return PData();
}

template <typename T>
struct save_dispatch {
	static bool apply(const T3DImage<T>& image, COutputFile& f) {

		C3DFVector voxel = image.get_voxel_size();

		stringstream os;
		os << "ncaa;" << "\n";

		os << "size="
		   << image.get_size().x << " "
		   << image.get_size().y << " "
		   << image.get_size().z << ";\n";

		os << "rank=3;\n";
		os << "spacing=" << voxel.x << " "
		   << voxel.y << " "
		   << voxel.z << ";\n";

		os << "bits=" << 8 * sizeof(T) << ";\n";

		// end of header
		os << "\xC\n";

		if (fwrite(os.str().c_str(), os.str().size(), 1 , f) != 1) {
			perror("CVFFSaver");
			return false;
		}

		// endianness handling
		if (low_endian() && sizeof(T) > 1 ) {
			vector<T> v(image.size());
			copy(image.begin(), image.end(), v.begin());
			handle_endian(v.begin(), v.end());
			const T& vp = v[0];
			if (fwrite(&vp, sizeof(T), v.size(), f) != v.size()) {
				perror("CVFFSaver");
				return false;
			}
		}else if (fwrite(&image(0,0,0),sizeof(T), image.size(), f) != image.size()) {
			perror("CVFFSaver");
			return false;
		}
		return true;
	}
};

template <>
struct save_dispatch<bool> {
	static bool apply(const T3DImage<bool>& /*image*/, COutputFile& /*f*/) {
		throw invalid_argument("VFF doesn't support bit images");
	}
};

class CVFFSaver: public TFilter<bool> {
public:
	CVFFSaver(COutputFile& f):
		m_f(f)
	{
	}

	template <class T>
	bool operator ()(const T3DImage<T>& image) const
	{
		return save_dispatch<T>::apply(image, m_f);
	}

private:
	COutputFile& m_f;
};

bool CVFF3DImageIO::do_save(string const&  filename, const C3DImageVector& data)const
{
	if (data.size() != 1) {
		cverr() << "CVFF3DImageIO::save: support only 1 image\n";
		return false;
	}

	COutputFile f(filename);

	if (!f) {
		cverr() << filename << ":" << strerror(errno) << "\n";
		return false;
	}

	CVFFSaver s(f);

	return filter(s, **data.begin());
}

const string  CVFF3DImageIO::do_get_descr()const
{
	return string("VFF Sun raster format");
}

NS_END
