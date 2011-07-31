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

#ifndef mia_core_regmodel_hh
#define mia_core_regmodel_hh

#include <mia/core/factory.hh>

NS_MIA_BEGIN

struct EXPORT_CORE regmodel_type {
	static const char *type_descr;
};

template <int Dim> 
struct RegistrationTraits {
	typedef void need_to_declare_RegistrationTraits_for_DIM; 
	typedef need_to_declare_RegistrationTraits_for_DIM Data;
	typedef need_to_declare_RegistrationTraits_for_DIM Force; 
	typedef need_to_declare_RegistrationTraits_for_DIM Transformation; 
}; 

/**
   \ingroup templates 
   \brief Base class for PDE model based registrations
   
   Base class template for PDE based image registration models.
   The template parameters are 
   \tparam Dim dimension of registration model 
 */
template <int Dim>
class TRegModel :public CProductBase {
public:
	/// Imaga data type for the registration model 
	typedef typename RegistrationTraits<Dim>::Data Data; 

	/// gradient force data type for the registration model 
	typedef typename RegistrationTraits<Dim>::Force Force; 

	/// transformation type for the registrationmodel 
	typedef typename RegistrationTraits<Dim>::Transformation Transformation; 

	/// plug-in helper type for search path evaluation 
	typedef regmodel_type plugin_type; 
	
	/// plug-in helper type for search path evaluation 
	typedef Data plugin_data; 
	
	TRegModel();
	virtual ~TRegModel();

	/**
	   PDE solver for the registration model 
	   \param[in] b force the gradient force of the cost function 
	   \param[in,out] x estimated transformation direction that can be further optimized 
	   by a line search 
	   \remark the naming needs to be reviewed,  
	 */
	void solve (const Force& b, Transformation& x) const;

	/**
	   \returns the scaling applied to the gradient force 
	   \remark why is this needed? 
	 */
	float get_force_scale() const;
private:
	virtual void do_solve(const Force& b, Transformation& x) const = 0;
	virtual float do_get_force_scale() const;
};

//
// implementation part of the registration model
//

template <int Dim>
TRegModel<Dim>::TRegModel()
{
}

template <int Dim>
TRegModel<Dim>::~TRegModel()
{
}

template <int Dim>
void TRegModel<Dim>::solve (const Force& b, Transformation& x) const
{
	do_solve(b,x);
}

template <int Dim>
float TRegModel<Dim>::get_force_scale() const
{
	return do_get_force_scale();
}

template <int Dim>
float  TRegModel<Dim>::do_get_force_scale() const
{
	return 1.0f;
}

NS_MIA_END

#endif
