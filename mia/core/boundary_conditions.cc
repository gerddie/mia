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

#include <cassert>
#include <mia/core/msgstream.hh>
#include <mia/core/boundary_conditions.hh>

NS_MIA_BEGIN
using std::vector; 
using std::invalid_argument; 

CBoundaryCondition::CBoundaryCondition():
	m_width(0)
{
	
}

CBoundaryCondition::CBoundaryCondition(int width):
	m_width(width)
{
}

void CBoundaryCondition::set_width(int width)
{
	m_width = width; 
	do_set_width(width); 
}

void CBoundaryCondition::do_set_width(int width)
{
}

void CBoundaryCondition::filter_line(std::vector<double>& coeff, const std::vector<double>& poles)const
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


bool CBoundaryCondition::apply(std::vector<int>& index, std::vector<double>& weights) const
{
	assert(m_width > 0); 
	if ( (index[0] >= 0) && index[index.size()-1] < m_width) 
		return true; 
	do_apply(index, weights); 
	return false; 
}

CMirrorOnBoundary::CMirrorOnBoundary():
	m_width2(0)
{
}

CMirrorOnBoundary::CMirrorOnBoundary(int width):
	CBoundaryCondition(width), 
	m_width2(2*width - 2)
{
}


void CMirrorOnBoundary::do_set_width(int width)
{
	m_width2 = 2*width - 2; 
}

void CMirrorOnBoundary::test_supported(int npoles) const
{
}

void CMirrorOnBoundary::do_apply(std::vector<int>& index, std::vector<double>& weights) const
{
	for (size_t k = 0; k < index.size(); k++) {
		int idx = (index[k] < 0) ? -index[k] : index[k]; 
		
		idx = (get_width() == 1) ? (0) : ((idx < m_width2) ? idx : idx % m_width2);
		if (get_width() <= idx) {
			cvdebug() << "yes:" << idx << "\n"; 
			idx = m_width2 - idx;
		}
		index[k] = idx; 
		cvdebug() << k << ": " << index[k] << "\n"; 
	}
}


double CMirrorOnBoundary::initial_coeff(const std::vector<double>& coeff, double pole)const
{ 
	double zn = pole;
	double iz = 1.0 / pole;
	double z2n = pow(pole, (double)(coeff.size() - 1));
	double sum = coeff[0] + z2n * coeff[coeff.size() - 1];
	
	z2n *= z2n * iz;
	
	for (size_t n = 1; n < coeff.size()  - 1; n++) {
		sum += (zn + z2n) * coeff[n];
		zn *= pole;
		z2n *= iz;
	}
	
	return(sum / (1.0 - zn * zn));
}

double CMirrorOnBoundary::initial_anti_coeff(const std::vector<double>& coeff, double pole)const
{
	return ((pole / (pole * pole - 1.0)) * 
		(pole * coeff[coeff.size() - 2] + coeff[coeff.size() - 1]));
	
}


CZeroBoundary::CZeroBoundary(int width):
	CBoundaryCondition(width)
{
}
void CZeroBoundary::test_supported(int npoles) const
{
	/**
	   Unfortunately, the current pre-filtering does not work properly 
	   for splines with more then one pole. 
	 */

	if (npoles > 1) {
		THROW(invalid_argument, "Currently, zero-boundary not supported for splines with more then one pole");  
	}
}


void CZeroBoundary::do_apply(std::vector<int>& index, std::vector<double>& weights) const
{
	for (size_t k = 0; k < index.size(); k++) {
		if (index[k] < 0 || index[k] >= get_width()) {
			index[k] = 0; 
			weights[k] = 0; 
		}	
	}
}

double CZeroBoundary::initial_coeff(const std::vector<double>& coeff, double pole) const
{
	double zn = pole  * pole;
	double ip = 1.0/ pole; 
	double z2n = pow(pole, 2.0 * coeff.size()+2) ;

	double sum = 0.0; 
	for (size_t n = 0; n < coeff.size() - 1; n++) {
		sum -= (zn + z2n) * coeff [n]; 
		zn *= pole;
		z2n *= ip; 
	}
	
	sum -= zn * coeff [coeff.size() - 1]; 
	return coeff [0] + sum / ( 1- zn * zn); 

}

double CZeroBoundary::initial_anti_coeff(const std::vector<double>& coeff, double pole)const
{
	return - coeff[coeff.size() - 1] * pole; 
}

CRepeatBoundary::CRepeatBoundary():
	m_widthm1(0)
{
}

CRepeatBoundary::CRepeatBoundary(int width):
	CBoundaryCondition(width), 
	m_widthm1(width-1)
{
}

void CRepeatBoundary::test_supported(int npoles) const
{
	if (npoles > 1) {
		THROW(invalid_argument, "Currently, repeat-boundary not supported for splines woth more then one pole");  
	}
}

void CRepeatBoundary::do_set_width(int width)
{
	m_widthm1 = width-1; 
}

void CRepeatBoundary::do_apply(std::vector<int>& index, std::vector<double>& weights) const
{
	for (size_t k = 0; k < index.size(); k++) {
		if (index[k] < 0) 
			index[k] = 0; 
		else if (index[k] > m_widthm1) {
			index[k] = m_widthm1; 
		}	
	}
}


double CRepeatBoundary::initial_coeff(const std::vector<double>& coeff, double pole)const
{ 
	double zn = pole;
	double iz = 1.0 / pole;
	double z2n = pow(pole, (double)(2 * coeff.size()));

	double sum = 0.0; 
	for (size_t n = 0; n < coeff.size(); n++) {
		sum += (zn  + z2n) * coeff [n]; 
		zn *= pole;
		z2n *= iz;
	}
	return coeff[0] + sum / ( 1 - zn * zn); 
}

double CRepeatBoundary::initial_anti_coeff(const std::vector<double>& coeff, double pole)const
{
	return - pole / (1 - pole) * coeff[coeff.size() - 1];
}


NS_MIA_END
