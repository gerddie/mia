/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <sstream>

#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>

#include <mia/core/file.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/2dimageio.hh>


NS_BEGIN(IMAGEIO_2D_EXR)

NS_MIA_USE;

using namespace std;
using namespace boost;
using namespace Imf;
using namespace Imath;

class CEXR2DImageIOPlugin : public C2DImageIOPlugin {
public:
	CEXR2DImageIOPlugin();
private:
	void do_add_suffixes(multimap<string, string>& map) const;
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
};

CEXR2DImageIOPlugin::CEXR2DImageIOPlugin():
	C2DImageIOPlugin("exr")
{
	add_supported_type(it_float);
	add_supported_type(it_uint);
}

void CEXR2DImageIOPlugin::do_add_suffixes(multimap<string, string>& map) const
{
	map.insert(pair<string,string>(".exr", get_name()));
	map.insert(pair<string,string>(".EXR", get_name()));
}


CEXR2DImageIOPlugin::PData  CEXR2DImageIOPlugin::do_load(const string& filename) const
{
	try {
		InputFile file (filename.c_str());

		Box2i dw = file.header().dataWindow();
		C2DBounds size(dw.max.x - dw.min.x + 1, dw.max.y - dw.min.y + 1);
		int dx = dw.min.x;
		int dy = dw.min.y;
		cvdebug() << "EXR:Get image of size " <<size.x << ", "<< size.y << "\n";

		const ChannelList& channels = file.header().channels();
		FrameBuffer frameBuffer;

		std::shared_ptr<C2DImageVector > result(new C2DImageVector);

		for (ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i) {

			const Channel& channel = i.channel();

			cvdebug() << "channel '"<< i.name() <<"' of type " << channel.type << "\n";
			switch (channel.type) {
			case UINT: {
				std::shared_ptr<C2DUIImage > img(new C2DUIImage(size));
				frameBuffer.insert ("Y", Slice(UINT, (char*)(&(*img)(0,0) - (dx + size.x * dy)),
							       4, 4 * size.x, 1, 1, 0.0));
				result->push_back(img);
			}break;
			case FLOAT: {
				std::shared_ptr<C2DFImage > img(new C2DFImage(size));
				frameBuffer.insert ("Y", Slice(FLOAT, (char*)(&(*img)(0,0) - (dx + size.x * dy)),
							       4, 4 * size.x, 1, 1, 0.0));
				result->push_back(img);
			}break;
			default:
				throw invalid_argument("EXRImageIO::load: only FLOAT and UINT supported");
			};
		}
		file.setFrameBuffer (frameBuffer);
		file.readPixels (dw.min.y, dw.max.y);
		return result;
	}
	catch (...) {
		return CEXR2DImageIOPlugin::PData();
	}


	return CEXR2DImageIOPlugin::PData();
}


struct CImageSaver {
	typedef bool result_type;

	CImageSaver(const string& filename):
		m_fname(filename)
	{
	}

	template <typename T>
	result_type operator ()(const T2DImage<T>& image) const;
private:
	bool write_header(int bpp, int compression, const C2DBounds& size);

	string m_fname;
};

template <typename T>
struct pixel_trait_exr
{
	enum {pixel_size = 0 };
	enum { supported = 0 };
};

template <>
struct pixel_trait_exr<float> {
	enum {pixel_size = 4 };
	enum { supported = 1 };
	static PixelType pt() {
		return Imf::FLOAT;
	}
};

template <>
struct pixel_trait_exr<unsigned int> {
	static PixelType pt() {
		return Imf::UINT;
	}
	enum {pixel_size = 4 };
	enum { supported = 1 };
};

template <typename T, int supported>
struct image_writer_exr {
	static bool apply (const T2DImage<T>& /*image*/, const string& /*fname*/) {
		stringstream msg;
		msg << "Pixel type '" << CPixelTypeDict.get_name((EPixelType)pixel_type<T>::value) << "' not supported by 'exr' file format";

		throw runtime_error(msg.str());
	}
};

template <typename T>
struct image_writer_exr<T, 1>  {
	static bool apply (const T2DImage<T>& image, const string& fname) {
		cvdebug() << "image_writer<T, 1>::apply image size ("<<
			image.get_size().x << ", " << image.get_size().y<<")\n";

		try {
			Header header (image.get_size().x, image.get_size().y);
			header.channels().insert ("Y", Channel (pixel_trait_exr<T>::pt()));

			OutputFile file (fname.c_str(), header);

			FrameBuffer frameBuffer;

			frameBuffer.insert ("Y",                    // name
					    Slice (pixel_trait_exr<T>::pt(),               // type
						   (char *) &image(0,0), // base
						   pixel_trait_exr<T>::pixel_size,           // xStride
						   pixel_trait_exr<T>::pixel_size * image.get_size().x));     // yStride

			file.setFrameBuffer (frameBuffer);
			file.writePixels (image.get_size().y);
			return true;
		}
		catch (...) {
			return false;
		}

	}
};

template <typename T>
CImageSaver::result_type
CImageSaver::operator()(const T2DImage<T>& image) const
{
	return image_writer_exr<T, pixel_trait_exr<T>::supported>::apply(image,m_fname);
}



bool CEXR2DImageIOPlugin::do_save(const string& fname, const Data& data) const
{
	cvdebug() << "CEXR2DImageIO::save begin\n";

	if (data.size() != 1)
		throw runtime_error("CEXR2DImageIO::save: only single images can be saved");

	CImageSaver saver(fname);

	for (C2DImageVector::const_iterator iimg = data.begin(); iimg != data.end(); ++iimg)
		filter(saver, **iimg);

	cvdebug() << "CEXR2DImageIO::save end\n";
	return true;
}

const string CEXR2DImageIOPlugin::do_get_descr() const
{
	return "a 2dimage io plugin for OpenEXR images";
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	return new CEXR2DImageIOPlugin();
}

NS_END

