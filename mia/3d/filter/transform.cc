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

#include <mia/3d/filter/transform.hh>
#include <mia/3d/transformio.hh>

NS_BEGIN( transform_3dimage_filter)

NS_MIA_USE; 

C3DTransform::C3DTransform(const std::string& name, const std::string& kernel, const std::string& bc):
m_name(name), 
	m_kernel(kernel), 
	m_bc(bc)
{
}

mia::P3DImage C3DTransform::do_filter(const mia::C3DImage& image) const
{
        auto transform = load_transform<P3DTransformation>(m_name); 
	if (!m_kernel.empty()) {
		if (m_bc.empty()) {
			cvdebug() << "Override interpolator with kernel="
				  << m_kernel << " and mirror boundary conditions\n"; 
			C3DInterpolatorFactory ipf(m_kernel, "mirror");
			transform->set_interpolator_factory(ipf); 
		}else{
			cvdebug() << "Override interpolator with kernel="
				  << m_kernel << " and "<< m_bc << " boundary conditions\n"; 
			C3DInterpolatorFactory ipf(m_kernel, m_bc);
			transform->set_interpolator_factory(ipf); 
		}
	}
	
        return (*transform)(image); 
	
}

C3DTransformFilterPluginFactory::C3DTransformFilterPluginFactory(): 
	C3DFilterPlugin("transform")
{
	add_parameter("file", new CStringParameter(m_filename, CCmdOptionFlags::required_input,
						   "Name of the file containing the transformation.", 
						   &C3DTransformationIOPluginHandler::instance()));

	
	add_parameter("imgkernel", new CStringParameter(m_interpolator_kernel, CCmdOptionFlags::none, 
							"override image interpolator kernel", 
							&CSplineKernelPluginHandler::instance()));
	add_parameter("imgboundary", new CStringParameter(m_interpolator_bc, CCmdOptionFlags::none, 
							  "override image interpolation boundary conditions", 
							  &CSplineBoundaryConditionPluginHandler::instance())); 
 

}

mia::C3DFilter *C3DTransformFilterPluginFactory::do_create()const
{
	

	return new C3DTransform(m_filename, m_interpolator_kernel, m_interpolator_bc); 
}

const std::string C3DTransformFilterPluginFactory::do_get_descr()const
{
	return "Transform the input image with the given transformation."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DTransformFilterPluginFactory();
}
NS_END
