/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#ifndef mia_internal_transformfactory_hh
#define mia_internal_transformfactory_hh

#include <set>
#include <mia/core/transformation.hh>
#include <mia/core/factory.hh>
#include <mia/core/splinekernel.hh>
#include <mia/core/boundary_conditions.hh>
#include <mia/core/import_handler.hh>
#include <mia/core/parameter.hh>

NS_MIA_BEGIN

/**
   @ingroup registration 
   
   This class is the base class for a Creater of transformations according to a 
   given model. 
   Derived from this class are all the plug-ins that may create transformations 
   of different types. 
   @tparam Transform the transformation type to be created by this creator   
 */
template <typename Transform> 
class EXPORT_HANDLER TTransformCreator: public CProductBase {
public:

	/// helper type for plug-in handling 
	typedef typename Transform::Data plugin_data; 

	/// helper type for plug-in handling 
	typedef Transform plugin_type; 

	/// the type of the interpolation factory used by this transformation 
	typedef typename Transform::InterpolatorFactory InterpolatorFactory; 

	/// the size type of this transformation 
	typedef typename Transform::Size Size; 

	/// the pointer type for this transformation creator 
	typedef std::shared_ptr<TTransformCreator<Transform> > Pointer; 

	/** Copy constructor 
	 */
	TTransformCreator(const InterpolatorFactory& ipf);

	/**
	   Creates a transformation according to the given model and defined 
	   on a grid [(0,0), size}
	 */
	typename Transform::Pointer create(const Size& size) const;
	
	/**
	   This function checks for a given property of the transformation creator. 
	   \param property 
	   \returns \a true if property is supported
	 */
	bool has_property(const char *property) const;
protected:
	/**
	   Add a property 
	   \param property 
	 */
	void add_property(const char *property);
private:
	virtual	typename Transform::Pointer do_create(const Size& size, const InterpolatorFactory& ipf) const = 0;

	std::set<std::string> m_properties;
	InterpolatorFactory m_ipf; 
};


/**
   @ingroup registration 
   \brief Factory class that is used to create the transformation creator 
   \tparam Transform the type of the transformation to be created 
*/ 
template <typename Transform> 
class EXPORT_HANDLER TTransformCreatorPlugin : public TFactory<TTransformCreator<Transform> > {
public: 
	typedef typename TFactory<TTransformCreator<Transform> >::Product Product; 

	/// the type of the interpolation factory used by the transformation 
	typedef typename Transform::InterpolatorFactory InterpolatorFactory; 

	/**
	   Plug-in constructor 
	   \param name name of the plug-in 
	 */
	TTransformCreatorPlugin(const char *const name); 
private: 
	virtual Product *do_create() const __attribute__((warn_unused_result));
	virtual Product *do_create(const InterpolatorFactory& factory) const __attribute__((warn_unused_result)) = 0 ;

	PSplineKernel m_image_interpolator; 
	PSplineBoundaryCondition m_image_boundary; 
}; 


NS_MIA_END

#endif
