/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#define VSTREAM_DOMAIN "3dtimestep"
#include <mia/core/export_handler.hh>
#include <mia/3d/timestep.hh>

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>


NS_MIA_BEGIN

C3DRegTimeStep::C3DRegTimeStep(float min, float max):
	m_min(min),
	m_max(max),
	m_current((max - min)/ 2.0 + min ),
	m_step((max-min) / 20)
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
	m_current /= 2.0;
	if (m_current < m_min) {
		m_current = m_min;
		return false;
	}
	return true;
}

void C3DRegTimeStep::increase()
{
	m_current *= 1.5;
	if (m_current > m_max)
		m_current = m_max;
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
	return (m_current / maxshift);
}

C3DRegTimeStepPlugin::C3DRegTimeStepPlugin(const char *name):
	TFactory<C3DRegTimeStep>(name),
	m_min(0.1),
	m_max(2.0)
{
	add_parameter("min", make_positive_param(m_min, false, "minimum time step allowed"));
	add_parameter("max", make_positive_param(m_max, false, "maximum time step allowed"));
}

float C3DRegTimeStepPlugin::get_min_timestep() const
{
	return m_min;
}

float C3DRegTimeStepPlugin::get_max_timestep() const
{
	return m_max;
}

template <> const char *  const 
TPluginHandler<C3DRegTimeStepPlugin>::m_help =  
   "These plug-ins define tifferent types of time-steps for time marching registration algorithms.";

template class TPlugin<C3DImage, timestep_type>;
template class TFactory<C3DRegTimeStep>;
template class THandlerSingleton<TFactoryPluginHandler<C3DRegTimeStepPlugin> >;
template class TFactoryPluginHandler<C3DRegTimeStepPlugin>;
template class TPluginHandler<C3DRegTimeStepPlugin>;

NS_MIA_END
