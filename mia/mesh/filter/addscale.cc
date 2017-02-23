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

#include <mia/mesh/filter/addscale.hh>
#include <mia/3d/imageio.hh>
#include <mia/3d/interpolator.hh>

NS_BEGIN(mia_meshfilter_addscale) 
using namespace mia; 
using namespace std; 


CAddScaleMeshFilter::CAddScaleMeshFilter(const string& image_filename ):
	m_image_filename(image_filename) 
{
}
	
struct FSetScale: public TFilter<void> {
        FSetScale(CTriangleMesh& mesh): m_mesh(mesh){}; 
        
        template <typename T> 
        void operator ()( const T3DImage<T>& image) {
                C3DInterpolatorFactory ipf("bspline:d=1", "zero");
                unique_ptr<T3DConvoluteInterpolator<T>> ip(ipf.create(image.data())); 
                
		
                transform(m_mesh.vertices_begin(), m_mesh.vertices_end(), 
                          m_mesh.scale_begin(), [&ip](const C3DFVector& v) -> float {
                                  return (*ip)(v); 
                          });
        }
private: 
        CTriangleMesh& m_mesh; 
}; 

PTriangleMesh CAddScaleMeshFilter::do_filter(const CTriangleMesh& mesh) const
{
        PTriangleMesh result = make_shared<CTriangleMesh>(mesh); 
        P3DImage image = load_image3d(m_image_filename); 
        
        FSetScale ssc(*result); 
        mia::accumulate(ssc, *image); 

        return result; 
}

CAddScaleMeshFilterPlugin::CAddScaleMeshFilterPlugin():
	CMeshFilterPlugin("addscale")
{
	add_parameter("img", make_param(m_image_filename, true, 
					"3DImage that contains the scale values as a volume image."));
}

mia::CMeshFilter *CAddScaleMeshFilterPlugin::do_create()const
{
	return new CAddScaleMeshFilter(m_image_filename);
}

const std::string CAddScaleMeshFilterPlugin::do_get_descr()const
{
	return "This filter sorts sets the scale parameter for each vertex from a 3D image. "
                "Image and mesh are expected to be in the same physical coordinate space."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CAddScaleMeshFilterPlugin();
}

NS_END
