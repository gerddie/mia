/*
** Copyright Madrid (c) 2010 BIT ETSIT UPM
**                    Gert Wollny <gw.fossdev @ gmail.com>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#include <mia/2d/groundtruthproblem.hh>

NS_MIA_BEGIN

using namespace std; 
using gsl::DoubleVector; 

GroundTruthProblem::GroundTruthProblem(double a, double b, 
				       const C2DBounds& slice_size, 
				       size_t nframes, 
				       const gsl::DoubleVector& left_side,
				       const  CCorrelationEvaluator::result_type& corr):
	gsl::CFDFMinimizer::Problem(left_side.size()),
	m_a(a), 
	m_b(b),
	m_slice_size(slice_size), 
	m_nframes(nframes), 
	m_frame_size(slice_size.x * slice_size.y), 
	m_left_side(left_side), 
	m_corr(corr)
{
}


void GroundTruthProblem::evaluate_slice_gradient(DoubleVector::const_iterator ii,  vector<double>::iterator iout) 
{
	{
		auto icorH = m_corr.horizontal.begin(); 
		auto icorV = m_corr.vertical.begin(); 
		
		// evaluate first row
		for (size_t x = 0; x < m_slice_size.x - 1; ++x)
			iout[x] = (ii[x+1] - ii[x]) * icorH[x]; 
		
		for (size_t x = 1; x < m_slice_size.x; ++x)
			iout[x] += (ii[x] - ii[x-1]) * icorH[x-1]; 

		for (size_t x = 0; x < m_slice_size.x; ++x)
			iout[x] += (ii[x+m_slice_size.x] - ii[x]) * icorV[x]; 
	}
	
	for (size_t y = 1;  y < m_slice_size.y - 1; ++y)  {
		
		auto iii = ii + y * m_slice_size.x; 
		auto iiout = iout + y * m_slice_size.x; 
		auto icorH = m_corr.horizontal.begin_at(0,y); 
		auto icorVm = m_corr.vertical.begin_at(0,y-1); 
		auto icorVp = m_corr.vertical.begin_at(0,y); 

		for (size_t x = 0; x < m_slice_size.x - 1; ++x)
			iiout[x] = (iii[x+1] - iii[x]) * icorH[x]; 

		for (size_t x = 1; x < m_slice_size.x; ++x)
			iiout[x] += (iii[x] - iii[x-1]) * icorH[x-1]; 
		
		for (size_t x = 0; x < m_slice_size.x; ++x)
			iiout[x] += (iii[x+m_slice_size.x] - iii[x])* icorVp[x]; 
		
		for (size_t x = 0; x < m_slice_size.x; ++x)
			iiout[x] += (iii[x] - iii[x-m_slice_size.x])* icorVm[x]; 

	}
	{
		
		auto iii = ii + (m_slice_size.y - 1) * m_slice_size.x; 
		auto iiout = iout + (m_slice_size.y - 1) * m_slice_size.x; 
		auto icorH = m_corr.horizontal.begin_at(0,m_slice_size.y - 1); 
		auto icorVm = m_corr.vertical.begin_at(0,m_slice_size.y - 2); 
	// evaluate last row
		for (size_t x = 0; x < m_slice_size.x - 1; ++x) {
			iiout[x] = (iii[x+1] - iii[x]) * icorH[x]; 
		}
		
		for (size_t x = 0; x < m_slice_size.x; ++x)
			iiout[x] += (iii[x] - iii[x-m_slice_size.x]) * icorVm[x]; 
		
		for (size_t x = 1; x < m_slice_size.x; ++x) 
			iiout[x] += (iii[x] - iii[x-1]) * icorH[x-1]; 
	}
}

void GroundTruthProblem::evaluate_spacial_gradients(const DoubleVector& x)
{
	m_spacial_gradient.resize(x.size()); 
	
	for(size_t f = 0; f < m_nframes; ++f) {
		evaluate_slice_gradient(x.begin() + f *m_frame_size, 
					m_spacial_gradient.begin() + f *m_frame_size); 
	}
}

void GroundTruthProblem::evaluate_time_gradients(const DoubleVector& x)
{
	m_time_derivative.resize(x.size()); 
	
	fill(m_time_derivative.begin(), m_time_derivative.begin() + m_frame_size, 0.0); 
	auto im = x.begin(); 
	auto ii = x.begin() + m_frame_size; 
	auto ip = x.begin() + 2 * m_frame_size; 
	
	auto io = m_time_derivative.begin() + m_frame_size; 

	for(size_t f = 1; f < m_nframes - 1; ++f) {
		for (size_t k = 0; k < m_frame_size; ++k, ++io, ++ip, ++im, ++ii)
			*io = 2* *ii - *im - *ip; 
	}

	fill(io, m_time_derivative.end(), 0.0);
}


double  GroundTruthProblem::do_f(const DoubleVector& x)
{
	double result = 0.0; 
	for(auto ix = x.begin(), il = m_left_side.begin(); 
	    ix != x.end(); ++ix, ++il) {
		double v = *ix - *il; 
		result += v*v; 
	}
	
	// inner product them with alpha 
	for (auto i = m_spacial_gradient.begin(); i != m_spacial_gradient.end(); ++i) 
		result += *i * *i * m_a; 

	// inner product then with beta 
	for (auto i = m_time_derivative.begin(); i != m_time_derivative.end(); ++i) 
		result += *i * *i * m_b; 

	cvinfo() << "GroundTruthProblem::f = " << result << "\n"; 
	return result * 0.5; 
}

void GroundTruthProblem::do_df(const DoubleVector&  x, DoubleVector&  g)
{
	evaluate_spacial_gradients(x); 
	evaluate_time_gradients(x); 

	
	auto ix = x.begin(); 
	auto il = m_left_side.begin(); 
	auto ddt = m_time_derivative.begin(); 
	auto dds = m_spacial_gradient.begin(); 
	
	cvdebug() << " GroundTruthProblem::g = "; 
	for (auto ig = g.begin(); ig != g.end(); ++ig, ++ix, ++ddt, ++dds, ++il) {
		*ig =  *ix - *il + m_a * *dds + m_b * *ddt; 
		cverb << *ig << " "; 
	}
	cverb  << "\n"; 

}

double  GroundTruthProblem::do_fdf(const DoubleVector&  x, DoubleVector&  g)
{
	evaluate_spacial_gradients(x); 
	evaluate_time_gradients(x); 

	auto ix = x.begin(); 
	auto il = m_left_side.begin(); 
	auto ddt = m_time_derivative.begin(); 
	auto dds = m_spacial_gradient.begin(); 
	cvdebug() << " GroundTruthProblem::g = "; 
	for (auto ig = g.begin(); ig != g.end(); ++ig, ++ix, ++ddt, ++dds, ++il) {
		*ig =  *ix - *il + m_a * *dds + m_b * *ddt; 
		cverb << *ig << " "; 
	}
	cverb << "\n"; 

	double result = 0.0; 
	for(auto ix = x.begin(), il = m_left_side.begin(); 
	    ix != x.end(); ++ix, ++il) {
		double v = *ix - *il; 
		result += v*v; 
	}

	for (auto i = m_spacial_gradient.begin(); i != m_spacial_gradient.end(); ++i) 
		result += *i * *i * m_a; 

	for (auto i = m_time_derivative.begin(); i != m_time_derivative.end(); ++i) 
		result += *i * *i * m_b; 
	
	cvinfo() << "GroundTruthProblem::fdf = " << result << "\n"; 
	return result * 0.5; 	
}

const std::vector<double>& GroundTruthProblem::get_spacial_gradient() const
{
	return m_spacial_gradient; 
}

const std::vector<double>& GroundTruthProblem::get_time_derivative() const
{
	return m_time_derivative; 
}

NS_MIA_END
