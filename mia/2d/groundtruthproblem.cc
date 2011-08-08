/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <numeric>
#include <mia/2d/groundtruthproblem.hh>

NS_MIA_BEGIN

using namespace std;

GroundTruthProblem::GroundTruthProblem(double a, double b,
				       const C2DBounds& slice_size,
				       size_t nframes,
				       const CDoubleVector& left_side,
				       const  CCorrelationEvaluator::result_type& corr):
	m_a(a),
	m_b(b),
	m_slice_size(slice_size),
	m_nframes(nframes),
	m_frame_size(slice_size.x * slice_size.y),
	m_left_side(left_side),
	m_corr(corr)
{
	add(property_gradient); 
}

size_t GroundTruthProblem::do_size() const
{
	return m_left_side.size(); 
}

// this function can certainly be optimized
double GroundTruthProblem::evaluate_slice_gradient(CDoubleVector::const_iterator ii,
						   vector<double>::iterator iout)
{
	double value = 0.0;
	{
		auto icorH = m_corr.horizontal.begin();
		auto icorV = m_corr.vertical.begin();

		// evaluate first row
		for (size_t x = 0; x < m_slice_size.x; ++x) {
			const double delta = ii[x] - ii[x+m_slice_size.x] ;
			iout[x] = 2 * delta * icorV[x];
			value += delta * delta * icorV[x];
		}

		for (size_t x = 0; x < m_slice_size.x - 1; ++x) {
			const double delta =  ii[x] - ii[x+1];
			iout[x] += 2 * delta * icorH[x];
			value += delta * delta * icorH[x];
		}

		for (size_t x = 1; x < m_slice_size.x; ++x) {
			const double delta = ii[x] - ii[x-1];
			iout[x] += 2 * delta * icorH[x-1];
			value += delta * delta * icorH[x - 1];
		}

	}

	for (size_t y = 1;  y < m_slice_size.y - 1; ++y)  {

		auto iii = ii + y * m_slice_size.x;
		auto iiout = iout + y * m_slice_size.x;
		auto icorH = m_corr.horizontal.begin_at(0,y);
		auto icorVm = m_corr.vertical.begin_at(0,y-1);
		auto icorVp = m_corr.vertical.begin_at(0,y);

		for (size_t x = 0; x < m_slice_size.x; ++x) {
			double delta = iii[x] - iii[x+m_slice_size.x];
			iiout[x] = 2 * delta * icorVp[x];
			value += delta * delta * icorVp[x];
		}

		for (size_t x = 0; x < m_slice_size.x; ++x) {
			double delta = iii[x] - iii[x - m_slice_size.x];
			iiout[x] += 2 * delta * icorVm[x];
			value += delta * delta * icorVm[x];
		}


		for (size_t x = 0; x < m_slice_size.x - 1; ++x) {
			double delta = iii[x] - iii[x+1];
			iiout[x] += 2 * delta * icorH[x];
			value += delta * delta * icorH[x];
		}

		for (size_t x = 1; x < m_slice_size.x; ++x) {
			double delta = iii[x] - iii[x-1];
			iiout[x] += 2 * delta * icorH[x-1];
			value +=  delta * delta * icorH[x-1];
		}
	}
	{

		auto iii = ii + (m_slice_size.y - 1) * m_slice_size.x;
		auto iiout = iout + (m_slice_size.y - 1) * m_slice_size.x;
		auto icorH = m_corr.horizontal.begin_at(0,m_slice_size.y - 1);
		auto icorVm = m_corr.vertical.begin_at(0,m_slice_size.y - 2);
	// evaluate last row

		for (size_t x = 0; x < m_slice_size.x; ++x) {
			double delta = iii[x] - iii[x-m_slice_size.x];
			iiout[x] = 2 * delta * icorVm[x];
			value +=  delta * delta * icorVm[x];
		}

		for (size_t x = 0; x < m_slice_size.x - 1; ++x) {
			double delta = iii[x] - iii[x+1];
			iiout[x] += 2 * delta * icorH[x];
			value +=  delta * delta * icorH[x];
		}

		for (size_t x = 1; x < m_slice_size.x; ++x) {
			double delta = iii[x] - iii[x-1];
			iiout[x] += 2 * delta * icorH[x-1];
			value +=  delta * delta * icorH[x-1];
		}
	}
	return value;
}

double GroundTruthProblem::evaluate_spacial_gradients(const CDoubleVector& x)
{
	double result = 0.0;
	m_spacial_gradient.resize(x.size());

	for(size_t f = 0; f < m_nframes; ++f) {
		result += evaluate_slice_gradient(x.begin() + f *m_frame_size,
						  m_spacial_gradient.begin() + f *m_frame_size);
	}
	return result;
}

double  GroundTruthProblem::evaluate_time_gradients(const CDoubleVector& x)
{
	/*
	  The finite difference test shows that in the first and in the last
	  slice the gradient needs the factor 5, while in the other slices a
	  factor 6 is used (the latter follows from a proper derivation)
	 */

	double value = 0.0;
	m_time_derivative.resize(x.size());

	auto im2 = x.begin();
	auto im = x.begin();
	auto ii = x.begin();
	auto ip = x.begin() + m_frame_size;
	auto ip2 = x.begin() + 2*m_frame_size;
	auto ih = m_time_derivative.begin();

	for (size_t k = 0; k < m_frame_size; ++k, ++ih, ++ip, ++ii, ++ip2) {
		const double v =  2 * *ii - 2 * *ip ;
		value += v * v;
		*ih = 5 * *ii +  *ip2 - 6 * *ip;
	}

	for (size_t k = 0; k < m_frame_size; ++k, ++ih, ++ip, ++ii, ++ip2, ++im) {
		const double imp = *ip + *im;
		const double v =  2 * *ii - imp;
		value += v * v;
		*ih = 9 *  *ii +  *ip2 - 6 * *im - 4 * *ip;
	}

	for(size_t f = 2; f < m_nframes - 2; ++f) {
		for (size_t k = 0; k < m_frame_size; ++k, ++ih, ++ip, ++im, ++ii, ++ip2, ++im2) {
			const double imp = *ip + *im;
			const double v =  2 * *ii - imp;
			value += v * v;
			*ih = 6 * *ii +  (*ip2 + *im2)  - 4 * imp;
		}

	}

	for (size_t k = 0; k < m_frame_size; ++k, ++ih, ++im, ++ii, ++ip, ++im2) {
		const double imp = *ip + *im;
		double v =  2 * *ii - imp;
		value += v * v;
		*ih = 9 *  *ii +  *im2 - 6 * *ip - 4 * *im;
	}

	for (size_t k = 0; k < m_frame_size; ++k, ++ih, ++im, ++ii, ++im2) {
		double v =  2 * *ii -  2 * *im;
		value += v * v;
		*ih = 5 * *ii +  *im2 - 6 * *im;
	}

	return value;
}


double  GroundTruthProblem::do_f(const CDoubleVector& x)
{
	double spacial_cost = evaluate_spacial_gradients(x);
	double temporal_cost = evaluate_time_gradients(x);

	double result = 0.0;
	for(auto ix = x.begin(), il = m_left_side.begin();
	    ix != x.end(); ++ix, ++il) {
		double v = *ix - *il;
		result += v*v;
	}
	// inner product them with alpha
	result += m_a * spacial_cost;
	result += m_b * temporal_cost;
	result *=  0.5;
	cvmsg() << "GroundTruthProblem::f = " << result << "\n";

	return result;
}

void GroundTruthProblem::do_df(const CDoubleVector&  x, CDoubleVector&  g)
{
	evaluate_spacial_gradients(x);
	evaluate_time_gradients(x);

	auto ix = x.begin();
	auto il = m_left_side.begin();
	auto ddt = m_time_derivative.begin();
	auto dds = m_spacial_gradient.begin();

	for (auto ig = g.begin(); ig != g.end(); ++ig, ++ix, ++ddt, ++dds, ++il) {
		*ig =  (*ix - *il) + m_a * *dds + m_b * *ddt;
	}

}

double  GroundTruthProblem::do_fdf(const CDoubleVector&  x, CDoubleVector&  g)
{
	const double spacial_cost = evaluate_spacial_gradients(x);
	const double temporal_cost = evaluate_time_gradients(x);

	double result = 0.0;
	for(auto ix = x.begin(), il = m_left_side.begin();
	    ix != x.end(); ++ix, ++il) {
		double v = *ix - *il;
		result += v*v;
	}

	result += m_a * spacial_cost;
	result += m_b * temporal_cost;

	auto ix = x.begin();
	auto il = m_left_side.begin();
	auto ddt = m_time_derivative.begin();
	auto dds = m_spacial_gradient.begin();

	for (auto ig = g.begin(); ig != g.end(); ++ig, ++ix, ++ddt, ++dds, ++il) {
		*ig =  *ix - *il + m_a * *dds + m_b * *ddt;
	}

	result *=  0.5;
	cvmsg() << "GroundTruthProblem::f = " << result << "\n";

	return result;

}

void GroundTruthProblem::set_alpha_beta(double a, double b)
{
	m_a = a;
	m_b = b;
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
