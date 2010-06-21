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
#include <mia/2d/groundtruthproblem.hh>

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

	gsl::CFDFMinimizer minimizer(gtp,  gsl_multimin_fdfminimizer_vector_bfgs2);
	
	DoubleVector gt(input);
	minimizer.run(gt); 
	 
	// copy back the result 
	std::vector<P2DImage> result(originals.size()); 
	auto igt = gt.begin(); 

	for(size_t i = 0; i < originals.size(); ++i) {
		C2DFImage *image = new C2DFImage(originals[0]->get_size()); 
		copy(igt, igt + slice_size, image->begin()); 
		result[i] = P2DImage(image); 
		igt += slice_size; 
	}
	
	return result; 
}

DataCopy::DataCopy(DoubleVector& target, size_t slice_size):
	m_target(target), 
	i_target(target.begin()), 
	m_slice_size(slice_size)
{
	assert( target.size() % m_slice_size == 0); 

	cvdebug() << "Image size =" << slice_size << "\n"; 
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
