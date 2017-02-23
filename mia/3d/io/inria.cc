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

#include <cassert>
#include <errno.h>
#include <cstring>
#include <string>
#include <sstream>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/imageio.hh>

NS_BEGIN(ns_3d_io_inria)

NS_MIA_USE
using namespace std;
using namespace boost;

enum EDataType {dt_float, dt_unsigned, dt_signed};

struct SHeader {
	C3DBounds size;
	C3DFVector voxel;
	int dim;
	EDataType type;
	int bpp;
	string scale;
	bool lowendian;
	SHeader();
};
SHeader::SHeader():
	size(0,0,0),
	voxel(1.0,1.0,1.0),
	dim(3),
	type(dt_unsigned),
	bpp(8),
	scale("1**0"),
	lowendian(true)
{
	TRACE("SHeader::SHeader");
}


class CInria3DImageIOPlugin : public C3DImageIOPlugin {
public:
	CInria3DImageIOPlugin();
private:

	virtual PData do_load(const string&  filename) const;
	virtual bool do_save(const string& fname, const Data& data) const;
	virtual const string do_get_descr() const;
	const std::string do_get_preferred_suffix() const; 
};

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CInria3DImageIOPlugin();
}


CInria3DImageIOPlugin::CInria3DImageIOPlugin():
	C3DImageIOPlugin("inria")
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
	add_suffix(".inr");
	add_suffix(".INR");
}


typedef bool (*FTranslator)(const char *value, SHeader *h);


static bool translate_xdim(const char *value, SHeader *h)
{
	h->size.x = atoi(value);
	return h->size.x > 0;
}

static bool translate_ydim(const char *value, SHeader *h)
{
	h->size.y = atoi(value);
	return h->size.y > 0;
}

static bool translate_zdim(const char *value, SHeader *h)
{
	h->size.z = atoi(value);
	return h->size.z > 0;
}

static bool translate_vdim(const char *value, SHeader *h)
{
	h->dim = atoi(value);
	return h->dim > 0;
}


static bool translate_zvox(const char *value, SHeader *h)
{
	h->voxel.z = atof(value);
	return h->voxel.z > 0;
}

static bool translate_xvox(const char *value, SHeader *h)
{
	h->voxel.x = atof(value);
	return h->voxel.x > 0;
}

static bool translate_yvox(const char *value, SHeader *h)
{
	h->voxel.y = atof(value);
	return h->voxel.y > 0;
}

static bool translate_type(const char *value, SHeader *h)
{
	if (!strncmp(value, "unsigned fixed",14)) {
		h->type = dt_unsigned;
		return true;
	}
	if (!strncmp(value, "signed fixed",12)) {
		h->type = dt_signed;
		return true;
	}
	if (!strncmp(value, "float",5)) {
		h->type = dt_float;
		return true;
	}
	return false;
}

struct SluEndian {
	const char *cpu;
	int size;
	bool lowendian;
};

static bool translate_bpp(const char *value, SHeader *h)
{
	h->bpp = atoi(value);
	return h->bpp > 0;
}

const SluEndian lu_endian[] ={
	{ "decm", 4, true },
	{ "alpha",5, true },
	{ "pc", 2, true },
	{ "sun", 3, false },
	{ "sgi", 3,false },
	{ 0, 0, false}
};



static bool translate_endian(const char *value, SHeader *h)
{
	const SluEndian *lu = lu_endian;
	while (lu->cpu) {
		if (!strncmp(lu->cpu, value, lu->size)) {
			h->lowendian = lu->lowendian;
			return true;
		}
		++lu;
	}
	return false;
}

static bool translate_scale(const char *value, SHeader *h)
{
	h->scale = string(value);
	return true;
}

struct Slut {
	const char *key;
	FTranslator translate;
};

const Slut lut[] = {
	{ "XDIM",  translate_xdim},
	{ "YDIM",  translate_ydim},
	{ "ZDIM",  translate_zdim},
	{ "VX",    translate_xvox},
	{ "VY",    translate_yvox},
	{ "VZ",    translate_zvox},
	{ "VDIM",  translate_vdim},
	{ "TYPE",  translate_type},
	{ "PIXSIZE",translate_bpp},
	{ "CPU",   translate_endian},
	{ "SCALE", translate_scale},
	{ 0,0}
};

bool store_info(const char *key, const char *value, SHeader *h)
{
	assert(value);
	assert(h);
	const Slut *lu = lut;
	while (lu->key) {
		if (!strcmp(lu->key, key))
			return lu->translate(value, h);
		++lu;
	}
	return false;
}

template <class T>
bool load_image(T3DImage<T>& image, const C3DFVector& voxel, FILE *f)
{
	size_t load_size =  image.size();
	image.set_voxel_size(voxel);
	return load_size == fread(&image(0,0,0), sizeof(T), load_size, f);
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


	typename Image::iterator b = image.begin();
	typename Image::iterator e = image.end();

	typedef union  {
		char s[sizeof(typename Image::value_type)];
		typename Image::value_type v;
	} shuffle;



	switch (sizeof(typename Image::value_type)) {
	case 2:
		while (b != e) {
			shuffle s;
			s.v = *b;
			swap(s.s[0], s.s[1]);
			*b = s.v;
		}
		break;
	case 4:
		while (b != e) {
			shuffle s;
			s.v = *b;
			swap(s.s[0], s.s[3]);
			swap(s.s[1], s.s[2]);
			*b = s.v;
			++b;
		}
		break;
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
		// default:
	}
}

CInria3DImageIOPlugin::PData CInria3DImageIOPlugin::do_load(const string&  filename)const
{
	SHeader image_info;
	CInputFile f(filename);
	if (!f){
		cvdebug() << filename.c_str() << ":" << strerror(errno) << "\n";
		return PData();
	}

	char header[256];

	if (fgets(header, 256, f) == NULL) {
		perror(filename.c_str());
		return PData();
	}

	if (strncmp(header, "#INRIMAGE-4#{", 13)) {
		return PData();
	}



	while (fgets(header, 256, f)) {
		char *split = strchr(header, '=');
		if (split) {
			*split = 0;
			++split;
			if (!store_info(header, split, &image_info)) {
				cverr() << "unknown component in header:" << header << ":" << split << "\n";
				return PData();
			}
		}else{
			if ( !strncmp(header, "##}", 3 ) )
				break;
		}
	}

	if (image_info.dim != 1) {
		cvdebug() << "Not a scalar image, try loading as vector field" << "\n";
		return PData();
	}

	PData result(new C3DImageVector());

	switch (image_info.type) {
	case dt_signed:
		cvdebug() << "INRIA: load signed \n";
		switch (image_info.bpp) {
		case  8: {
			cvdebug() << " 8 bit \n";
			C3DSBImage *image = new C3DSBImage(image_info.size);
			std::shared_ptr<C3DImage > pimage(image);
			if (load_image(*image, image_info.voxel, f))
				result->push_back(pimage);
		}break;
		case 16:{
			cvdebug() << " 16 bit \n";
			C3DSSImage *image = new C3DSSImage(image_info.size);
			std::shared_ptr<C3DImage > pimage(image);
			if (load_image(*image, image_info.voxel, f)) {
				handle_endian(*image, image_info.lowendian);
				result->push_back(pimage);
			}
		}break;
		case 32: {
			cvdebug() << " 32 bit \n";
			C3DSIImage *image = new C3DSIImage(image_info.size);
			std::shared_ptr<C3DImage > pimage(image);
			if (load_image(*image, image_info.voxel, f)) {
				handle_endian(*image, image_info.lowendian);
				result->push_back(pimage);
			}
		}break;
		default:
			cverr() << image_info.bpp << " bit signed images not supported" << "\n";
		} break;
	case dt_unsigned:
		switch (image_info.bpp) {
			cvdebug() << "INRIA: load unsigned \n";
		case  8: {
			cvdebug() << " 8 bit \n";
			C3DUBImage *image = new C3DUBImage(image_info.size);
			std::shared_ptr<C3DImage > pimage(image);
			if (load_image(*image, image_info.voxel, f))
				result->push_back(pimage);
		}break;
		case 16:{
			cvdebug() << " 16 bit \n";
			C3DUSImage *image = new C3DUSImage(image_info.size);
			std::shared_ptr<C3DImage > pimage(image);
			if (load_image(*image, image_info.voxel, f)) {
				handle_endian(*image, image_info.lowendian);
				result->push_back(pimage);
			}
		}break;
		case 32: {
			cvdebug() << " 32 bit \n";
			C3DUIImage *image = new C3DUIImage(image_info.size);
			std::shared_ptr<C3DImage > pimage(image);
			if (load_image(*image, image_info.voxel, f)) {
				handle_endian(*image, image_info.lowendian);
				result->push_back(pimage);
			}
		}break;
		default:
			cverr() << image_info.bpp << " unsigned images not supported" << "\n";
		} break;
	case dt_float:
		switch (image_info.bpp) {
		case 32: {
			cvdebug() << "INRIA: load float \n";
			C3DFImage *image = new C3DFImage(image_info.size);
			std::shared_ptr<C3DImage > pimage(image);
			if (load_image(*image, image_info.voxel, f)) {
				handle_endian(*image, image_info.lowendian);
				result->push_back(pimage);
			}
		}break;

		case 64:{
			cvdebug() << "INRIA: load double \n";
			C3DDImage *image = new C3DDImage(image_info.size);
			std::shared_ptr<C3DImage > pimage(image);
			if (load_image(*image, image_info.voxel, f)) {
				handle_endian(*image, image_info.lowendian);
				result->push_back(pimage);
			}
		}break;
		default:
			cverr() << image_info.bpp << " bit float images not supported" << "\n";
		}break;
	default:
		cverr() << "unknown image type" << "\n";
	}

	cvdebug() << " got " << result->size() <<" image(s)\n";

	if (result->size() > 0)
		return result;

	return PData();
}

template <typename T>
struct saver {
	static bool apply(const T3DImage<T>& image, COutputFile& f) {
		C3DFVector voxel = image.get_voxel_size();

		stringstream os;
		os << "#INRIMAGE-4#{" << "\n";
		os << "XDIM=" << image.get_size().x << "\n";
		os << "YDIM=" << image.get_size().y << "\n";
		os << "ZDIM=" << image.get_size().z << "\n";
		os << "VDIM=1" << "\n";
		os << "VX=" << voxel.x << "\n";
		os << "VY=" << voxel.y << "\n";
		os << "VZ=" << voxel.z << "\n";

// replace this by a template specialisation

		if (typeid(T) == typeid(double)) {
			os << "TYPE=float" << "\n";
		}else if (typeid(T) == typeid(float)) {
			os << "TYPE=float" << "\n";
		}else if (typeid(T) == typeid(unsigned int)) {
			os << "TYPE=unsigned fixed" << "\n";
		}else if (typeid(T) == typeid(signed int)) {
			os << "TYPE=signed fixed" << "\n";
		}else if (typeid(T) == typeid(unsigned short)) {
			os << "TYPE=unsigned fixed" << "\n";
		}else if (typeid(T) == typeid(signed short)) {
			os << "TYPE=signed fixed" << "\n";
		}else if (typeid(T) == typeid(unsigned char)) {
			os << "TYPE=unsigned fixed" << "\n";
		}else if (typeid(T) == typeid(signed char)) {
			os << "TYPE=signed fixed" << "\n";
		}else {
			os << "TYPE=signed fixed" << "\n";

		}
		os << "PIXSIZE="<< 8 * sizeof(T) << "\n";
		os << "SCALE=2**0" << "\n";
		os << "CPU=" << (my_endianess() ? "pc": "sgi") << "\n";

		while (os.str().size() < 252) os << "\n";
		os << "##}\n" << "\n";

		if (fwrite(os.str().c_str(), 1, 256, f) != 256) {
			cverr() << "CInriaSaver:" << strerror(errno) << "\n";
			return false;
		}
		if (fwrite(&image[0],sizeof(T), image.size(), f) != image.size()) {
			cverr() << "CInriaSaver:" << strerror(errno) << "\n";
			return false;
		}
		return true;

	}
};

template <>
struct saver<bool> {
	static bool apply(const T3DImage<bool>& /*image*/, COutputFile& /*f*/) {
		throw invalid_argument("INRIA images don't support bit images\n");

	}
};

class CInriaSaver: public TFilter<bool> {
public:
	CInriaSaver(COutputFile& f):
		m_f(f)
	{
	}

	template <class T>
	bool operator ()(const T3DImage<T>& image) const
	{
		return saver<T>::apply(image, m_f);
	}


private:
	COutputFile& m_f;
};

bool CInria3DImageIOPlugin::do_save(string const&  filename, const C3DImageVector& data) const
{
	if (data.size() != 1) {
		cverr() << "CInria3DImageIO::save: support only 1 image\n";
		return false;
	}

	COutputFile f(filename);

	if (!f) {
		cverr() << filename.c_str() << ":" << strerror(errno) << "\n";
		return false;
	}

	CInriaSaver s(f);

	return filter(s, **data.begin());
}


const std::string CInria3DImageIOPlugin::do_get_preferred_suffix() const
{
	return "inr"; 
}

const string  CInria3DImageIOPlugin::do_get_descr() const
{
	return string("INRIA image");
}


NS_END
