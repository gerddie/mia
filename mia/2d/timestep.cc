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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
  LatexBeginPluginSection{2D time steps for model based registration}
  \label{sec:timestep2d}
  
  These plug-ins refer to non-linear registration approaches that use 
  a PDE based model for the regularization of the transformation.
  The plug-ins implement different time step models. 

  LatexEnd

*/


#include <mia/core/export_handler.hh>

#include <mia/core/factory.hh>
#include <mia/2d/timestep.hh>

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

C2DRegTimeStep::C2DRegTimeStep(float min, float max):
	m_min(min),
	m_max(max),
	m_current((max - min)/ 2.0 + min ),
	m_step((max-min) / 20)
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
	assert(maxshift > 0.0f);
	return (m_current / maxshift);
}

C2DRegTimeStepPlugin::C2DRegTimeStepPlugin(const char *name):
	TFactory<C2DRegTimeStep>(name),
	m_min(0.1),
	m_max(2.0)
{
	add_parameter("min", new CFloatParameter(m_min, 0.001, numeric_limits<float>::max(),
							   false, "minimum time step allowed"));
	add_parameter("max", new CFloatParameter(m_max, 0.002, numeric_limits<float>::max(),
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


EXPLICIT_INSTANCE_PLUGIN(C2DRegTimeStep); 
EXPLICIT_INSTANCE_PLUGIN_HANDLER(C2DRegTimeStepPlugin); 

NS_MIA_END
