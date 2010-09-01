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

#ifndef mia_core_creator_hh
#define mia_core_creator_hh

#include <mia/core/shared_ptr.hh>
#include <mia/core/pixeltype.hh>
#include <mia/core/factory.hh>
#include <mia/core/handler.hh>


NS_MIA_BEGIN

struct EXPORT_CORE creator_type {
	static const char *value;
};

/**
   This template defines a generic base class for a creator of images containing some shapes. 
   \tparam Image type to create 
 */
template <typename T>
class EXPORT_HANDLER TImageCreator:public CProductBase {
public:
	virtual ~TImageCreator(){}

	/**
	   Creator function, need to be overloaded  
	   \param size output image size 
	   \param type pixel type 
	   \returns the created Image 
	 */
	virtual std::shared_ptr<T> operator () (const typename T::dimsize_type& size, EPixelType type) const = 0;
};


#define DEFINE_Image_Creator(TYPE) \
	typedef TImageCreator<TYPE> TYPE##Creator; \
	typedef TFactory<TImageCreator<TYPE>, TYPE, creator_type> TYPE##CreatorPlugin; \
	typedef THandlerSingleton<TFactoryPluginHandler<TYPE##CreatorPlugin> > TYPE##CreatorPluginHandler;\
	FACTORY_TRAIT(THandlerSingleton<TFactoryPluginHandler<TYPE##CreatorPlugin> >); 


#define INSTANCIATE_Image_Creator(TYPE) \
	template class  TImageCreator<TYPE>;\
	template class  TPlugin<TYPE, creator_type>;			\
	template class  TFactory<TImageCreator<TYPE>, TYPE, creator_type>; \
	template class  TPluginHandler<TYPE##CreatorPlugin>;	\
        template class  TFactoryPluginHandler<TYPE##CreatorPlugin>;	\
	template class  THandlerSingleton<TFactoryPluginHandler<TYPE##CreatorPlugin> >; 


NS_MIA_END

#endif
