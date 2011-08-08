/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef mia_core_fatcost_hh
#define mia_core_fatcost_hh

#include <mia/core/defines.hh>
#include <mia/core/product_base.hh>

#ifndef EXPORT_HANDLER
#  ifdef WIN32
#     define EXPORT_HANDLER __declspec(dllimport)
#  else
#     define EXPORT_HANDLER
#  endif
#endif

NS_MIA_BEGIN

/// plugin helper type
struct EXPORT_CORE fatcost_type {
	/// actual value of the helper type
	static const char *type_descr;
};


/**
   \deprecated move to TFullCost
   The basic cost function type interface of a "fat" cost that handles the data itself.
   Template parameter D describes the input data class, e.g. T3DImage, parameter
   T is for the Transformation and F for the force field.
   The class defines an abstract interface and the functions \a do_value and \a do_evaluate_force have to be implemented by
   derived functions.
 */

template <typename T, typename F>
class EXPORT_HANDLER TFatCost: public CProductBase {
public:
	/// \cond DEPRECATED 
	typedef T Transform;
	typedef F Force;
	typedef typename T::Data Data;
	
	typedef typename T::Data plugin_data;
	typedef fatcost_type plugin_type;


	/**

	 */

	TFatCost(std::shared_ptr<Data > src, std::shared_ptr<Data > ref, float weight = 1.0);

	/// ensure virtual destruction, since we have virtual functions
	virtual ~TFatCost();

	/** The cost value evaluation function
	    \returns the cost value describing the distance between the current floating data and the reference
	*/
	double value() const;

	/** The force evaluation function
	    \param force the force field in which to accumulate the force
	    \param[out] force added the weighted accumutaed force
	*/
	double evaluate_force(Force& force) const;

	/**
	   Transform the containing data
	   \param transform the transfomation to be applied
	*/
	void transform(const T& transform);

	const Data& get_src() const {
		return *m_src;
	}
	const Data& get_ref() const{
		return *m_ref;
	}
	const Data& get_floating() const {
		return *m_floating;
	}

	float get_weight() const {
		return m_weight;
	}
private:
	virtual double do_value() const = 0;
	virtual double do_evaluate_force(Force& force) const = 0;

	const std::shared_ptr<Data >  m_src;
	const std::shared_ptr<Data >  m_ref;
	std::shared_ptr<Data > m_floating;
	float m_weight;
	
	mutable double m_cost;
	mutable bool m_cost_valid;

	/// \endcond
};

NS_MIA_END
#endif
