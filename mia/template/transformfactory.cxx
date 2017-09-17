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

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>
#include <mia/core/parameter.hh>

NS_MIA_BEGIN

template <typename T> 
TTransformCreator<T>::TTransformCreator(const InterpolatorFactory& ipf):
	m_ipf(ipf)
{
}

template <typename T> 
typename T::Pointer TTransformCreator<T>::create(const typename T::Size& size) const
{
	typename T::Pointer result = do_create(size, m_ipf);
	result->set_creator_string(get_init_string()); 
	return result; 
}

template <typename T> 
bool TTransformCreator<T>::has_property(const char *property) const
{
	return m_properties.find(property) != m_properties.end();
}

template <typename T> 
void TTransformCreator<T>::add_property(const char *property)
{
	m_properties.insert(property);
}


template <typename T> 
TTransformCreatorPlugin<T>::TTransformCreatorPlugin(const char *const name):
	TFactory<TTransformCreator<T> >(name)
{
	this->add_parameter("imgkernel", make_param(m_image_interpolator, "bspline:d=3", false, "image interpolator kernel"));
	this->add_parameter("imgboundary", make_param<CSplineBoundaryCondition>(m_image_boundary, "mirror", false, "image interpolation boundary conditions")); 
}

template <typename T> 
typename TTransformCreatorPlugin<T>::Product *
TTransformCreatorPlugin<T>::do_create() const
{
	InterpolatorFactory ipf(m_image_interpolator, *m_image_boundary); 
	return do_create(ipf); 
}

NS_MIA_END
