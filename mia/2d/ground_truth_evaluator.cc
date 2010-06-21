
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

#include <numeric>
#include <mia/2d/ground_truth_evaluator.hh>
#include <mia/2d/correlation_weight.hh>
#include <gsl++/multimin.hh>

NS_MIA_BEGIN

using gsl::DoubleVector; 
using namespace std; 

struct C2DGroundTruthEvaluatorImpl {
	C2DGroundTruthEvaluatorImpl(double alpha, double beta, double rho); 
	std::vector<P2DImage> run(const std::vector<P2DImage>& originals) const; 
private: 
	double m_alpha;
	double m_beta; 
	double m_rho;
};

C2DGroundTruthEvaluator::C2DGroundTruthEvaluator(double alpha, double beta, double rho):
	impl(new C2DGroundTruthEvaluatorImpl(alpha, beta, rho))
{
}


C2DGroundTruthEvaluator::~C2DGroundTruthEvaluator()
{
	delete impl; 
}

std::vector<P2DImage> C2DGroundTruthEvaluator::operator () (const std::vector<P2DImage>& originals) const
{
	return impl->run(originals); 
}

C2DGroundTruthEvaluatorImpl::C2DGroundTruthEvaluatorImpl(double alpha, double beta, double rho):
	m_alpha(alpha), 
	m_beta(beta), 
	m_rho(rho)

{
}

struct DataCopy : public TFilter<void> {
	DataCopy(gsl::DoubleVector& target, size_t ss); 
	
	template <typename T> 
	void operator() (const T2DImage<T>& image); 
	
private: 
	DoubleVector& m_target; 
	DoubleVector::iterator i_target; 
	size_t m_slice_size; 
}; 

class GroundTruthProblem : public gsl::CFDFMinimizer::Problem {
public:
	GroundTruthProblem(double a, double b, 
			   const C2DBounds& slice_size, 
			   size_t nframes, 
			   const gsl::DoubleVector& left_side, 
			   const  CCorrelationEvaluator::result_type& corr); 
private: 
	virtual double  do_f(const DoubleVector& x); 
	virtual void    do_df(const DoubleVector&  x, DoubleVector&  g); 
	virtual double  do_fdf(const DoubleVector&  x, DoubleVector&  g); 
	
	void evaluate_spacial_gradients(const DoubleVector& x); 
	void evaluate_time_gradients(const DoubleVector& x); 
	void evaluate_slice_gradient(DoubleVector::const_iterator ii,  vector<double>::iterator iout); 

	vector<double> m_spacial_gradient; 
	vector<double> m_time_derivative;
	
	double m_a;
	double m_b;
	const C2DBounds& m_slice_size; 
	size_t m_nframes; 
	size_t m_frame_size; 
	const gsl::DoubleVector& m_left_side; 
	const  CCorrelationEvaluator::result_type& m_corr; 
}; 

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
			iout[x] += (ii[x+1] - ii[x]) * icorH[x]; 
		for (size_t x = 1; x < m_slice_size.x; ++x)
			iout[x] += (ii[x] - ii[x-1]) * icorH[x-1]; 

		for (size_t x = 0; x < m_slice_size.x; ++x)
			iout[x] += (ii[x+m_slice_size.x] - ii[x]) * icorV[x]; 

	}
	
	for (size_t y = 1;  y < m_slice_size.y - 1; ++y)  {
		
		auto iii = ii + y * m_slice_size.x; 
		auto iiout = iout + y * m_slice_size.x; 
		auto iicorH = m_corr.horizontal.begin_at(0,y); 
		auto iicorVm = m_corr.vertical.begin_at(0,y-1); 
		auto iicorVp = m_corr.vertical.begin_at(0,y); 

		for (size_t x = 0; x < m_slice_size.x - 1; ++x)
			iiout[x] += (iii[x+1] - iii[x]) * iicorH[x]; 
		for (size_t x = 1; x < m_slice_size.x; ++x)
			iiout[x] += (iii[x] - iii[x-1]) * iicorH[x-1]; 
		
		for (size_t x = 0; x < m_slice_size.x; ++x)
			iiout[x] += (iii[x+m_slice_size.x] - iii[x])* iicorVm[x]; 
		for (size_t x = 0; x < m_slice_size.x; ++x)
			iiout[x] += (iii[x] - iii[x-m_slice_size.x])* iicorVp[x]; 
	}
	{
		
		auto iii = ii + (m_slice_size.y - 1) * m_slice_size.x; 
		auto iiout = iout + (m_slice_size.y - 1) * m_slice_size.x; 
		auto iicorH = m_corr.horizontal.begin_at(0,m_slice_size.y - 1); 
		auto iicorVm = m_corr.vertical.begin_at(0,m_slice_size.y - 2); 
	// evaluate last row
		for (size_t x = 0; x < m_slice_size.x - 1; ++x) {
			iiout[x] += (iii[x+1] - iii[x]) * iicorH[x]; 
		}
		
		for (size_t x = 0; x < m_slice_size.x; ++x)
			iiout[x] += (iii[x] - iii[x-m_slice_size.x])* iicorVm[x]; 
		
		for (size_t x = 1; x < m_slice_size.x; ++x) 
			iiout[x] += (iii[x] - iii[x-1]) * iicorH[x-1]; 

		
	}
}

void GroundTruthProblem::evaluate_spacial_gradients(const DoubleVector& x)
{
	m_spacial_gradient.resize(x.size()); 
	
	for(size_t f = 0; f < m_nframes; ++f) {
		cvdebug() << "slice " << f << "\n"; 
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

	for(size_t f = 1; f < m_nframes - 1; ++f, ++io, ++ip, ++im, ++ii)
		*io = 2* *ii - *im - *ip; 

	fill(io, m_time_derivative.end(), 0.0);
}


double  GroundTruthProblem::do_f(const DoubleVector& x)
{
	// (g-f)^T (g-f) 
	double result = inner_product(x.begin(), x.end(), m_left_side.begin(), 0.0); 
	
	
	// inner product them with alpha 
	for (auto i = m_spacial_gradient.begin(); i != m_spacial_gradient.end(); ++i) 
		result += *i * *i * m_a; 

	
	// inner product then with beta 
	for (auto i = m_time_derivative.begin(); i != m_time_derivative.end(); ++i) 
		result += *i * *i * m_b; 

	return result * 0.5; 
}

void GroundTruthProblem::do_df(const DoubleVector&  x, DoubleVector&  g)
{
	evaluate_spacial_gradients(x); 
	evaluate_time_gradients(x); 

	
	auto ix = x.begin(); 
	auto ddt = m_time_derivative.begin(); 
	auto dds = m_spacial_gradient.begin(); 
	
	for (auto ig = g.begin(); ig != g.end(); ++ig) {
		*ig = *ix + m_a * *dds + m_b * *ddt; 
	}
}

double  GroundTruthProblem::do_fdf(const DoubleVector&  x, DoubleVector&  g)
{
	evaluate_spacial_gradients(x); 
	evaluate_time_gradients(x); 

	auto ix = x.begin(); 
	auto ddt = m_time_derivative.begin(); 
	auto dds = m_spacial_gradient.begin(); 
	for (auto ig = g.begin(); ig != g.end(); ++ig) {
		*ig = *ix + m_a * *dds + m_b * *ddt; 
	}
	double result = inner_product(x.begin(), x.end(), m_left_side.begin(), 0.0); 
	for (auto i = m_spacial_gradient.begin(); i != m_spacial_gradient.end(); ++i) 
		result += *i * *i * m_a; 
	for (auto i = m_time_derivative.begin(); i != m_time_derivative.end(); ++i) 
		result += *i * *i * m_b; 
	return result * 0.5; 	
}

	

std::vector<P2DImage> C2DGroundTruthEvaluatorImpl::run(const std::vector<P2DImage>& originals) const
{
	CCorrelationEvaluator ce(m_rho); 
	auto correlation = ce(originals); 
	size_t slice_size = originals[0]->get_size().x * originals[0]->get_size().y; 

	size_t n = originals.size() * slice_size; 
	
	gsl::DoubleVector input(n, false); 
	
	DataCopy dc(input, slice_size);
	for (auto io = originals.begin(); io != originals.end(); ++io)
		::mia::accumulate(dc, **io); 

	gsl::CFDFMinimizer::PProblem gtp(new GroundTruthProblem(m_alpha, m_beta, originals[0]->get_size(), 
					    originals.size(), input, correlation));

	gsl::CFDFMinimizer minimizer(gtp,  gsl_multimin_fdfminimizer_conjugate_fr);
	
	DoubleVector gt(input);
	minimizer.run(gt); 
	 
	// copy back the result 
	std::vector<P2DImage> result(originals.size()); 
	auto igt = gt.begin(); 

	for(size_t i = 0; i < originals.size(); ++i) {
		C2DFImage *image = new C2DFImage(originals[0]->get_size()); 
		copy(igt, igt + originals.size(), image->begin()); 
		result[i] = P2DImage(image); 
		igt += originals.size(); 
	}
	
	return result; 
}

DataCopy::DataCopy(DoubleVector& target, size_t slice_size):
	m_target(target), 
	i_target(target.begin()), 
	m_slice_size(slice_size)
{
	assert( target.size() % m_slice_size == 0); 
}

template <typename T> 
void DataCopy::operator() (const T2DImage<T>& image)
{
	if (image.size() != m_slice_size) 
		THROW(invalid_argument, __FILE__ << ": unexpected input image pixel number " 
		      << image.size() << ", expect " << m_slice_size); 

	assert(i_target != m_target.end()); 

	copy(image.begin(), image.end(), i_target); 
	i_target += m_slice_size; 
}


NS_MIA_END
