/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2011 Gert Wollny 
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

/*
  LatexBeginPluginSection{Spline Interpolation Boundary Conditions}
  \label{sec:splinebc}
  
  These plug-ins implement various boundary conditions that are used when spline 
  interpolation is applied. 
  
  Specific boundary conditions are created by calling \emph{produce\_spline\_boundary\_condition}. 
  
  LatexEnd
*/


#include <limits>
#include <cassert>
#include <mia/core/export_handler.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/boundary_conditions.hh>
#include <mia/core/optionparser.hh>
#include <mia/core/optparam.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>
#include <boost/filesystem.hpp>


NS_MIA_BEGIN
using std::vector; 
using std::invalid_argument; 
using std::numeric_limits; 

const char * const CSplineBoundaryCondition::type_descr = "1d"; 
const char * const CSplineBoundaryCondition::data_descr = "splinebc"; 

CSplineBoundaryCondition::CSplineBoundaryCondition():
	m_width(0)
{
	
}

CSplineBoundaryCondition::CSplineBoundaryCondition(int width):
	m_width(width)
{
}

void CSplineBoundaryCondition::set_width(int width)
{
	m_width = width; 
	do_set_width(width); 
}

void CSplineBoundaryCondition::do_set_width(int width)
{
}

void CSplineBoundaryCondition::filter_line(std::vector<double>& coeff, const std::vector<double>& poles)const
{
	/* special case required by mirror boundaries */
	if (coeff.size() < 2) {
		return;
	}

	/// interpolating splines? 
	if (poles.empty()) 
		return; 

	test_supported(poles.size()); 

	/* compute the overall gain */
	double	lambda = 1.0;
	for (auto p = poles.begin(); p != poles.end(); ++p)
		lambda  *=  2 - *p - 1.0 / *p;
	
	/* apply the gain */
	for_each(coeff.begin(), coeff.end(), [lambda](double& x) { x *= lambda;});
	
	/* loop over all poles */
	for (size_t k = 0; k < poles.size(); ++k) {
		auto p = poles[k]; 

		coeff[0] = initial_coeff(coeff, p);
		cvdebug() << "initial coeff  ["<< k <<"]= " << coeff[0] << "\n"; 

		/* causal recursion */
		for (size_t n = 1; n < coeff.size(); ++n) {
			cvdebug() << "causal[" << n << "] = "<< coeff[n - 1] << " * " << p << " + " << coeff[n] << "\n"; 
			coeff[n] += p * coeff[n - 1];
			cvdebug() << "causal[" << n << "] = "<<coeff[n] << "\n"; 
		}
		
		/* anticausal initialization */
		coeff[coeff.size() - 1] = initial_anti_coeff(coeff, p);

		cvdebug() << "initial anti- coeff  ["<< coeff.size() - 1 <<"]= " << coeff[coeff.size() - 1] << "\n"; 
		/* anticausal recursion */
		for (int n = coeff.size() - 2; 0 <= n; n--) {
			cvdebug() << "anticoeff["<< n <<"]= " 
				  << p << " * (" << coeff[n + 1]  << " - " << coeff[n] << ") = " ; 
			
			coeff[n] = p * (coeff[n + 1] - coeff[n]);
			cverb << coeff[n] << "\n"; 
		}
	}
}

bool CSplineBoundaryCondition::apply(std::vector<int>& index, std::vector<double>& weights) const
{
	assert(m_width > 0); 
	if ( (index[0] >= 0) && index[index.size()-1] < m_width) 
		return true; 
	do_apply(index, weights); 
	return false; 
}


CSplineBoundaryConditionPlugin::CSplineBoundaryConditionPlugin(const char * name):
	TFactory<CSplineBoundaryCondition>(name), 
	m_width(0)
{
	add_parameter("w", new CIntParameter(m_width, 0, numeric_limits<int>::max(), false, "index range")); 
}
	
CSplineBoundaryCondition *CSplineBoundaryConditionPlugin::do_create() const
{
	return do_create(m_width); 
}

EXPLICIT_INSTANCE_DERIVED_FACTORY_HANDLER(CSplineBoundaryCondition, CSplineBoundaryConditionPlugin); 

using boost::filesystem::path; 
CSplineBoundaryConditionTestPath::CSplineBoundaryConditionTestPath()
{
	list< path> sksearchpath; 
	sksearchpath.push_back( path(MIA_BUILD_ROOT"/mia/core/splinebc"));
	CSplineBoundaryConditionPluginHandler::set_search_path(sksearchpath); 
	
}


PSplineBoundaryCondition produce_spline_boundary_condition(const std::string& descr, int width)
{
	stringstream d; 
	d << descr << ":w=" << width; 
	return CSplineBoundaryConditionPluginHandler::instance().produce_unique(d.str()); 
}



NS_MIA_END
