/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_core_regmodel_hh
#define mia_core_regmodel_hh

#include <mia/core/factory.hh>

NS_MIA_BEGIN

struct EXPORT_CORE regmodel_type {
	static const char *type_descr;
};

/**
   @cond INTERNAL 
   \ingroup traits 
   \brief a trait to provide some information about the registration objects 
*/
template <int Dim> 
struct RegistrationTraits {
	typedef void need_to_declare_RegistrationTraits_for_DIM; 
	typedef need_to_declare_RegistrationTraits_for_DIM Data;
	typedef need_to_declare_RegistrationTraits_for_DIM Force; 
	typedef need_to_declare_RegistrationTraits_for_DIM Transformation; 
}; 
/// @endcond 

/**
   \ingroup registration 
   \brief Base class for PDE model based registrations
   
   Generic base class template for PDE based image registration models.
   
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
