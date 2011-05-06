/*  -*- mia-c++ -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * 2007 Gert Wollny
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

// $Id: inria_3dimage_io.cc 896 2006-03-27 10:01:12Z write1 $

/*! \brief plugin function to handle VTK-format IO

\file vtk.cc
\author Gert Wollny <wollny@cbs.mpg.de>

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
// MIA specific


#include <string>
#include <cstring>
#include <sstream>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/3dvfio.hh>

NS_BEGIN(vtkvf_3dimage_io)

NS_MIA_USE
using namespace std;
using namespace boost;

enum EDataType {dt_float, dt_unknown};


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

class CVtk3DVFIOPlugin : public C3DVFIOPlugin {
public:
	CVtk3DVFIOPlugin();
private:

	virtual PData do_load(const string&  filename) const;
	virtual bool do_save(const string& fname, const Data& data) const;
	virtual void do_add_suffixes(multimap<string, string>& map) const;
	virtual const string do_get_descr() const;
	void load_line(char *buf, CInputFile& f, const string& filename) const;
};

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CVtk3DVFIOPlugin();
}


CVtk3DVFIOPlugin::CVtk3DVFIOPlugin():
	C3DVFIOPlugin("vtk")
{
	add_supported_type(it_float);
}


typedef bool (*FTranslator)(const char *value, SHeader *h);


template <typename T3D>
static bool translate_3dval(const char *value, T3D& h)
{
	istringstream vals(value);
	vals >> h.x >> h.y >> h.z;

	cvmsg() << value << ":" << h << "\n";
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
	{ "float", 5, it_float},
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

	if (h->nselms != 3)
		throw invalid_argument("VTK Vector fields: only support 3D vectors");

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


CVtk3DVFIOPlugin::PData read_vf(const SHeader& header, 	CInputFile& f)
{
	CVtk3DVFIOPlugin::PData result(new C3DIOVectorfield(header.size));

	if (fread(&(*result)(0,0,0), 3 * sizeof(float), result->size(), f) != result->size())
		throw runtime_error("error reading input file");
	//result->set_voxel_size(header.spacing);
	return result;
}

static void swap(char& a, char& b)
{
	char c = a;
	a = b;
	b = c;
}

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
struct data_type_name<float> {
	enum {value = dt_float};
	static const char *get() {
		return "float";
	}

};

void CVtk3DVFIOPlugin::load_line(char *buf, CInputFile& f, const string& filename)const
{
	if (fgets(buf, 1024, f) == NULL) {
		stringstream msg;
		msg << filename << ": bogus file";
		throw runtime_error(msg.str());
	}
}

CVtk3DVFIOPlugin::PData CVtk3DVFIOPlugin::do_load(const string&  filename)const
{

	CInputFile f(filename);
	if (!f){
		cvdebug() << filename.c_str() << ":" << strerror(errno) << "\n";
		return PData();
	}
	char buf[1024];

	load_line(buf, f, filename);

	if (strncmp("#vtk DataFile Version", buf, 21))
		return PData();

	load_line(buf, f, filename);
	cvdebug() << "load VTK data set: '" << buf << "'\n";

	load_line(buf, f, filename);
	if (strncmp("BINARY", buf, 6)) {
		cverr() << "VTK only BINARY flies are supported\n";
		return PData();
	}

	load_line(buf, f, filename);
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

	switch (header.type) {
	case it_float:  return read_vf(header, f); break;
	default:
		throw invalid_argument("Data type not supported");
	}

	return PData();
}

bool CVtk3DVFIOPlugin::do_save(string const&  filename, const C3DIOVectorfield& data) const
{

	COutputFile f(filename);

	if (!f) {
		cverr() << filename.c_str() << ":" << strerror(errno) << "\n";
		return false;
	}

	fprintf(f, "#vtk DataFile Version 3.0\n");
	fprintf(f, "MIA generated VTK vector field\n");
	fprintf(f, "BINARY\n");
	fprintf(f, "DATASET STRUCTURED_POINT\n");
	fprintf(f, "DIMENSIONS %d %d %d\n", data.get_size().x, data.get_size().y, data.get_size().z);
	//const C3DFVector voxel = data.get_voxel_size();
	//fprintf(f, "SPACING %20.10g %20.10g %20.10g \n", voxel.x, voxel.y, voxel.z);
	fprintf(f, "ORIGIN  0 0 0\n");
	fprintf(f, "POINT_DATA %d\n", (unsigned int) data.size());
	fprintf(f, "SCALARS scalars float 3\n");
	return fwrite(&data(0,0,0), 3 * sizeof(float), data.size(), f) == data.size();

}

void CVtk3DVFIOPlugin::do_add_suffixes(multimap<string, string>& map) const
{
	map.insert(pair<string,string>(".vtk", get_name()));
}

const string  CVtk3DVFIOPlugin::do_get_descr() const
{
	return string("VTK vector field input/output support");
}


NS_END
