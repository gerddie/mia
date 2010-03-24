/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
   Base class for the noise generators. 
 */


class EXPORT_CORE  CNoiseGenerator : public CProductBase {
public:
	CNoiseGenerator(unsigned int seed);
	virtual ~CNoiseGenerator();
	double operator ()()const;
protected:
	double ranf() const;
private:
	virtual double get() const = 0;

};

typedef TFactory<CNoiseGenerator, grayscale_noise_data, generator_type> CNoiseGeneratorPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<CNoiseGeneratorPlugin> > CNoiseGeneratorPluginHandler;



NS_MIA_END

#endif
