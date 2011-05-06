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


#ifndef __mia_core_noisegen_hh
#define __mia_core_noisegen_hh

#include <mia/core/defines.hh>
#include <mia/core/factory.hh>
#include <mia/core/handler.hh>

NS_MIA_BEGIN

struct EXPORT_CORE grayscale_noise_data {
	static const char *type_descr;
};


struct EXPORT_CORE generator_type {
	static const char *value;
};

/**
   \brief Base class for the noise generators.

   This class defines the interface for noise generators. Real implementations 
   are provided as plug-ins. 
*/
class EXPORT_CORE  CNoiseGenerator : public CProductBase {
public:
	/// typedef helber for plugin search path 
	typedef grayscale_noise_data plugin_data; 
	
	/// typedef helber for plugin search path 
	typedef generator_type plugin_type; 

	/**
	   Constructor to initialize the noise generator with the given seed 
	   \param seed 
	 */
	CNoiseGenerator(unsigned int seed);

	
	virtual ~CNoiseGenerator();
	
	/// \returns the next random value comprising the noise 
	double operator ()()const;
protected:
	/** @return a random value by using the system call to rand()  and 
	    that is then used to create the noise as requested 
	    
	 */
	double ranf() const;
private:
	virtual double get() const = 0;

};

/// Base class for al noise generator plugins 
typedef TFactory<CNoiseGenerator> CNoiseGeneratorPlugin;

/// The noise generator plugin handler 
typedef THandlerSingleton<TFactoryPluginHandler<CNoiseGeneratorPlugin> > 
      CNoiseGeneratorPluginHandler;

FACTORY_TRAIT(CNoiseGeneratorPluginHandler); 


NS_MIA_END

#endif
