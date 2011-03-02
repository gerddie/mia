/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
 *
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

#include <stdexcept>
#include <sstream>
#include <boost/lambda/lambda.hpp>
#include <algorithm>
#include <mia/core/minimizer.hh>
#include <mia/core/errormacro.hh>

#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>


NS_MIA_BEGIN

const char *const CMinimizer::value = "vector"; 
const char *const CMinimizer::type_descr = "minimizer"; 

CMinimizer::Problem::Problem(size_t n_params):
	m_size(n_params) 
{
}
	
size_t CMinimizer::Problem::size() const
{
	return m_size; 
}

CMinimizer::CMinimizer()
{
}

void CMinimizer::set_problem(PProblem p)
{
	m_problem = p; 
}
	

CMinimizer::~CMinimizer()
{
}

CMinimizer::Problem& CMinimizer::get_problem()
{
	return *m_problem; 
}
	
int CMinimizer::run(DoubleVector& x)
{
	DEBUG_ASSERT_RELEASE_THROW(m_problem, "CMinimizer::run: no minimization problem given");
	if (!m_problem->has_all_in(*this)) {
		stringstream msg; 
		msg << "The optimizer requests some properties that the minimization problem"
			" doesn't provide:"; 
		auto missing = m_problem->get_missing_properties(*this); 
		for (auto i = missing.begin(); i != missing.end(); ++i) 
			msg << "'" <<  *i << "' ";
		throw invalid_argument(msg.str()); 
	}
	return do_run(x); 
}


EXPLICIT_INSTANCE_HANDLER(CMinimizer);

NS_MIA_END
