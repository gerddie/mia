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

#ifndef mia_core_cost_hh
#define mia_core_cost_hh

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
struct EXPORT_CORE cost_type {
	/// plugin path helper variable 
	static const char *type_descr;
};


/**
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

template <typename T, typename V>
class TCost : public CProductBase{
public:
	/// typedef for generic programming: The data type used by the cost function 
	typedef T Data;
	
	/// typedef for generic programming: The gradient forca type create by the cost function 
	typedef V Force;

	/// Const reference holder TRefHolder of the cost function data 
	typedef TRefHolder<T> RData; 
	
	/// Pointer to const reference holder TRefHolder of the cost function data 
	typedef typename RData::Pointer PData;

	/// plugin searchpath helper type 
	typedef T plugin_data; 
	
	/// plugin searchpath helper type 
	typedef cost_type plugin_type; 

	/// ensure virtual destruction, since we have virtual functions
	virtual ~TCost();

	/** \deprecated The cost value evaluation function, call TCost::value(const T& a) instead 
	    after setting the reference image
	    \param src
	    \param ref
	    \returns the cost value describing the distance between the entities \a a and \a b.
	 */
	double value(const T& src, const T& ref) const __attribute__((deprecated));

	/** \deprecated The force evaluation function, 
   	      TCost::use evaluate_force(const T& a, float scale, V& force) instead 
	    \param src input entity
	    \param ref input entity
	    \param scale a force scaling parameter
	    \param[out] force The external force of \a a with respect to \a b that lead to cost minimisation
	 */
	double evaluate_force(const T& src, const T&ref, float scale, V& force) const  __attribute__((deprecated));

	/**
	   \deprecated use TCost::set_reference instead 
	   prepare the reference for this cost function 
	   \param ref 
	 */
	virtual void prepare_reference(const T& ref)  __attribute__((deprecated)); 

	/**
	   Evaluate the value of the cost function petreen the given src image and 
	   the reference that was set by calling set_reference(const T& ref). 
	   \param src 
	   \returns the cost function value 
	 */
	double value(const T& src) const;

	/**
	   Evaluate the value of the cost function and its gradient with respect 
	   to the given src image and  the reference that was set by 
           calling set_reference(const T& ref). 
	   \param src 
	   \param scale scaling of the force vectors 
	   \param[out] force gradient force 
	   \returns the cost function value 
	 */
	double evaluate_force(const T& src, float scale, V& force) const;
	
	/**
	   Set the new reference of the cost function. The virtual private function  
	   post_set_reference(const T& ref) is then called to run possible preparations 
	   on the reference image. 
	 */
	void set_reference(const T& ref);
private:
	virtual double do_value(const T& a, const T& b) const = 0;
	virtual double do_evaluate_force(const T& a, const T& b, float scale, V& force) const = 0;
	virtual void post_set_reference(const T& ref); 
	
	PData m_reference; 
};


NS_MIA_END
#endif
