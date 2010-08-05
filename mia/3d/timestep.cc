/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#include <mia/core/export_handler.hh>
#include <mia/3d/timestep.hh>

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>


NS_MIA_BEGIN

C3DRegTimeStep::C3DRegTimeStep(float min, float max):
	_M_min(min),
	_M_max(max),
	_M_current((max - min)/ 2.0 + min ),
	_M_step((max-min) / 20)
{
}

C3DRegTimeStep::~C3DRegTimeStep()
{

}

void C3DRegTimeStep::apply(const C3DFVectorfield& infield, C3DFVectorfield& outfield, float delta)
{
	C3DFVectorfield::const_iterator inf = infield.begin();
	C3DFVectorfield::const_iterator enf = infield.end();
	C3DFVectorfield::iterator onf = outfield.begin();

	while (inf != enf)
		*onf++ += delta * *inf++;
}


bool C3DRegTimeStep::decrease()
{
	_M_current /= 2.0;
	if (_M_current < _M_min) {
		_M_current = _M_min;
		return false;
	}
	return true;
}

void C3DRegTimeStep::increase()
{
	_M_current *= 1.5;
	if (_M_current > _M_max)
		_M_current = _M_max;
}

float C3DRegTimeStep::calculate_pertuberation(C3DFVectorfield& io, const C3DFVectorfield& shift) const
{
	return do_calculate_pertuberation(io, shift);
}

bool C3DRegTimeStep::regrid_requested (const C3DFVectorfield& b, const C3DFVectorfield& v, float delta) const
{
	return do_regrid_requested(b, v, delta);
}

bool C3DRegTimeStep::has_regrid () const
{
	return do_has_regrid();
}
float C3DRegTimeStep::get_delta(float maxshift) const
{
	assert(maxshift > 0.0f);
	return (_M_current / maxshift);
}

C3DRegTimeStepPlugin::C3DRegTimeStepPlugin(const char *name):
	TFactory<C3DRegTimeStep, C3DImage, timestep_type>(name),
	_M_min(0.1),
	_M_max(2.0)
{
	typedef CParamList::PParameter PParameter;
	add_parameter("min", new CFloatParameter(_M_min, 0.001, numeric_limits<float>::max(),
							   false, "minimum time step allowed"));
	add_parameter("max", new CFloatParameter(_M_max, 0.002, numeric_limits<float>::max(),
							       false, "maximum time step allowed"));
}

float C3DRegTimeStepPlugin::get_min_timestep() const
{
	return _M_min;
}

float C3DRegTimeStepPlugin::get_max_timestep() const
{
	return _M_max;
}

template class TPlugin<C3DImage, timestep_type>;
template class TFactory<C3DRegTimeStep, C3DImage, timestep_type>;
template class THandlerSingleton<TFactoryPluginHandler<C3DRegTimeStepPlugin> >;
template class TFactoryPluginHandler<C3DRegTimeStepPlugin>;
template class TPluginHandler<C3DRegTimeStepPlugin>;

NS_MIA_END
