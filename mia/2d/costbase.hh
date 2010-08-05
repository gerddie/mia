/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

#include <mia/2d/transform.hh>
#include <mia/core/cost.hh>


NS_MIA_BEGIN

struct EXPORT_2D cost_data2d_type {
	static const char *type_descr;
};


class EXPORT_2D C2DCostBase : public CProductBase {
public:
	C2DCostBase(float weight);

	double evaluate(const C2DTransformation& t, C2DFVectorfield& force) const;
private:
	virtual double do_evaluate(const C2DTransformation& t, C2DFVectorfield& force) const = 0;
	float _M_weight;
};

class EXPORT_CORE C2DCostBasePlugin: public TFactory<C2DCostBase, cost_data2d_type, cost_type> {
public:
	C2DCostBasePlugin(const char *const name);
protected:
	float get_weight() const;
private:
	float _M_weight;
};


typedef THandlerSingleton<TFactoryPluginHandler<C2DCostBasePlugin> > C2DCostBasePluginHandler;
typedef SHARED_PTR(C2DCostBase) P2DCostBase;

NS_MIA_END
