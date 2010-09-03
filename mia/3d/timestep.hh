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
#ifndef mia_3d_timestep_hh
#define mia_3d_timestep_hh

#include <mia/core/factory.hh>
#include <mia/core/typedescr.hh>
#include <mia/3d/3DImage.hh>

NS_MIA_BEGIN

class EXPORT_3D C3DRegTimeStep :public CProductBase {
public:
	typedef C3DImage plugin_data; 
	typedef timestep_type plugin_type; 

	C3DRegTimeStep(float min, float max);

	virtual ~C3DRegTimeStep();

	void apply(const C3DFVectorfield& infield, C3DFVectorfield& outfield, float scale);

	float calculate_pertuberation(C3DFVectorfield& io, const C3DFVectorfield& shift) const;

	bool decrease();

	void increase();

	float get_delta(float maxshift) const;

	bool regrid_requested (const C3DFVectorfield& b, const C3DFVectorfield& v, float delta) const;

	bool has_regrid () const;

private:
	virtual float do_calculate_pertuberation(C3DFVectorfield& io, const C3DFVectorfield& shift) const = 0;
	virtual bool do_regrid_requested (const C3DFVectorfield& b, const C3DFVectorfield& v, float delta) const = 0;
	virtual bool do_has_regrid () const = 0;


	float _M_min;
	float _M_max;
	float _M_current;
	float _M_step;
};

class EXPORT_3D C3DRegTimeStepPlugin : public TFactory<C3DRegTimeStep>
{
public:
	C3DRegTimeStepPlugin(const char *name);
protected:
	float get_min_timestep() const;
	float get_max_timestep() const;
	float _M_min;
	float _M_max;
};

typedef std::shared_ptr<C3DRegTimeStep > P3DRegTimeStep;
typedef THandlerSingleton<TFactoryPluginHandler<C3DRegTimeStepPlugin> > C3DRegTimeStepPluginHandler;

NS_MIA_END

#endif
