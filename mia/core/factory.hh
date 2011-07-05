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

// $Id: factory.hh,v 1.4 2005/06/03 11:00:38 gerddie Exp $

/** 
    \file; core/factory.hh
    \author: Gert Wollny < wollny at cbs mpg de >
    Defines and implements tha infrastructor for factories and 
    plugins that provide factories. 
*/

#ifndef FACTORY_HH
#define FACTORY_HH

#include <iostream>
#include <memory>
#include <string>
#include <mia/core/handler.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/plugin_base.hh>
#include <mia/core/product_base.hh>
#include <mia/core/optionparser.hh>

#include <mia/core/import_handler.hh>

NS_MIA_BEGIN


/** 
   \ingroup infrastructure 

    @brief This is tha base of all plugins that create "things", like filters, cost functions 
    time step operatores and the like. 
    
    This template is the model for all factory plugins, i.e. plugins that create certain objects.
   \tparam P the object type created by the factory.
*/
template <typename P>
class EXPORT_HANDLER TFactory: 
	public TPlugin<typename P::plugin_data, typename P::plugin_type> {
public: 

	/// typedef to give the output type a nice name 
	typedef P Product; 
	
	/// typedef to give the output pointer type a nice name 
	typedef std::shared_ptr<P > ProductPtr; 
	
	/** initialise the plugin by the names 
	    \remark what are these names and types good for?
	*/
	TFactory(char const * const  name);
	
	/** This function creates the object handled by this plugin 
	    It uses options to set its parameters and, if successfull, 
	    sets the init string of the object to params and 
	    returns the newly created object as a shared pointer. 
	    
	    @param options the options to initialise the plugin 
	    @param params original parameter string 
	    @returns an instance of the requested object
	*/
	virtual ProductPtr create(const CParsedOptions& options, char const *params);
	
private:
	virtual bool do_test() const; 
	virtual ProductPtr do_create() const = 0;
	CMutex m_mutex; 
};


/**
   \ingroup infrastructure 

   @brief the Base class for all plugn handlers that deal with factory plugins.  
   
   Base class for all plugin handlers that are derived from TFactory. 
 */
template <typename  I>
class EXPORT_HANDLER TFactoryPluginHandler: public  TPluginHandler< I > {
protected: 
	//! \name Constructors
        //@{
        /*! \brief Initializes the plugin handler based on a given plugin search path list 
	  \param searchpath list of directories to search for plugins 
	*/

	
	TFactoryPluginHandler(const std::list<boost::filesystem::path>& searchpath); 
        //@}
public: 
	/// The pointer type of the the object this plug in hander produces 
	typedef typename I::ProductPtr ProductPtr; 

	/**
	   Create an object according to the given description. If creation fails, an empty 
	   pointer is returned. if plugindescr is set to "help" then print out some help.  
	   
	 */
	ProductPtr produce(const char *plugindescr) const;

	/// \overload produce(const char *plugindescr)
	ProductPtr produce(const std::string& params)const {
		return produce(params.c_str()); 
	}

}; 

/**
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
class FactoryTrait {
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
class FactoryTrait<std::shared_ptr<T> >  {
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



/*
  Implementation of the factory
*/

template <typename I>
TFactory<I>::TFactory(char const * const  name):
	TPlugin<typename I::plugin_data, typename I::plugin_type>(name)
{
}

template <typename I>
typename TFactory<I>::ProductPtr TFactory<I>::create(const CParsedOptions& options, char const *params)
{
	CScopedLock lock(m_mutex); 
	try {
		this->set_parameters(options);
		this->check_parameters();
		ProductPtr product = this->do_create();
		if (product) {
			product->set_module(this->get_module()); 
			product->set_init_string(params); 
		}
		return product; 
	}
	catch (std::length_error& x) {
		std::stringstream msg; 
		msg << "CParamList::set: Some string was not created properly\n"; 
		msg << "  options were:\n"; 
		for (auto i = options.begin();
		     i != options.end(); ++i) {
			msg << "    " << i->first << "=" << i->second << "\n";
		}
		cverr() << msg.str(); 
		throw logic_error("Probably a race condition"); 

	}
}

template <typename  I>
TFactoryPluginHandler<I>::TFactoryPluginHandler(const std::list<boost::filesystem::path>& searchpath):
	TPluginHandler< I >(searchpath)
{
}

	
template <typename  I>
typename TFactoryPluginHandler<I>::ProductPtr 
TFactoryPluginHandler<I>::produce(char const *params)const
{
	assert(params); 
	CComplexOptionParser param_list(params);
		
	if (param_list.size() < 1) 
		return ProductPtr(); 
		
	cvdebug() << "TFactoryPluginHandler<P>::produce use '" << param_list.begin()->first << "'\n"; 
	const std::string& factory_name = param_list.begin()->first; 
	
	if (factory_name == plugin_help) {
		cvdebug() << "print help\n"; 
		cvmsg() << "\n"; 
		this->print_help(cverb);
		return ProductPtr(); 
	}

	cvdebug() << "TFactoryPluginHandler<>::produce: Create plugin from '" << factory_name << "'\n"; 

	P *factory = this->plugin(factory_name.c_str());
	if (factory) 
		return factory->create(param_list.begin()->second,params);
	else 
		return ProductPtr(); 
}

template <typename I>
bool TFactory<I>::do_test() const
{
	cvfail() << "do_test() is obsolete\n"; 
	return false; 
}

/// Do some explicit instanciation for a plugin based on TFactory 
#define EXPLICIT_INSTANCE_PLUGIN(T) \
	template class TPlugin<T::plugin_data, T::plugin_type>; \
	template class TFactory<T>;					

/// Do some explicit instanciation for a plugin based on TFactoryPluginHandler
#define EXPLICIT_INSTANCE_PLUGIN_HANDLER(P) \
	template class TPluginHandler<P>;			\
	template class TFactoryPluginHandler<P>;		\
	template class THandlerSingleton<TFactoryPluginHandler<P> >;

/** Do some explicit instanciation for the plugin classe and the handler of 
    a plugin based on TFactoryPluginHandler */
#define EXPLICIT_INSTANCE_HANDLER(T) \
	template class TPlugin<T::plugin_data, T::plugin_type>; \
	template class TFactory<T>;					\
	template class TPluginHandler<TFactory<T> >;			\
	template class TFactoryPluginHandler<TFactory<T> >;		\
	template class THandlerSingleton<TFactoryPluginHandler<TFactory<T> > >;



NS_MIA_END
#endif
