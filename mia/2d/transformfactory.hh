/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

#ifndef mia_2d_transformfactory_hh
#define mia_2d_transformfactory_hh

#include <set>
#include <mia/2d/transform.hh>
#include <mia/core/factory.hh>

NS_MIA_BEGIN

/**
   This class is the base class for a Creater of transformations according to a 
   given model. 
   Derived from this class are all the plug-ins that may create transformations 
   of different types. 
 */
class  EXPORT_2D C2DTransformCreator: public CProductBase {
public:
	/** Standard constructor 
	    \remark it is empty, so why it is defined? 
	 */
	C2DTransformCreator();

	/**
	   Creates a transformation according to the given model and defined 
	   on a grid [(0,0), size}
	 */

	P2DTransformation create(const C2DBounds& size) const;
	
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
	virtual P2DTransformation do_create(const C2DBounds& size) const = 0;

	std::set<std::string> _M_properties;
};

typedef SHARED_PTR(C2DTransformCreator) P2DTransformationFactory;
typedef TFactory<C2DTransformCreator, C2DImage, C2DTransformation> C2DTransformCreatorPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<C2DTransformCreatorPlugin> > C2DTransformCreatorHandler;
FACTORY_TRAIT(C2DTransformCreatorHandler); 

NS_MIA_END


#endif
