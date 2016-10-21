/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_template_masked_cost_hh
#define mia_template_masked_cost_hh

#include <mia/core/factory.hh>
#include <mia/core/refholder.hh>


#ifndef EXPORT_HANDLER
#  ifdef WIN32
#     define EXPORT_HANDLER __declspec(dllimport)
#  else
#     define EXPORT_HANDLER
#  endif
#endif

NS_MIA_BEGIN

/// plugin helper type
struct EXPORT_CORE masked_cost_type {
	/// plugin path helper variable 
	static const char *type_descr;
};


/**
   \ingroup registration 

   \brief The generic cost function interface. 

   The class defines an abstract interface for a cost function between two entities of the same 
   type. 
   The pure virtual functions 
    -double do_value(const T& a, const T& b) const, and 
    -double do_evaluate_force(const T& a, const T& b, float scale, V& force) const
    have to be implemented in the derived class to make it a real cost function. 
    The virtual function 
    - void post_set_reference(const T& ref)
    may be overwritten in order to prepare the reference data for the implemented cost function.
    \tparam T the data type of the objects that the cost evaluation is based on 
    \tparam V the type of the gradient force field created by this cost function 
*/

template <typename T, typename M, typename V>
class EXPORT_HANDLER TMaskedCost : public CProductBase{
public:
	/// typedef for generic programming: The data type used by the cost function 
	typedef T Data;

	/// typedef for generic programming: The data type used by the cost function 
	typedef M Mask;
	
	/// typedef for generic programming: The gradient forca type create by the cost function 
	typedef V Force;

	/// Const reference holder TRefHolder of the cost function data 
	typedef TRefHolder<T> RData; 
	
	/// Pointer to const reference holder TRefHolder of the cost function data 
	typedef typename RData::Pointer PData;

	/// plugin searchpath helper type 
	typedef T plugin_data; 
	
	/// plugin searchpath helper type 
	typedef masked_cost_type plugin_type; 

	/// ensure virtual destruction, since we have virtual functions
	virtual ~TMaskedCost();

	/**
	   Evaluate the value of the cost function petreen the given moving image and 
	   the reference that was set by calling set_reference(const T& ref). 
	   \param mov the moving image 
	   \param mask the mask to be applied when evaluating the cost function 
	   \returns the cost function value 
	 */
	double value(const T& mov, const M& mask) const;

	/**
	   Evaluate the value of the cost function and its gradient with respect 
	   to the given moving image and  the reference that was set by 
           calling set_reference(const T& ref). 
	   \param mov the moving image 
	   \param mask the mask to be applied when evaluating the cost function 
	   \param[out] force gradient force 
	   \returns the cost function value 
	 */
	double evaluate_force(const T& mov, const M& mask, V& force) const;
	
	/**
	   Set the new reference of the cost function. The virtual private function  
	   post_set_reference(const T& ref) is then called to run possible preparations 
	   on the reference image. 
	 */
	void set_reference(const T& ref);
private:
	virtual double do_value(const T& a, const T& b, const M& mask) const = 0;
	virtual double do_evaluate_force(const T& a, const T& b, const M& mask, V& force) const = 0;
	virtual void post_set_reference(const T& ref); 
	
	PData m_reference; 
};


NS_MIA_END
#endif
