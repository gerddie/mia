/* -*- mia-c++  -*-
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_2d_2dimageui_hh
#define mia_2d_2dimageui_hh

#include <set>
#include <vector>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>
#include <mia/2d/2DImage.hh>

NS_MIA_BEGIN

class EXPORT_2D C2DImageVector: public C2DImageSeries,
		      public CIOData {
 public:
	C2DImageVector *clone() const;
};


class EXPORT_2D io_2dimage_type {
public:
	typedef  C2DImageVector type;
	static const char *type_descr;
};


typedef TIOPlugin<io_2dimage_type> C2DImageIOPlugin;

class EXPORT_2D C2DImageIOPPH: public TIOPluginHandler<C2DImageIOPlugin> {
public:
	typedef TIOPluginHandler<C2DImageIOPlugin>::CSuffixmap CSuffixmap;
	typedef TIOPluginHandler<C2DImageIOPlugin>::PData PData;
protected:
	C2DImageIOPPH(const std::list< ::boost::filesystem::path>& searchpath);
};

typedef TIOHandlerSingleton< C2DImageIOPPH > C2DImageIOPluginHandler;
typedef C2DImageIOPluginHandler::Instance::DataKey C2DImageDataKey;

typedef C2DImageIOPluginHandler::Instance::PData P2DImageVector;
P2DImageVector EXPORT_2D create_image2d_vector(P2DImage image);

P2DImage load_image2d(const std::string& filename);
bool save_image2d(const std::string& filename, P2DImage image);

NS_MIA_END

#endif
