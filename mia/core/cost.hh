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

#ifndef mia_core_cost_hh
#define mia_core_cost_hh

#include <mia/core/factory.hh>


#ifndef EXPORT_HANDLER
#  ifdef WIN32
#     define EXPORT_HANDLER __declspec(dllimport)
#  else
#     define EXPORT_HANDLER
#  endif
#endif

NS_MIA_BEGIN

/// plugin helper type
struct EXPORT_CORE cost_type {
	static const char *value;
};


/**
   The basic cost function type interface. Template parameter T describes the input data class, e.g. T3DImage, parameter
   V is for the force field description.
   The class defines an abstract interface and the functions \a do_value and \a do_evaluate_force have to be implemented by
   derived functions.
 */

template <typename T, typename V>
class TCost : public CProductBase{
public:
	typedef T Data;
	typedef V Force;

	typedef T plugin_data; 
	typedef cost_type plugin_type; 

	/// ensure virtual destruction, since we have virtual functions
	virtual ~TCost();

	/** The cost value evaluation function
	    \param a
	    \param b
	    \returns the cost value describing the distance between the entities \a a and \a b.
	 */
	double value(const T& a, const T& b) const;

	/** The force evaluation function
	    \param a input entity
	    \param b input entity
	    \param scale a force scaling parameter
	    \retval force The external force of \a a with respect to \a b that lead to cost minimisation
	 */
	double evaluate_force(const T& a, const T& b, float scale, V& force) const;

	virtual void prepare_reference(const T& ref); 
private:
	virtual double do_value(const T& a, const T& b) const = 0;
	virtual double do_evaluate_force(const T& a, const T& b, float scale, V& force) const = 0;
};


NS_MIA_END
#endif
