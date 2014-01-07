/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_core_factory_hh
#define mia_core_factory_hh

#include <iostream>
#include <memory>
#include <string>
#include <mia/core/handler.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/product_base.hh>
#include <mia/core/optionparser.hh>
#include <mia/core/productcache.hh>
#include <mia/core/traits.hh>
#include <mia/core/import_handler.hh>

NS_MIA_BEGIN


/** 
    \ingroup plugin

    @brief This is tha base of all plugins that create "things", like filters, cost functions 
    time step operatores and the like. 
    
    This template is the model for all factory plugins, i.e. plugins that create certain objects.
   \tparam P the object type created by the factory.
*/
template <typename P>
class EXPORT_HANDLER TFactory: 
	public TPlugin<typename P::plugin_data, typename P::plugin_type> {
public: 

	/// typedef to describe the product of the factory 
	typedef P Product; 
	
	/// typedef for the shared version of the product 
	typedef std::shared_ptr<P > SharedProduct; 


	/// typedef for the unique version of the product 
	typedef std::unique_ptr<P > UniqueProduct; 
	
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
	virtual Product *create(const CParsedOptions& options, char const *params) __attribute__((warn_unused_result));
	
private:
	virtual Product *do_create() const __attribute__((warn_unused_result)) = 0 ;
	CMutex m_mutex; 
};


/**
    \ingroup plugin

   @brief the Base class for all plugn handlers that deal with factory plugins.  
   
   Base class for all plugin handlers that are derived from TFactory. 
 */
template <typename  I>
class EXPORT_HANDLER TFactoryPluginHandler: public  TPluginHandler< I > {
protected: 
	//! \name Constructors
        //@{
        /*! \brief Initializes the plugin handler 
	*/

	
	TFactoryPluginHandler(); 
        //@}
public: 
	/// The type of the the object this plug in hander produces 
	typedef typename I::Product Product; 

	/// The shared pointer type of the the object this plug in hander produces 
	typedef typename I::SharedProduct ProductPtr; 

	/// The unique pointer type of the the object this plug in hander produces 
	typedef typename I::UniqueProduct UniqueProduct; 

        /**
	   Create an object according to the given description. If creation fails, the function 
	   will throw an invalid_argument exception 
	   \param plugindescr the description of the plug-in 
	   \returns a shared pointer containing the product of the plug-in according to the description 
	*/
	ProductPtr produce(const std::string& plugindescr)const;  
	
	/// \overload produce(const std::string& plugindescr)
	ProductPtr produce(const char *plugindescr) const{
		return produce(std::string(plugindescr)); 
	}

	/**
	   Create an object according to the given description. If creation fails, the function 
	   will throw an invalid_argument exception 
	   \param plugindescr the description of the plug-in 
	   \returns a unique pointer of the product of the plug-in according to the description 
	*/

	UniqueProduct produce_unique(const std::string& plugindescr)const; 
		
	/// \overload produce(const std::string& plugindescr)
	UniqueProduct produce_unique(const char *plugindescr) const{
		return produce_unique(std::string(plugindescr)); 
	}

	/**
	   Sets whether the created shared pointer products should be cached. 
	   Unique products will never be cached. 
	   \param enable 
	 */
	void set_caching(bool enable) const; 


private: 
	std::string get_handler_type_string_and_help(std::ostream& os) const; 
	
	std::string do_get_handler_type_string() const; 
	
	typename I::Product *produce_raw(const std::string& plugindescr) const;

	mutable TProductCache<ProductPtr> m_cache; 

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
typename TFactory<I>::Product *TFactory<I>::create(const CParsedOptions& options, char const *params)
{
	CScopedLock lock(m_mutex); 
	try {
		this->set_parameters(options);
		this->check_parameters();
		auto product = this->do_create();
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
		throw std::logic_error("Probably a race condition"); 

	}
}

template <typename  I>
TFactoryPluginHandler<I>::TFactoryPluginHandler():
	m_cache(this->get_descriptor())
{
	set_caching(__cache_policy<I>::apply()); 
}

template <typename  I>
void TFactoryPluginHandler<I>::set_caching(bool enable) const 
{
	cvdebug() << this->get_descriptor() << ":Set cache policy to " << enable << "\n"; 
	m_cache.enable_write(enable); 
}

template <typename  I>
typename TFactoryPluginHandler<I>::ProductPtr 
TFactoryPluginHandler<I>::produce(const std::string& plugindescr) const
{
	auto result = m_cache.get(plugindescr);
	if (!result) {
		result.reset(this->produce_raw(plugindescr)); 
		m_cache.add(plugindescr, result); 
	}else
		cvdebug() << "Use cached '" << plugindescr << "'\n"; 
	return result; 
}

template <typename  I>
typename TFactoryPluginHandler<I>::UniqueProduct
TFactoryPluginHandler<I>::produce_unique(const std::string& plugindescr) const
{
	return UniqueProduct(this->produce_raw(plugindescr)); 
}

template <typename  I>
std::string TFactoryPluginHandler<I>::get_handler_type_string_and_help(std::ostream& os) const
{
	os << " The string value will be used to construct a plug-in."; 
	return do_get_handler_type_string(); 
}

template <typename  I>
std::string TFactoryPluginHandler<I>::do_get_handler_type_string() const
{
	return "factory"; 
}
	
template <typename  I>
typename I::Product *TFactoryPluginHandler<I>::produce_raw(const std::string& params)const
{
	if (params.empty()) {
		throw create_exception<std::invalid_argument>("Factory ", this->get_descriptor(), ": Empty description string given. "
						    "Supported plug-ins are '", this->get_plugin_names(), "'. " 
						    "Set description to 'help' for more information."); 
	}
	
	CComplexOptionParser param_list(params);
		
	if (param_list.size() < 1) {
		throw create_exception<std::invalid_argument>( "Factory " , this->get_descriptor(), ": Description string '"
		      , params , "' can not be interpreted. "
		      "Supported plug-ins are '" , this->get_plugin_names() , "'. " 
		      "Set description to 'help' for more information."); 
	}
		
	cvdebug() << "TFactoryPluginHandler<P>::produce use '" << param_list.begin()->first << "'\n"; 
	const std::string& factory_name = param_list.begin()->first; 
	
	if (factory_name == plugin_help) {
		cvdebug() << "print help\n"; 
		cvmsg() << "\n"; 
		this->print_help(cverb);
		return NULL; 
	}

	cvdebug() << "TFactoryPluginHandler<>::produce: Create plugin from '" << factory_name << "'\n"; 

	auto factory = this->plugin(factory_name.c_str());
	if (!factory) 
		throw create_exception<std::invalid_argument>("Unable to find plugin for '", factory_name.c_str(), "'");
	return factory->create(param_list.begin()->second,params.c_str());

}

/**     
	\ingroup plugin
	Do some explicit instanciation for a plugin based on TFactory 
*/
#define EXPLICIT_INSTANCE_PLUGIN(T) \
	template class TPlugin<T::plugin_data, T::plugin_type>; \
	template class TFactory<T>;					

/**     
	\ingroup plugin
	Do some explicit instanciation for a plugin based on TFactoryPluginHandler
*/
#define EXPLICIT_INSTANCE_PLUGIN_HANDLER(P) \
	template class TPluginHandler<P>;			\
	template class TFactoryPluginHandler<P>;		\
	template class THandlerSingleton<TFactoryPluginHandler<P> >;

/** 
    \ingroup plugin
    Do some explicit instanciation for the plugin classe and the handler of 
    a plugin based on TFactoryPluginHandler 
*/
#define EXPLICIT_INSTANCE_HANDLER(T) \
	template class TPlugin<T::plugin_data, T::plugin_type>; \
	template class TFactory<T>;					\
	template class TPluginHandler<TFactory<T> >;			\
	template class TFactoryPluginHandler<TFactory<T> >;		\
	template class THandlerSingleton<TFactoryPluginHandler<TFactory<T> > >;

/** 
    \ingroup plugin
   Do an explicit instanciation of plug-in classes and handlers for plugins that are 
   explicitely derived from TFactory.  
 */

#define EXPLICIT_INSTANCE_DERIVED_FACTORY_HANDLER(T, F)		\
	template class TPlugin<T::plugin_data, T::plugin_type>; \
	template class TFactory<T>;					\
	template class TPluginHandler<F>;			\
	template class TFactoryPluginHandler<F>;		\
	template class THandlerSingleton<TFactoryPluginHandler<F> >;



NS_MIA_END
#endif
