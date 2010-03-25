/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010, Gert Wollny
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#include <cassert>
#include <mia/2d/costbase.hh>

#include <boost/lambda/lambda.hpp>
#include <algorithm>

#ifdef WIN32
#  define EXPORT_HANDLER __declspec(dllexport)
#else
#  define EXPORT_HANDLER
#endif
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>


NS_MIA_BEGIN

using boost::lambda::_1; 
using boost::lambda::_2; 

C2DCostBase::C2DCostBase(float weight):
	_M_weight(weight)
{
	
}

double C2DCostBase::evaluate(const C2DTransformation& t, C2DFVectorfield& force) const
{
	assert(t.get_size() == force.get_size()); 
	C2DFVectorfield f(force.get_size()); 
	
	const double result = _M_weight * do_evaluate(t, f); 
	transform(f.begin(), f.end(), force.begin(), force.begin(),( _1 * _M_weight) + _2); 
	
	return result; 
}

const char *cost_data2d_type::type_descr = "2d"; 

template class EXPORT_HANDLER TPlugin<cost_data2d_type, cost_type>;
template class EXPORT_HANDLER TFactory<C2DCostBase, cost_data2d_type, cost_type>;
template class EXPORT_HANDLER TFactoryPluginHandler<C2DCostBasePlugin>;
template class EXPORT_HANDLER THandlerSingleton<TFactoryPluginHandler<C2DCostBasePlugin> >;
template class EXPORT_HANDLER TPluginHandler<C2DCostBasePlugin>;

NS_MIA_END
