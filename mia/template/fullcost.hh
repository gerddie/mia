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

#ifndef mia_internal_fullcost_hh
#define mia_internal_fullcost_hh

#include <mia/core/product_base.hh>
#include <mia/core/vector.hh>
#include <mia/core/import_handler.hh>

NS_MIA_BEGIN

/**
   \ingroup registration

   \tparam Transform the transformation type used to achieve registration by optimizing the cost function 

   \brief Base class for a general cost function. 

   This base class for a cost function does not make any assumptions about what kind of cost is measured. 
   It only considers a transformation to be applied to input data or to measure a transformation penalty. 
   
 */

template <typename Transform>
class EXPORT_HANDLER TFullCost : public CProductBase {
public: 

	/// Typedef of the size of the data considered by this cost function 
	typedef typename Transform::Size Size; 

	/// helper type for plug-in handling 
	typedef TFullCost<Transform> plugin_data; 
	
	/// helper type for plug-in handling 
	typedef TFullCost<Transform> plugin_type;

        /// helper string for plug-in handling 
	static const char *type_descr;
	
        /// helper string for plug-in handling 
	static const char *data_descr;

	/// The shatred pointer type for this cost function 
	typedef std::shared_ptr<TFullCost<Transform> > Pointer; 
	
	/**
	   Initialize the cost function with a weight 
	   \param weight 
	 */
	TFullCost(double weight);


	/**
	   Evaluate the weighted cost value and the weighted gradient in optimizer space 
	   given a current  transformation. 
	   \param t tranformation to be applied 
	   \param[out] gradient gradient in optimizer space 
	   \returns weighted cost value 
	 */
	double evaluate(const Transform& t, CDoubleVector& gradient) const;

	/**
	   Evaluate the weighted cost value 
	   \param t tranformation to be applied 
	   \returns weighted cost value 
	 */
	double cost_value(const Transform& t) const;

	/**
	   Evaluate the weighted cost value without transforming the image 
	   \returns weighted cost value 
	 */
	double cost_value() const;

	/**
	   Re-initialalize the cost function
	*/
	void reinit();
	
	/**
	   Set the size of the cost function 
	 */
	void set_size(const Size& size); 
	
	/**
	   Get the full size of the registration problem and see if everybody agrees on it. 
	   \param size if it is at Size() at input  input it will simply be overwritten, if 
	           it is not equal to Size(), it will be checked that the size is equal to the local one 
	   \returns true if the cost function has setthe size or has the same size as the nonzero one give  at input
	 */
	bool get_full_size(Size& size) const; 
	
protected: 
	/** \returns cost function weight  */
	double get_weight() const; 
	
	/** \returns the current size of the data in the cost function */ 
	const Size& get_current_size() const; 
private:
	virtual double do_evaluate(const Transform& t, CDoubleVector& gradient) const = 0;
	virtual double do_value(const Transform& t) const = 0;
	virtual double do_value() const = 0;
	virtual void do_reinit();
	virtual void do_set_size() = 0; 
	virtual bool do_get_full_size(Size& size) const; 
	
	double m_weight;
	Size m_current_size; 

}; 

/**
   \ingroup registration
   \tparam Transform the transformation type used to achieve registration by optimizing the cost function 
   \brief the base class for the TFullCost cost function plug-ins. 
*/ 

template <typename Transform>
class EXPORT_HANDLER TFullCostPlugin: public TFactory<TFullCost<Transform> > {
public:
	/**
	   Constructor for plug-in 
	   \param name the name of the plug-in 
	 */
	TFullCostPlugin(const char *name);
private:
	virtual TFullCost<Transform> *do_create() const;
	virtual TFullCost<Transform> *do_create(float weight) const = 0;
	float m_weight;
}; 


NS_MIA_END

#endif
