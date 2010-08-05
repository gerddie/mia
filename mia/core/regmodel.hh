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

#ifndef mia_core_regmodel_hh
#define mia_core_regmodel_hh

#include <mia/core/factory.hh>

NS_MIA_BEGIN

struct EXPORT_CORE regmodel_type {
	static const char *value;
};

/**
   Base class template for image registration models.
 */

template <typename Force, typename Transformation>
class TRegModel :public CProductBase {
public:
	TRegModel();
	virtual ~TRegModel();
	void solve (const Force& b, Transformation& x) const;
	float get_force_scale() const;
private:
	virtual void do_solve(const Force& b, Transformation& x) const = 0;
	virtual float do_get_force_scale() const;
};

//
// implementation part of the registration model
//

template <typename F, typename T>
TRegModel<F,T>::TRegModel()
{
}

template <typename F, typename T>
TRegModel<F,T>::~TRegModel()
{
}

template <typename F, typename T>
void TRegModel<F,T>::solve (const F& b, T& x) const
{
	do_solve(b,x);
}

template <typename F, typename T>
float TRegModel<F,T>::get_force_scale() const
{
	return do_get_force_scale();
}

template <typename F, typename T>
float  TRegModel<F,T>::do_get_force_scale() const
{
	return 1.0f;
}

NS_MIA_END

#endif
