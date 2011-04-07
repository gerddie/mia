/*  -*- mia-c++ -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
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

NS_BEGIN(IMAGEIO_2D_RAW)

NS_MIA_USE
using namespace std;
using namespace boost;



class CRaw2DImageIO: public C2DImageIOPlugin {
public:
	CRaw2DImageIO();

	friend struct C2DRawImageSaver;

private:
	void do_add_suffixes(multimap<string, string>& map) const;
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
}

void CRaw2DImageIO::do_add_suffixes(multimap<string, string>& map) const
{
	map.insert(pair<string,string>(".raw", get_name()));
	map.insert(pair<string,string>(".RAW", get_name()));
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
