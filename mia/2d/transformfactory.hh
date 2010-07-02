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


class  EXPORT_2D C2DTransformCreator: public CProductBase {
public:
	C2DTransformCreator();


	P2DTransformation create(const C2DBounds& size) const;

	bool has_property(const char *property) const;
protected:
	void add_property(const char *property);
private:
	virtual P2DTransformation do_create(const C2DBounds& size) const = 0;

	std::set<std::string> _M_properties;
};

typedef SHARED_PTR(C2DTransformCreator) P2DTransformationFactory;

typedef TFactory<C2DTransformCreator, C2DImage, C2DTransformation> C2DTransformCreatorPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<C2DTransformCreatorPlugin> > C2DTransformCreatorHandler;

NS_MIA_END


#endif
