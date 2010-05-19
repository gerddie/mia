/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2009
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


#ifndef mia_core_optimizer_hh
#define mia_core_optimizer_hh

#include <mia/core/problem.hh>
#include <mia/core/plugin_base.hh>

NS_MIA_BEGIN

/**
   Base class for all optimizers 
*/

class COptimizer : public CPluginBase{
public: 
	enum EOptimizerResults { or_failed=0,
				 or_residum_low, 
				 or_gradient_low,
				 or_step_low, 
				 or_keep_running }; 

	virtual ~COptimizer(); 
	EOptimizerResults run(CProblem& problem); 
private: 

	/**
	   This one needs to be implemented to obtaine
	*/
	virtual EOptimizerResults do_run(CProblem& problem) const = 0;
}; 

NS_MIA_END

#endif
