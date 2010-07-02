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

#ifdef WIN32
#  define EXPORT_HANDLER __declspec(dllexport)
#else
#  define EXPORT_HANDLER
#endif

#include <mia/core/factory.hh>
#include <mia/2d/timestep.hh>

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

C2DRegTimeStep::C2DRegTimeStep(float min, float max):
	_M_min(min),
	_M_max(max),
	_M_current((max - min)/ 2.0 + min ),
	_M_step((max-min) / 20)
{
}

C2DRegTimeStep::~C2DRegTimeStep()
{

}

bool C2DRegTimeStep::decrease()
{
	_M_current /= 2.0;
	if (_M_current < _M_min) {
		_M_current = _M_min;
		return false;
	}
	return true;
}

void C2DRegTimeStep::increase()
{
	_M_current *= 1.5;
	if (_M_current > _M_max)
		_M_current = _M_max;
}

float C2DRegTimeStep::calculate_pertuberation(C2DFVectorfield& io, const C2DTransformation& shift) const
{
	return do_calculate_pertuberation(io, shift);
}

bool C2DRegTimeStep::regrid_requested (const C2DTransformation& b, const C2DFVectorfield& v, float delta) const
{
	return do_regrid_requested(b, v, delta);
}

bool C2DRegTimeStep::has_regrid () const
{
	return do_has_regrid();
}
float C2DRegTimeStep::get_delta(float maxshift) const
{
	assert(maxshift > 0.0f);
	return (_M_current / maxshift);
}

C2DRegTimeStepPlugin::C2DRegTimeStepPlugin(const char *name):
	TFactory<C2DRegTimeStep, C2DImage, timestep_type>(name),
	_M_min(0.1),
	_M_max(2.0)
{
	add_parameter("min", new CFloatParameter(_M_min, 0.001, numeric_limits<float>::max(),
							   false, "minimum time step allowed"));
	add_parameter("max", new CFloatParameter(_M_max, 0.002, numeric_limits<float>::max(),
							       false, "maximum time step allowed"));
}

float C2DRegTimeStepPlugin::get_min_timestep() const
{
	return _M_min;
}

float C2DRegTimeStepPlugin::get_max_timestep() const
{
	return _M_max;
}


template class TPlugin<C2DImage,timestep_type>;
template class TFactory<C2DRegTimeStep, C2DImage, timestep_type>;
template class TFactoryPluginHandler<C2DRegTimeStepPlugin>;
template class TPluginHandler<C2DRegTimeStepPlugin>;
template class THandlerSingleton<TFactoryPluginHandler<C2DRegTimeStepPlugin> >;

NS_MIA_END
