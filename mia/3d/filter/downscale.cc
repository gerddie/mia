/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

/* 
   LatexBeginPluginDescription{3D image filters}
   
   \subsection{Downscale filter}
   \label{filter3d:downscale}
   
   \begin{description}
   
   \item [Plugin:] downscale
   \item [Description:] Downscale the input image by using a given block size to define the 
            downscale factor. Prior to scaling the image is filtered by a smoothing filter to eliminate 
	    high frequency data and avoid aliasing artifacts. 
   \item [Input:] Abitrary gray scale or binary image 
   \item [Output:] The downscaled image.  
   
   \plugtabstart
   bx & int & downscale factor in x direction & 1  \\
   by & int & downscale factor in y direction & 1  \\
   bz & int & downscale factor in z direction & 1  \\
   b & string & Alternative to define the scaling factors as a 3D vector & <1,1,1>  \\
   kernel & string & Base type of the smoothing kernel, the filter kernel width is estimated based 
       on the downscale factors & gauss \\\hline 
   \plugtabend
   
   \end{description}

   LatexEnd  
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
				 (src.get_size().z + m_block_size.z - 1) / m_block_size.z));

	CDownscale::result_type Result(fresult);

	typename T3DImage<T>::iterator i = fresult->begin();
	C3DBounds Start(m_block_size.x/2,m_block_size.y/2,m_block_size.z/2);

	// Put the Blockaverages into the target
	for (; Start.z < src.get_size().z; Start.z += m_block_size.z){
		for (Start.y = 0; Start.y < src.get_size().y; Start.y += m_block_size.y){
			for (Start.x = 0; Start.x < src.get_size().x; Start.x += m_block_size.x,++i){
				*i = src(Start);
			}
		}
	}


	C3DFVector pixel_size = src.get_voxel_size();
	pixel_size.x /= m_block_size.x;
	pixel_size.y /= m_block_size.y;
	pixel_size.z /= m_block_size.z;
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
	add_parameter("bx", new CUIntParameter(m_b.x, 1,
					       numeric_limits<int>::max(), false,
					       "blocksize in x direction"));
	
	add_parameter("by", new CUIntParameter(m_b.y, 1,
					       numeric_limits<int>::max(), false,
					       "blocksize in y direction"));
	
	add_parameter("bz", new CUIntParameter(m_b.z, 1,
					       numeric_limits<int>::max(), false,
                                                "blocksize in z direction"));

	add_parameter("b", new C3DBoundsParameter(m_b, false, "blocksize"));

	add_parameter("kernel", new CStringParameter(m_filter, false,
                                                "smoothing filter kernel to be applied"));
}

C3DDownscaleFilterPlugin::ProductPtr C3DDownscaleFilterPlugin::do_create()const
{
	return C3DDownscaleFilterPlugin::ProductPtr(new CDownscale(m_b, m_filter));
}

void C3DDownscaleFilterPlugin::prepare_path() const
{
}


bool  C3DDownscaleFilterPlugin::do_test() const
{
	return false;
}

const string C3DDownscaleFilterPlugin::do_get_descr()const
{
	return "3D image downscaler filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DDownscaleFilterPlugin();
}

NS_END
