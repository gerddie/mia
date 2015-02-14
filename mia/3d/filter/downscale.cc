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

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/3d/filter/downscale.hh>

#include <limits>

NS_BEGIN(downscale_3dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs= ::boost::filesystem;

CDownscale::CDownscale(const C3DBounds& block_size, const string& filter):
	m_block_size(block_size)
{
	stringstream fcompose;

	fcompose << "sepconv:kx=["<< filter <<":w=" << block_size.x / 2
		 << "],ky=["<< filter <<":w=" << block_size.y / 2
		 << "],kz=["<< filter <<":w=" << block_size.z / 2
		 << "]";
	cvdebug() << "CDownscale::CDownscale: smoothing kernel '" << fcompose.str() << "'\n";

	m_smoothing = C3DFilterPluginHandler::instance().produce(fcompose.str().c_str());

	if (!m_smoothing)
		throw invalid_argument(string("smoothing filter creation from '") +
				       fcompose.str() + string("failed"));
	else
		cvdebug() << " Created filter with use count:" <<m_smoothing.use_count() << "\n";

}

CDownscale::~CDownscale()
{
	cvdebug() << " Release filter with use count:" <<m_smoothing.use_count() << "\n";
}

template <class T>
CDownscale::result_type CDownscale::operator () (const T3DImage<T>& src) const
{
	cvdebug() << "CDownscale::operator () begin\n";
	T3DImage<T> *fresult = new T3DImage<T>(
		       C3DBounds((src.get_size().x + m_block_size.x - 1) / m_block_size.x,
				 (src.get_size().y + m_block_size.y - 1) / m_block_size.y,
				 (src.get_size().z + m_block_size.z - 1) / m_block_size.z), src);

	CDownscale::result_type Result(fresult);

	typename T3DImage<T>::iterator i = fresult->begin();
	C3DBounds Start(m_block_size.x/2,m_block_size.y/2,m_block_size.z/2);

	// Put the Blockaverages into the target
	for (size_t z = 0; z < fresult->get_size().z; Start.z += m_block_size.z, ++z){
		Start.y = m_block_size.y/2; 
		for (size_t y = 0; y < fresult->get_size().y; Start.y += m_block_size.y, ++y){
			Start.x = m_block_size.x/2; 
			for (size_t x = 0; x < fresult->get_size().x; Start.x += m_block_size.x, ++x, ++i){
				*i = src(Start);
			}
		}
	}


	C3DFVector pixel_size = src.get_voxel_size();
	pixel_size.x *= m_block_size.x;
	pixel_size.y *= m_block_size.y;
	pixel_size.z *= m_block_size.z;
	fresult->set_voxel_size(pixel_size);
	cvdebug() << "CDownscale::operator () end\n";
	return Result;
}

CDownscale::result_type CDownscale::do_filter(const C3DImage& image) const
{
	std::shared_ptr<C3DImage > smooth_image = m_smoothing->filter(image);
	return mia::filter(*this, *smooth_image);
}


C3DDownscaleFilterPlugin::C3DDownscaleFilterPlugin():
	C3DFilterPlugin("downscale"),
	m_b(1,1,1), 
	m_filter("gauss")
{
	add_parameter("bx", make_lc_param(m_b.x, 1, false, "blocksize in x direction"));
	
	add_parameter("by", make_lc_param(m_b.y, 1,false, "blocksize in y direction"));
	
	add_parameter("bz", make_lc_param(m_b.z, 1, false, "blocksize in z direction"));

	add_parameter("b", new C3DBoundsParameter(m_b, false, "blocksize"));


	add_parameter("kernel", new CStringParameter(m_filter, CCmdOptionFlags::none, 
						     "smoothing filter kernel to be applied, the "
						     "size of the filter is estimated based on the blocksize.", 
						     &C1DSpacialKernelPluginHandler::instance())); 
}

C3DFilter *C3DDownscaleFilterPlugin::do_create()const
{
	return new CDownscale(m_b, m_filter);
}

const string C3DDownscaleFilterPlugin::do_get_descr()const
{
	return "Downscale the input image by using a given block size to define the "
		"downscale factor. Prior to scaling the image is filtered by a smoothing filter to eliminate "
		"high frequency data and avoid aliasing artifacts.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DDownscaleFilterPlugin();
}

NS_END
