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

#ifndef mia_2d_timestep_hh
#define mia_2d_timestep_hh

#include <mia/core/factory.hh>
#include <mia/core/typedescr.hh>
#include <mia/2d/2DImage.hh>
#include <mia/2d/transform.hh>

NS_MIA_BEGIN


class EXPORT_2D C2DRegTimeStep : public CProductBase {
public:
	C2DRegTimeStep(float min, float max);



	virtual ~C2DRegTimeStep();

	float calculate_pertuberation(C2DFVectorfield& io, const C2DTransformation& shift) const;
	bool regrid_requested(const C2DTransformation& b, const C2DFVectorfield& v, float delta) const;
	bool decrease();
	void increase();
	float get_delta(float maxshift) const;
	bool has_regrid () const;

private:
	virtual float do_calculate_pertuberation(C2DFVectorfield& io, const C2DTransformation& shift) const = 0;
	virtual bool do_regrid_requested (const C2DTransformation& b, const C2DFVectorfield& v,
					  float delta) const = 0;

	virtual bool do_has_regrid () const = 0;


	float _M_min;
	float _M_max;
	float _M_current;
	float _M_step;
};


typedef std::shared_ptr<C2DRegTimeStep > P2DRegTimeStep;

class EXPORT_2D C2DRegTimeStepPlugin : public TFactory<C2DRegTimeStep, C2DImage, timestep_type>
{
public:
	C2DRegTimeStepPlugin(const char *name);
protected:
	float get_min_timestep() const;
	float get_max_timestep() const;
	float _M_min;
	float _M_max;
};

typedef THandlerSingleton<TFactoryPluginHandler<C2DRegTimeStepPlugin> > C2DRegTimeStepPluginHandler;

NS_MIA_END

#endif
