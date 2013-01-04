/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <mia/2d/vfio.hh>



NS_BEGIN(EXRIO_VF)

NS_MIA_USE

using namespace std;
using namespace boost;
using namespace Imf;
using namespace Imath;

class CEXR2DVFIOPlugin : public C2DVFIOPlugin {
public:
	CEXR2DVFIOPlugin();
private:
	PData do_load(const string& fname) const;
	bool do_save(const string& fname, const Data& data) const;
	const string do_get_descr() const;
};

CEXR2DVFIOPlugin::CEXR2DVFIOPlugin():
	C2DVFIOPlugin("exr")
{
	add_supported_type(it_float);
	add_supported_type(it_uint);

	add_suffix(".exr");
	add_suffix(".EXR");
}

CEXR2DVFIOPlugin::PData  CEXR2DVFIOPlugin::do_load(const string& filename) const
{
	try {
		InputFile file (filename.c_str());

		Box2i dw = file.header().dataWindow();
		C2DBounds size(dw.max.x - dw.min.x + 1, dw.max.y - dw.min.y + 1);
		int dx = dw.min.x;
		int dy = dw.min.y;
		cvdebug() << "EXR:Get vf of size " <<size.x << ", "<< size.y << "\n";

		const ChannelList& channels = file.header().channels();
		FrameBuffer frameBuffer;

		std::shared_ptr<C2DIOVectorfield > vf(new C2DIOVectorfield(size));

		for (ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i) {

			const Channel& channel = i.channel();

			cvdebug() << "channel '"<< i.name() <<"' of type " << channel.type << "\n";
			switch (channel.type) {
			case FLOAT: {

				frameBuffer.insert ("X", Slice(FLOAT, (char*)(&(*vf)(0,0).x - (dx + size.x * dy)),
							       8, 8 * size.x, 1, 1, 0.0));
				frameBuffer.insert ("Y", Slice(FLOAT, (char*)(&(*vf)(0,0).y - (dx + size.x * dy)),
                                                                    8, 8 * size.x, 1, 1, 0.0));
			}break;
			default:
				throw invalid_argument("EXRVFIO::load: only FLOAT supported");
			};
		}
		file.setFrameBuffer (frameBuffer);
		file.readPixels (dw.min.y, dw.max.y);
		return vf;
	}
	catch (...) {
		return CEXR2DVFIOPlugin::PData();
	}


	return CEXR2DVFIOPlugin::PData();
}

bool CEXR2DVFIOPlugin::do_save(const string& fname, const Data& vf) const
{

	cvdebug() << "CEXR2DVFIOPlugin::do_save vf size ("<<
		vf.get_size().x << ", " << vf.get_size().y<<")\n";

	try {
		Header header (vf.get_size().x, vf.get_size().y);

		header.channels().insert ("X", Channel (FLOAT));
		header.channels().insert ("Y", Channel (FLOAT));

		OutputFile file (fname.c_str(), header);

		FrameBuffer frameBuffer;

		frameBuffer.insert ("X",                    // name
				    Slice (FLOAT,               // type
					   (char *) &vf(0,0).x, // base
					   8,           // xStride
					   8 * vf.get_size().x));     // yStride

		frameBuffer.insert ("Y",                         // name
				    Slice (FLOAT,                // type
					   (char *) &vf(0,0).y,  // base
					   8,                    // xStride
					   8 * vf.get_size().x));// yStride


		file.setFrameBuffer (frameBuffer);
		file.writePixels (vf.get_size().y);
	}
	catch (...) {
		return false;
	}

	return true;
}

const string CEXR2DVFIOPlugin::do_get_descr() const
{
	return "a 2dvf io plugin for OpenEXR vfs";
}

extern "C" EXPORT  CPluginBase *get_plugin_interface()
{
	return new CEXR2DVFIOPlugin();
}

NS_END

