/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string>
#include <cstring>
#include <sstream>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/imageio.hh>

NS_BEGIN(vtk_3dimage_io)

NS_MIA_USE
using namespace std;
using namespace boost;

enum EDataType {dt_bit, dt_ubyte, dt_sbyte, dt_ushort, dt_sshort,
		dt_uint, dt_sint, dt_ulong, dt_slong, dt_float,dt_double, dt_unknown};


enum FillFlags {
	f_none = 0,
	f_size = 1,
	f_spacing = 2,
	f_origin = 4,
	f_type = 8,
	f_nselms = 0x10,
	f_nelms = 0x20,
	f_all = 0x3F
};


struct SHeader {
	C3DBounds size;
	C3DFVector spacing;
	C3DFVector origin;
	EPixelType type;
	size_t nselms;
	string descr;
	size_t nelms;
	int ff;

	SHeader():ff(f_none){}
};

class CVtk3DImageIOPlugin : public C3DImageIOPlugin {
public:
	CVtk3DImageIOPlugin();
private:

	virtual PData do_load(const string&  filename) const;
	virtual bool do_save(const string& fname, const Data& data) const;
	virtual const string do_get_descr() const;
};

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CVtk3DImageIOPlugin();
}


CVtk3DImageIOPlugin::CVtk3DImageIOPlugin():
	C3DImageIOPlugin("vtk")
{

//	add_property(io_plugin_property_canpipe);
	add_supported_type(it_ubyte);
 	add_supported_type(it_sbyte);
	add_supported_type(it_ushort);
	add_supported_type(it_sshort);
	add_supported_type(it_uint);
	add_supported_type(it_sint);
	add_supported_type(it_float);
	add_supported_type(it_double);
	add_suffix(".vtk");
}


typedef bool (*FTranslator)(const char *value, SHeader *h);


template <typename T3D>
static bool translate_3dval(const char *value, T3D& h)
{
	istringstream vals(value);
	vals >> h.x >> h.y >> h.z;
	return vals.good();
}

static bool translate_dims(const char *value, SHeader *h)
{
	h->ff |= f_size;
	return translate_3dval(value, h->size);
}

static bool translate_origin(const char *value, SHeader *h)
{
	h->ff |= f_origin;
	return translate_3dval(value, h->origin);
}

static bool translate_spacing(const char *value, SHeader *h)
{
	h->ff |= f_spacing;
	return translate_3dval(value, h->spacing);
}


static bool translate_point_data(const char *value, SHeader *h)
{
	h->ff |= f_nelms;
	h->nelms = atoi(value);
	return true;
}

static bool translate_LUT(const char *value, SHeader */*h*/)
{
	return !strcmp(value, "default");
}

struct PTLut{
	const char *key;
	const size_t key_length;
	EPixelType pixel_type;
};

static const PTLut pixel_table[] = {
	{ "char", 4, it_sbyte},
	{ "unsigned char", 13, it_ubyte},
	{ "short", 5, it_sshort},
	{ "unsigned short", 14, it_ushort},
	{ "int", 3, it_sint},
	{ "unsigned int", 12, it_uint},
	{ "float", 5, it_float},
	{ "double", 6, it_double},
	{0, 0, it_unknown}
};



static bool translate_scalars(const char *value, SHeader *h)
{
	h->nselms = 1;

	while (*value == ' ')
		++value;

	if ( strncmp( value, "scalars", 7))
		return false;

	value += 7;
	while (*value == ' ')
		++value;

	const PTLut * pt = pixel_table;
	while (pt->key && strncmp(pt->key, value, pt->key_length))
		++pt;

	h->type = pt->pixel_type;
	h->ff |= f_type;

	value += pt->key_length;
	while (*value == ' ')
		++value;

	if (strlen(value))
		h->nselms = atoi(value);

	if (h->nselms != 1)
		throw invalid_argument("VTK images: only support gray scale images");

	h->ff |= f_nselms;
	return true;
}

struct SluEndian {
	const char *cpu;
	int size;
	bool lowendian;
};


struct Slut {
	const char *key;
	const int key_size;
	FTranslator translate;
};


static const Slut translator_table[] = {
	{ "DIMENSIONS", 10, translate_dims},
	{ "ORIGIN", 6, translate_origin},
	{ "SPACING", 7, translate_spacing},
	{ "POINT_DATA", 10, translate_point_data},
	{ "LOOKUP_TABLE", 12, translate_LUT},
	{ "SCALARS", 7, translate_scalars},
	{ 0, 0, 0}
};



bool read_param(const char *buf, SHeader *h)
{
	const Slut *s = translator_table;
	while (s->key && strncmp(s->key, buf, s->key_size)) {
		++s;
	}
	return s->translate  ? s->translate(&buf[s->key_size], h) : false;
}


template <typename T>
P3DImage read_image(const SHeader& header, 	CInputFile& f)
{
	T3DImage<T> *image = new T3DImage<T>(header.size);
	P3DImage result(image);
	if (fread(&(*image)(0,0,0), sizeof(T), image->size(), f) != image->size())
		throw runtime_error("error reading input file");
	image->set_voxel_size(header.spacing);
	return result;
}

#if 0
static void swap(char& a, char& b)
{
	char c = a;
	a = b;
	b = c;
}
#endif

static bool my_endianess()
{
	typedef union {
		char c[2];
		short s;
	} helpme;

	helpme test;

	test.s = 0x0001;
	return test.c[0];
}

template <class Image>
void handle_endian(Image& image, bool lowendian)
{
	if (lowendian == my_endianess())
		return;

	cvdebug() << "fix endianess\n";

	typename Image::iterator b = image.begin();
	typename Image::iterator e = image.end();

	typedef union  {
		char s[sizeof(typename Image::value_type)];
		typename Image::value_type v;
	} shuffle;



	switch (sizeof(typename Image::value_type)) {
	case 16:
		while (b != e) {
			shuffle s;
			s.v = *b;
			swap(s.s[0], s.s[1]);
			*b = s.v;
		}
		break;
	case 32:
		while (b != e) {
			shuffle s;
			s.v = *b;
			swap(s.s[0], s.s[3]);
			swap(s.s[1], s.s[2]);
			*b = s.v;
			++b;
		}
		break;
	case 64:
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
		// default:
	}
}

/*
#vtk DataFile Version 3.0
VTK File Generated by Insight Segmentation and Registration Toolkit (ITK)
BINARY
DATASET STRUCTURED_POINTS
DIMENSIONS 64 64 44
SPACING 8.0000000000000000e+000 8.0000000000000000e+000
3.9917199611663818e+000
ORIGIN 0.0000000000e+000 0.00000000000e+000 0.0000000000000e+000
POINT_DATA 180224
SCALARS scalars float 3
LOOKUP_TABLE default
*/


template <typename T>
struct data_type_name {
	enum {value = dt_unknown};
	static const char *get() {
		throw invalid_argument("unknown data type");
	}
};

template <>
struct data_type_name<bool> {
	enum {value = dt_bit};
	static const char *get() {
		return "bit";
	}
};

template <>
struct data_type_name<signed char> {
	enum {value = dt_sbyte};
	static const char *get() {
		return "char";
	}

};

template <>
struct data_type_name<unsigned char> {
	enum {value = dt_ubyte};
	static const char *get() {
		return "unsigned char";
	}

};

template <>
struct data_type_name<signed short> {
	enum {value = dt_sshort};
	static const char *get() {
		return "short";
	}

};

template <>
struct data_type_name<unsigned short> {
	enum {value = dt_ushort};
	static const char *get() {
		return "unsigned short";
	}

};

template <>
struct data_type_name<signed int> {
	enum {value = dt_sint};
	static const char *get() {
		return "int";
	}

};

template <>
struct data_type_name<unsigned int> {
	enum {value = dt_uint};
	static const char *get() {
		return "unsigned int";
	}

};


template <>
struct data_type_name<float> {
	enum {value = dt_float};
	static const char *get() {
		return "float";
	}

};

template <>
struct data_type_name<double> {
	enum {value = dt_double};
	static const char *get() {
		return "double";
	}
};

CVtk3DImageIOPlugin::PData CVtk3DImageIOPlugin::do_load(const string&  filename)const
{
	CInputFile f(filename);
	if (!f){
		cvdebug() << filename.c_str() << ":" << strerror(errno) << "\n";
		return PData();
	}
	char buf[1024];

	if (!fgets(buf, 1024, f))
		return PData();

	if (strncmp("#vtk DataFile Version", buf, 21))
		return PData();

	if (!fgets(buf, 1024, f))
		throw runtime_error(filename + ":VTK data set corrupt");

	cvmsg() << "load VTK data set: '" << buf << "'\n";

	if (!fgets(buf, 1024, f))
		throw runtime_error(filename + ":VTK data set corrupt");

	if (strncmp("BINARY", buf, 6)) {
		cverr() << "VTK only BINARY files are supported\n";
		return PData();
	}

	if (!fgets(buf, 1024, f))
		throw runtime_error(filename + ":VTK data set corrupt");

	if (strncmp("DATASET STRUCTURED_POINT", buf, 24)) {
		cverr() << "unsupported vtk data set '" << buf << "'";
		return PData();
	}

	SHeader header;

	while (header.ff != f_all && fgets(buf, 1024, f)) {
		if (!read_param(buf, &header))  {
			cverr() << "unsupported vtk data set or info '" << buf << "'";
			return PData();
		}
	}

	PData result(new C3DImageVector());

	switch (header.type) {
	case it_ubyte:  result->push_back(read_image<unsigned char>(header, f)); break;
	case it_sbyte:  result->push_back(read_image<signed char>(header, f)); break;
	case it_ushort: result->push_back(read_image<unsigned short>(header, f)); break;
	case it_sshort: result->push_back(read_image<signed short>(header, f)); break;
	case it_uint:   result->push_back(read_image<unsigned int>(header, f)); break;
	case it_sint:   result->push_back(read_image<signed int>(header, f)); break;
	case it_float:  result->push_back(read_image<float>(header, f)); break;
	case it_double: result->push_back(read_image<double>(header, f)); break;
	default:
		throw invalid_argument("Data type not supported");
	}


	return result;
}

#ifdef __x86_64
#define FORMAT_SIZE_T "%ld"
#else
#define FORMAT_SIZE_T "%d"
#endif

template <typename T>
struct __dispatch_save {
	static bool apply(const T3DImage<T>& image, COutputFile &f) {
		fprintf(f, "#vtk DataFile Version 3.0\n");
		fprintf(f, "MIA generated VTK file\n");
		fprintf(f, "BINARY\n");
		fprintf(f, "DATASET STRUCTURED_POINT\n");
		fprintf(f, "DIMENSIONS %d %d %d\n",
			image.get_size().x, image.get_size().y, image.get_size().z);
		const C3DFVector voxel = image.get_voxel_size();
		fprintf(f, "SPACING %20.10g %20.10g %20.10g \n", voxel.x, voxel.y, voxel.z);
		fprintf(f, "ORIGIN  0 0 0\n");
		fprintf(f, "POINT_DATA " FORMAT_SIZE_T "\n", image.size());
		fprintf(f, "SCALARS scalars %s 1\n", data_type_name<T>::get());
		return fwrite(&image(0,0,0), sizeof(T), image.size(), f) == image.size();
	}
};

template <>
struct __dispatch_save<bool> {
	static bool apply(const T3DImage<bool>& /*image*/, COutputFile &/*f*/) {
		throw invalid_argument("VTK: image IO not implemented for bit");
	}
};


class CVtkSaver: public TFilter<bool> {
public:
	CVtkSaver(COutputFile& f):
		m_f(f)
	{
	}

	template <class T>
	bool operator ()(const T3DImage<T>& image) const
	{
		return __dispatch_save<T>::apply(image, m_f);
	}

private:
	COutputFile& m_f;
};

bool CVtk3DImageIOPlugin::do_save(string const&  filename, const C3DImageVector& data) const
{
	if (data.size() != 1) {
		cverr() << "CVtk3DImageIO::save: support only 1 image\n";
		return false;
	}

	COutputFile f(filename);

	if (!f) {
		cverr() << filename.c_str() << ":" << strerror(errno) << "\n";
		return false;
	}

	CVtkSaver s(f);

	return filter(s, **data.begin());
}


const string  CVtk3DImageIOPlugin::do_get_descr() const
{
	return string("Kitware Inc Visualization Toolkit image");
}


NS_END
