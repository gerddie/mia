/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011 Gert Wollny 
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


#include <numeric>
#include <mia/2d/ground_truth_evaluator.hh>
#include <mia/2d/correlation_weight.hh>
#include <mia/2d/groundtruthproblem.hh>

NS_MIA_BEGIN

using namespace std;

struct C2DGroundTruthEvaluatorImpl {
	C2DGroundTruthEvaluatorImpl(double alpha, double beta, double rho);
	void run(const std::vector<P2DImage>& originals, std::vector<P2DImage>& estimate) const;
private:
	double m_alpha;
	double m_beta;
	CCorrelationEvaluator m_ce;
};

C2DGroundTruthEvaluator::C2DGroundTruthEvaluator(double alpha, double beta, double rho):
	impl(new C2DGroundTruthEvaluatorImpl(alpha, beta, rho))
{
}


C2DGroundTruthEvaluator::~C2DGroundTruthEvaluator()
{
	delete impl;
}

void C2DGroundTruthEvaluator::operator () (const std::vector<P2DImage>& originals, std::vector<P2DImage>& estimate) const
{
	impl->run(originals, estimate);
}

C2DGroundTruthEvaluatorImpl::C2DGroundTruthEvaluatorImpl(double alpha, double beta, double rho):
	m_alpha(alpha),
	m_beta(beta),
	m_ce(rho)

{
}

struct DataCopy : public TFilter<void> {
	DataCopy(CDoubleVector& target, size_t ss);

	template <typename T>
	void operator() (const T2DImage<T>& image);

private:
	CDoubleVector& m_target;
	CDoubleVector::iterator i_target;
	size_t m_slice_size;
};




void C2DGroundTruthEvaluatorImpl::run(const std::vector<P2DImage>& originals,
				      std::vector<P2DImage>& estimate) const
{
	auto correlation = m_ce(originals);

	size_t slice_size = originals[0]->get_size().x * originals[0]->get_size().y;
	size_t n = originals.size() * slice_size;

	// copy original data into vector for optimization
	CDoubleVector input(n, false);
	DataCopy dc(input, slice_size);
	for (auto io = originals.begin(); io != originals.end(); ++io)
		::mia::accumulate(dc, **io);

	// if an initial estimate is give, then copy it to the 
	// optimization output vector 
	// otherwise copy input to estimate 
	CDoubleVector output(n,false);
	if (estimate.size() == originals.size()) {
		DataCopy dc(output, slice_size);
		for (auto io = estimate.begin(); io != estimate.end(); ++io)
			::mia::accumulate(dc, **io);
	}else {
		copy(input.begin(), input.end(), output.begin());
		estimate.resize(originals.size());
	}

	// run optimization of ground truth
	CMinimizer::PProblem gtp(new GroundTruthProblem(m_alpha, m_beta, originals[0]->get_size(),
								originals.size(), input, correlation));

	// remark, maybe one should test other optimizers 
	auto  minimizer = CMinimizerPluginHandler::instance().produce("gsl:opt=bfgs2,step=0.1"); 
	minimizer->set_problem(gtp);

	auto min_status = minimizer->run(output);
	if (min_status != CMinimizer::success) 
		cvwarn() << "C2DGroundTruthEvaluator: optimization did not converge\n"; 

	// copy back the result
	auto igt = output.begin();
	for(size_t i = 0; i < originals.size(); ++i) {
		C2DFImage *image = new C2DFImage(originals[0]->get_size());
		copy(igt, igt + slice_size, image->begin());
		estimate[i] = P2DImage(image);
		igt += slice_size;
	}
}

DataCopy::DataCopy(CDoubleVector& target, size_t slice_size):
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
