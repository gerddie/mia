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

#define VSTREAM_DOMAIN "ICA-BASE"

#include <mia/core/export_handler.hh>
#include <mia/core/ica.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>


namespace  mia {

CIndepCompAnalysis::~CIndepCompAnalysis()
{

}

CIndepCompAnalysisFactory::CIndepCompAnalysisFactory():
	m_deterministic_seed(-1)
{
}
	
void CIndepCompAnalysisFactory::set_deterministic_seed(int seed)
{
	m_deterministic_seed = seed; 
}

CIndepCompAnalysis *CIndepCompAnalysisFactory::create() const
{
	auto retval = do_create();
	if (m_deterministic_seed >= 0) {
		cvdebug() << "Set deterministic to " << m_deterministic_seed << "\n";
		retval->set_deterministic_seed(m_deterministic_seed);
	}
	return retval; 
}

CIndepCompAnalysisFactory::~CIndepCompAnalysisFactory()
{

}

const char *CIndepCompAnalysisFactory::data_descr = "fastica";
const char *CIndepCompAnalysisFactory::type_descr = "implementation";

template<>  const char * const 
TPluginHandler<TFactory<CIndepCompAnalysisFactory>>::m_help = 
	"These plug-ins provide implementations for the ICA algoritm.";

EXPLICIT_INSTANCE_HANDLER(CIndepCompAnalysisFactory); 

}
