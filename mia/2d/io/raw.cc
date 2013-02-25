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

#include <string>
#include <sstream>
#include <stdexcept>
#include <cassert>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/imageio.hh>

NS_BEGIN(IMAGEIO_2D_RAW)

NS_MIA_USE
using namespace std;
using namespace boost;



class CRaw2DImageIO: public C2DImageIOPlugin {
public:
	CRaw2DImageIO();

	friend struct C2DRawImageSaver;

private:
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
};

CRaw2DImageIO::CRaw2DImageIO():
	C2DImageIOPlugin("raw")
{
	add_supported_type(it_float);
	add_supported_type(it_double);
	add_supported_type(it_sint);
	add_supported_type(it_uint);
	add_supported_type(it_sshort);
	add_supported_type(it_ushort);
	add_supported_type(it_sbyte);
	add_supported_type(it_ubyte);
	add_supported_type(it_bit);

	add_suffix(".raw");
	add_suffix(".RAW");

}

CRaw2DImageIO::PData CRaw2DImageIO::do_load(string const& /*filename*/)const
{
	cvwarn() << "CRaw2DImageIO::do_load:  raw image reading currently not supported\n";
	return CRaw2DImageIO::PData();
}

struct C2DRawImageSaver {
	typedef bool result_type;

	C2DRawImageSaver(CFile& f):
		m_f(f)
	{
	}

	template <typename T>
	result_type operator()(const T2DImage<T>& image) const;
private:
	CFile& m_f;
};


template <typename T>
struct T2DRawImageWriter
{
	static bool apply (const T2DImage<T>& image, FILE *f) {
		return image.size() == fwrite(&image(0,0),sizeof(T), image.size(),  f);
	}
};

template <>
struct T2DRawImageWriter<bool>  {
	static bool apply (const C2DBitImage& image, FILE *f) {
		vector<char> buf(image.size(), 0);
		copy(image.begin(), image.end(), buf.begin());
		return buf.size() == fwrite(&buf[0],1, image.size(),  f);
	}
};


template <typename T>
C2DRawImageSaver::result_type
C2DRawImageSaver::operator()(const T2DImage<T>& image) const
{
	return T2DRawImageWriter<T>::apply(image, m_f);
}

///

bool CRaw2DImageIO::do_save(string const& filename, const C2DImageVector& data) const
{
	cvdebug() << "CRaw2DImageIO::save begin\n";

	COutputFile f(filename);
	if (!f) {
		cverr() << "CRaw2DImageIO::save:unable to open output file:" << filename << "\n";
		return false;
	}

	C2DRawImageSaver saver(f);

	for (C2DImageVector::const_iterator iimg = data.begin(); iimg != data.end(); ++iimg)
		filter(saver, **iimg);

	cvdebug() << "CRaw2DImageIO::save end\n";
	return true;
}

const string  CRaw2DImageIO::do_get_descr()const
{
	return string("RAW 2D-image output support");
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
		return new CRaw2DImageIO;
}


NS_END
