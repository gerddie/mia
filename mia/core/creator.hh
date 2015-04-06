/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_core_creator_hh
#define mia_core_creator_hh

#include <mia/core/pixeltype.hh>
#include <mia/core/factory.hh>
#include <mia/core/handler.hh>


NS_MIA_BEGIN

struct EXPORT_CORE creator_type {
	static const char *type_descr;
};

/**
   \ingroup filtering 

   \brief The base class for generic image creation 

   This template defines a generic base class for a creator of images containing some shapes. 
   \tparam T image type to create 
 */
template <typename T>
class EXPORT_HANDLER TImageCreator:public CProductBase {
public:
	/// plugin searchpath helper type, data part  
	typedef T plugin_data; 
	
	/// plugin searchpath helper type, plugin type part  
	typedef creator_type plugin_type; 
	
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
	typedef TFactory<TImageCreator<TYPE> > TYPE##CreatorPlugin; \
	typedef THandlerSingleton<TFactoryPluginHandler<TYPE##CreatorPlugin> > TYPE##CreatorPluginHandler;\
	FACTORY_TRAIT(THandlerSingleton<TFactoryPluginHandler<TYPE##CreatorPlugin> >); 


#define INSTANCIATE_Image_Creator(TYPE) \
	template class  TImageCreator<TYPE>;\
	template class  TPlugin<TYPE, creator_type>;			\
	template class  TFactory<TImageCreator<TYPE> >;			\
	template class  TPluginHandler<TYPE##CreatorPlugin>;	\
        template class  TFactoryPluginHandler<TYPE##CreatorPlugin>;	\
	template class  THandlerSingleton<TFactoryPluginHandler<TYPE##CreatorPlugin> >; 


NS_MIA_END

#endif
