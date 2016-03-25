/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/core/export_handler.hh>

#include <mia/core/factory.hh>
#include <mia/2d/timestep.hh>

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN
using std::numeric_limits; 

C2DRegTimeStep::C2DRegTimeStep(float min, float max):
	m_min(min),
	m_max(max),
	m_current((max - min)/ 2.0 + min )
{
}

C2DRegTimeStep::~C2DRegTimeStep()
{

}

bool C2DRegTimeStep::decrease()
{
	if (m_current <= m_min)
		return false;
	
	m_current /= 2.0;
	if (m_current <= m_min)
		m_current = m_min;
	return true;
}

void C2DRegTimeStep::increase()
{
	m_current *= 1.5;
	if (m_current > m_max)
		m_current = m_max;
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
	// maxshift should be the result of a norm() and hence not negative
	// if maxshift == 0.0 then the registration is finished and we shouldn't call this function 
	assert(maxshift > 0.0f);
	return m_current / maxshift;
}

C2DRegTimeStepPlugin::C2DRegTimeStepPlugin(const char *name):
	TFactory<C2DRegTimeStep>(name),
	m_min(0.1),
	m_max(2.0)
{
	add_parameter("min", new CFBoundedParameter(m_min, EParameterBounds::bf_min_open, {0.0},
						    false, "minimum time step allowed"));
	add_parameter("max", new CFBoundedParameter(m_max, EParameterBounds::bf_min_open, {0.0},
						    false, "maximum time step allowed"));
}

float C2DRegTimeStepPlugin::get_min_timestep() const
{
	return m_min;
}

float C2DRegTimeStepPlugin::get_max_timestep() const
{
	return m_max;
}



template <> const char *  const 
TPluginHandler<C2DRegTimeStepPlugin>::m_help =  
   "These plug-ins define tifferent types of time-steps for time marching registration algorithms.";

EXPLICIT_INSTANCE_PLUGIN(C2DRegTimeStep); 
EXPLICIT_INSTANCE_PLUGIN_HANDLER(C2DRegTimeStepPlugin); 


NS_MIA_END
