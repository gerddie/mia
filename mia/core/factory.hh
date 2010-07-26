/* -*- mona-c++  -*-
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

// $Id: factory.hh,v 1.4 2005/06/03 11:00:38 gerddie Exp $

/** 
    \author: Gert Wollny < wollny at cbs mpg de >
    implements a plugin loading factory
*/

#ifndef FACTORY_HH
#define FACTORY_HH

#include <iostream>
#include <memory>
#include <string>
#include <mia/core/shared_ptr.hh>

#include <mia/core/handler.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/plugin_base.hh>
#include <mia/core/product_base.hh>
#include <mia/core/optionparser.hh>

#include <mia/core/import_handler.hh>

NS_MIA_BEGIN

/** this is the class to load a certain plugin */
template <typename P, typename D, typename T>
class EXPORT_HANDLER TFactory: public TPlugin<D,T> {
public: 
	typedef P Product; 
	typedef SHARED_PTR(P) ProductPtr; 
	
	/** initialise the plugin by the names 
	    \remark what are these names and types good for?
	*/
	TFactory(char const * const  name);
	
	/** the creation function 
	    \params options the options to initialise the plugin 
	    \returns an instance of the requested object
	*/
	virtual ProductPtr create(const CParsedOptions& options);
	
private:
	virtual bool do_test() const; 
	virtual ProductPtr do_create() const = 0;
};




template <typename  P>
class EXPORT_HANDLER TFactoryPluginHandler: public  TPluginHandler< P > {
protected: 
	//! \name Constructors
        //@{
        /*! \brief Initializes the plugin handler based on a given plugin search path list 
	  \param searchpath list of directories to search for plugins 
	*/
	
	
	TFactoryPluginHandler(const std::list<boost::filesystem::path>& searchpath); 
        //@}
public: 	
	typename P::ProductPtr produce(const char *plugindescr) const;

	typename P::ProductPtr produce(const std::string& params)const {
		return produce(params.c_str()); 
	}

	
}; 


template <typename P, typename D, typename T>
TFactory<P,D,T>::TFactory(char const * const  name):
	TPlugin<D,T>(name)
{
}

template <typename P, typename D, typename T>
typename TFactory<P,D,T>::ProductPtr TFactory<P,D,T>::create(const CParsedOptions& options)
{
	this->set_parameters(options);
	this->check_parameters();
	ProductPtr product = this->do_create();
	if (product) 
		product->set_module(this->get_module()); 
	return product; 
}

template <typename  P>
TFactoryPluginHandler<P>::TFactoryPluginHandler(const std::list<boost::filesystem::path>& searchpath):
	TPluginHandler< P >(searchpath)
{
}

	
template <typename  P>
typename P::ProductPtr TFactoryPluginHandler<P>::produce(char const *params)const
{
	typedef typename P::ProductPtr ProductPtr; 
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
		return factory->create(param_list.begin()->second);
	else 
		return ProductPtr(); 
}

template <typename P, typename D, typename T>
bool TFactory<P,D,T>::do_test() const
{
	cvfail() << "do_test() is obsolete\n"; 
	return false; 
}

NS_MIA_END
#endif
