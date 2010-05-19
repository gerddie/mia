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
#include <mia/core/errormacro.hh>
#include <mia/core/optimizer.hh>
#include <mia/core/property_flags.hh>

NS_MIA_BEGIN

using namespace std; 

COptimizer::~COptimizer()
{
}

COptimizer::EOptimizerResults COptimizer::run(CProblem& problem)
{
	EOptimizerResults result = or_failed;
	
	if (has_property(property_gradient) &&
	    !problem.has_property(property_gradient)) {
		THROW(invalid_argument, "Optimizer '" << get_name() 
		      <<  "' requires a gradient, but the problem formulation '"
		      << problem.get_name() << "' doesn't provide one"); 
	}
	
	problem.setup(); 
	result = do_run(problem); 
	problem.finalize(); 
	
	return result; 
}


NS_MIA_END
