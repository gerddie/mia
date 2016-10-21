/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/core/export_handler.hh>

#define VSTREAM_DOMAIN "NOISE_GENERATOR"

#include <ctime>
#include <cstdlib>

#include <mia/core/noisegen.hh>
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>


NS_MIA_BEGIN


const char *grayscale_noise_data::data_descr = "generator";
const char *generator_type::type_descr = "noise";

CNoiseGenerator::CNoiseGenerator(unsigned int seed)
{
	if (seed)
		srand(seed);
	else
		srand(time(NULL));
}
CNoiseGenerator::~CNoiseGenerator()
{
}

double CNoiseGenerator::operator ()()const
{
	return get();
}

double CNoiseGenerator::ranf() const
{
	double x = rand();

	return x / RAND_MAX;
}

template<> const char * const 
TPluginHandler<TFactory<CNoiseGenerator>>::m_help = "These plug-ins provide various noise generators.";


EXPLICIT_INSTANCE_HANDLER(CNoiseGenerator); 

NS_MIA_END
