/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef mia_core_factory_trait_hh
#define mia_core_factory_trait_hh

#include <mia/core/defines.hh>
#include <memory>

NS_MIA_BEGIN
/**
   \ingroup traits
   \brief Type trait to enable the use of a factory product as command
      line option

   Type trait that enables the use of the factory creation in commen line parsing.
   This trait needs to be specialized for all factories that are to be used
   utilizing the TCmdFactoryOption interface.
   \tparam T a class that can be created by a TFactory through the call to
    the method TFactoryPluginHandler::produce(const char *plugindescr) of the
    corresponding factory plugin handler.
    \sa TCmdFactoryOption
 */
template <class T>
class FactoryTrait
{
       /// the typetrait type if not defined properly
       typedef typename T::must_create_trait_using_FACTORY_TRAIT type;
};

/**
   \ingroup traits

   \brief Type trait to enable the use of a factory product as command
      line option

   This trait specializes FactoryTrait for shared pointers.
 */
template <class T>
class FactoryTrait<std::shared_ptr<T>>
{
public:
       /// the typetrait type
       typedef typename FactoryTrait<T>::type type;
};

/**
   \ingroup traits

   Specialize the FactoryTrait template for the given TFactoryPluginHandler
*/
#define FACTORY_TRAIT(F)			\
	template <>				\
	class FactoryTrait< F::Instance::ProductPtr::element_type >  {	\
	public:					\
	typedef F type;		\
	};


NS_MIA_END

#endif
