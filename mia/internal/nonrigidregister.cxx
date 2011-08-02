/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
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
 * along with this program; if not, write to theFree Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define VSTREAM_DOMAIN "NR-REG"

#include <boost/lambda/lambda.hpp>
#include <iomanip>

NS_MIA_BEGIN

template <int dim> 
struct TNonrigidRegisterImpl {
	typedef dimension_traits<dim> this_dim_traits;
	typedef typename this_dim_traits::PTransformation PTransformation; 
	typedef typename this_dim_traits::Size Size; 
	typedef typename this_dim_traits::Image Image; 
	typedef typename this_dim_traits::PImage PImage; 
	typedef typename this_dim_traits::PTransformationFactory PTransformationFactory; 
	typedef typename this_dim_traits::FullCostList FullCostList; 
	typedef typename this_dim_traits::Filter Filter; 
	typedef typename this_dim_traits::FilterPluginHandler FilterPluginHandler;

	TNonrigidRegisterImpl(FullCostList& costs, PMinimizer minimizer,
				PTransformationFactory transform_creator,
			      size_t mg_levels, int idx);

	PTransformation run(PImage src, PImage ref) const;
	PTransformation run() const;
private:

	FullCostList& m_costs;
	PMinimizer m_minimizer;
	PTransformationFactory m_transform_creator;
	size_t m_mg_levels; 
	int m_idx; 
};

template <int dim> 
class TNonrigRegGradientProblem: public CMinimizer::Problem {
public:
	typedef dimension_traits<dim> this_dim_traits;
	typedef typename this_dim_traits::Transformation Transformation; 
	typedef typename this_dim_traits::Size Size; 
	typedef typename this_dim_traits::Image Image; 
	typedef typename this_dim_traits::PImage PImage; 
	typedef typename this_dim_traits::PTransformationFactory PTransformationFactory; 
	typedef typename this_dim_traits::FullCostList FullCostList; 
	typedef typename this_dim_traits::Filter Filter; 
	typedef typename this_dim_traits::FilterPluginHandler FilterPluginHandler;


	TNonrigRegGradientProblem(const FullCostList& costs, Transformation& transf);

	void reset_counters(); 
	
	typedef shared_ptr<TNonrigRegGradientProblem<dim> > PNonrigRegGradientProblem; 
private:
	double  do_f(const CDoubleVector& x);
	void    do_df(const CDoubleVector& x, CDoubleVector&  g);
	double  do_fdf(const CDoubleVector& x, CDoubleVector&  g);

	double  evaluate_fdf(const CDoubleVector& x, CDoubleVector&  g);

	bool do_has(const char *property) const; 

	size_t do_size() const; 

	const FullCostList& m_costs; 
	Transformation& m_transf;
	size_t m_func_evals; 
	size_t m_grad_evals; 
	double m_start_cost; 
};

template <int dim> 
TNonrigidRegister<dim>::TNonrigidRegister(FullCostList& costs, PMinimizer minimizer,
					  PTransformationFactory transform_creation,
					  size_t mg_levels, int idx):
	impl(new TNonrigidRegisterImpl<dim>( costs, minimizer, transform_creation, mg_levels, idx))
{
}

template <int dim> 
TNonrigidRegister<dim>::~TNonrigidRegister()
{
	delete impl;
}

template <int dim> 
typename TNonrigidRegister<dim>::PTransformation 
TNonrigidRegister<dim>::run(PImage src, PImage ref) const
{
	return impl->run(src, ref);
}

template <int dim> 
typename TNonrigidRegister<dim>::PTransformation 
TNonrigidRegister<dim>::run() const
{
	return impl->run();
}


template <int dim> 
TNonrigidRegisterImpl<dim>::TNonrigidRegisterImpl(FullCostList& costs, PMinimizer minimizer,
						 PTransformationFactory transform_creation, size_t mg_levels, int idx):
	m_costs(costs),
	m_minimizer(minimizer),
	m_transform_creator(transform_creation), 
	m_mg_levels(mg_levels), 
	m_idx(idx)
{
}


/*
  This filter could be replaced by a histogram equalizing filter 
  or it should be moved outside the registration function 
  and considered what it is, a pre-processing step
*/

template <int dim> 
class FScaleFilterCreator: public TFilter<typename TNonrigidRegisterImpl<dim>::FilterPluginHandler::ProductPtr> {
	typedef typename TNonrigidRegisterImpl<dim>::FilterPluginHandler FilterPluginHandler;
public: 
	typedef typename TFilter<typename TNonrigidRegisterImpl<dim>::FilterPluginHandler::ProductPtr>::result_type result_type; 
	template <typename V, typename S>
	result_type operator ()(const V& a, const S& b) const {
		double sum = 0.0; 
		double sum2 = 0.0; 
		int n = 2 * a.size(); 

		auto ia = a.begin(); 
		
		for(auto ib = b.begin(); ia != a.end(); ++ia, ++ib) {
			sum += *ia + *ib; 
			sum2 += *ia * *ia + *ib * *ib;
		}
		
		double mean = sum / n; 
		double sigma = sqrt((sum2 - sum * sum / n) / (n - 1));

		// both images are of the same single color 
		if (sigma == 0.0) 
			return result_type(); 

		// I want a conversion filter, that makes the images together zero mean 
		// and diversion 1
		stringstream filter_descr; 
		filter_descr << "convert:repn=float,map=linear,b=" << -mean/sigma << ",a=" << 1.0/sigma; 
		cvinfo() << "Will convert using the filter:" << filter_descr.str() << "\n"; 
		
		return FilterPluginHandler::instance().produce(filter_descr.str()); 
		
	}; 
	
}; 

template <int dim> 
typename TNonrigidRegisterImpl<dim>::PTransformation 
TNonrigidRegisterImpl<dim>::run(PImage src, PImage ref) const
{
	assert(src);
	assert(ref);
	assert(src->get_size() == ref->get_size());

	PTransformation transform;

	// convert the images to float ans scale to range [-1,1]
	// this should be replaced by some kind of general pre-filter plug-in 
	FScaleFilterCreator<dim> fc; 
	auto tofloat_converter = ::mia::filter(fc, *src, *ref); 
	
	if (tofloat_converter) {
		src = tofloat_converter->filter(*src); 
		ref = tofloat_converter->filter(*ref); 
	}
	else // both images have only one value, and are, therefore, already registered
		return m_transform_creator->create(src->get_size());


	Size global_size = src->get_size();

	int shift = m_mg_levels;

	std::string src_name("src.@"); 
	std::string ref_name("ref.@"); 

	if (m_idx >= 0) {
		std::stringstream src_ss; 
		std::stringstream ref_ss; 

		src_ss << "src" << m_idx << ".@"; 
		ref_ss << "ref" << m_idx << ".@"; 
		src_name = src_ss.str(); 
		ref_name = ref_ss.str(); 
	}

	do {

		// this should be replaced by a per-dimension shift that honours a minimum size of the 
		// downscaled images -  this is especially important in 3D
		shift--;

		Size BlockSize; 
                BlockSize.fill(1 << shift);
		cvinfo() << "Blocksize = " << BlockSize  << "\n";

		stringstream downscale_descr;
		downscale_descr << "downscale:b=[" << BlockSize<<"]";
		auto downscaler = FilterPluginHandler::instance().produce(downscale_descr.str().c_str());

		PImage src_scaled = shift ? downscaler->filter(*src) : src;
		PImage ref_scaled = shift ? downscaler->filter(*ref) : ref;

		if (transform)
			transform = transform->upscale(src_scaled->get_size());
		else
			transform = m_transform_creator->create(src_scaled->get_size());

		cvinfo() << "register at " << src_scaled->get_size() << "\n";
		/**
		   This code is somewhat ugly, it stored the images in the internal buffer 
		   and then it forces the cost function to reload the images
		   However, currently the downscaling does not support a specific target size
		 */
		save_image(src_name, src_scaled);
		save_image(ref_name, ref_scaled);
		m_costs.reinit(); 
		
		// currently this call does nothing, however it should replace the three lines above 
		// and the cost function should handle the image scaling 

		m_costs.set_size(src_scaled->get_size()); 
		
		std::shared_ptr<TNonrigRegGradientProblem<dim> > 
			gp(new TNonrigRegGradientProblem<dim>( m_costs, *transform));
		
		m_minimizer->set_problem(gp);

		auto x = transform->get_parameters();
		cvinfo() << "Start Registration of " << x.size() <<  " parameters\n"; 
		m_minimizer->run(x);
		transform->set_parameters(x);
	
		// run the registration at refined splines 
		if (transform->refine()) {
			gp->reset_counters(); 
			m_minimizer->set_problem(gp);
			x = transform->get_parameters();
			cvinfo() << "Start Registration of " << x.size() <<  " parameters\n"; 
			m_minimizer->run(x);
			transform->set_parameters(x);
		}

	} while (shift); 
	return transform;
}


template <int dim> 
typename TNonrigidRegisterImpl<dim>::PTransformation 
TNonrigidRegisterImpl<dim>::run() const
{
	PTransformation transform;

	m_costs.reinit(); 
	Size global_size; 
	if (!m_costs.get_full_size(global_size))
		throw invalid_argument("Nonrigidregister: the given combination of cost functions don't "
				       "agree on the size of the registration problem"); 

	int shift = m_mg_levels;

	do {

		// this should be replaced by a per-dimension scale that honours a minimum size of the 
		// downscaled images -  this is especially important in 3D
		shift--;
		int scale_factor = 1 << shift; 
		Size local_size = global_size / scale_factor; 
		
		if (transform)
			transform = transform->upscale(local_size);
		else
			transform = m_transform_creator->create(local_size);

		cvinfo() << "register at " << local_size << "\n";
		m_costs.reinit(); 
		m_costs.set_size(local_size); 
		
		std::shared_ptr<TNonrigRegGradientProblem<dim> > 
			gp(new TNonrigRegGradientProblem<dim>( m_costs, *transform));
		
		m_minimizer->set_problem(gp);

		auto x = transform->get_parameters();
		cvinfo() << "Start Registration of " << x.size() <<  " parameters\n"; 
		m_minimizer->run(x);
		transform->set_parameters(x);
		
		// run the registration at refined splines 
		if (transform->refine()) {
			m_minimizer->set_problem(gp);
			x = transform->get_parameters();
			cvinfo() << "Start Registration of " << x.size() <<  " parameters\n"; 
			m_minimizer->run(x);
			transform->set_parameters(x);
		}

	} while (shift); 
	return transform;
}

template <int dim> 
TNonrigRegGradientProblem<dim>::TNonrigRegGradientProblem(const FullCostList& costs, Transformation& transf):
	m_costs(costs),
	m_transf(transf),
	m_func_evals(0),
	m_grad_evals(0), 
	m_start_cost(0.0)
{

}

template <int dim> 
void TNonrigRegGradientProblem<dim>::reset_counters()
{
	m_func_evals = m_grad_evals = 0; 
}

template <int dim> 
double  TNonrigRegGradientProblem<dim>::do_f(const CDoubleVector& x)
{
       

	m_transf.set_parameters(x);
	double result = m_costs.cost_value(m_transf);
	if (!m_func_evals && !m_grad_evals) 
		m_start_cost = result; 
	
	m_func_evals++; 
	cvinfo() << "Cost[fg="<<setw(4)<<m_grad_evals 
		<< ",fe="<<setw(4)<<m_func_evals<<"]=" 
		<< setw(20) << setprecision(12) << result 
		<< "ratio:" << setw(20) << setprecision(12) 
		<< result / m_start_cost <<   "\n"; 
	return result; 
}

template <int dim> 
void    TNonrigRegGradientProblem<dim>::do_df(const CDoubleVector& x, CDoubleVector&  g)
{
	this->evaluate_fdf(x,g); 
	m_grad_evals++; 
}

template <int dim> 
double  TNonrigRegGradientProblem<dim>::do_fdf(const CDoubleVector& x, CDoubleVector&  g)
{
	const double result = this->evaluate_fdf(x,g); 
	m_grad_evals++; 
	m_func_evals++; 
	return result; 
}

template <int dim> 
double  TNonrigRegGradientProblem<dim>::evaluate_fdf(const CDoubleVector& x, CDoubleVector&  g)
{
	m_transf.set_parameters(x);
	fill(g.begin(), g.end(), 0.0); 
	double result = m_costs.evaluate(m_transf, g);

	if (!m_func_evals && !m_grad_evals) 
		m_start_cost = result; 



	cvinfo() << "Cost[fg="<<setw(4)<<m_grad_evals 
		<< ",fe="<<setw(4)<<m_func_evals<<"]= with " 
		<< x.size() << " parameters= " 
		<< setw(20) << setprecision(12) << result 
		<< " ratio:" << setw(20) << setprecision(12) << result / m_start_cost <<  "\n"; 
	return result; 
}

template <int dim> 
bool TNonrigRegGradientProblem<dim>::do_has(const char *property) const
{
	return m_costs.has(property); 
}

template <int dim> 
size_t TNonrigRegGradientProblem<dim>::do_size() const
{
	return m_transf.degrees_of_freedom(); 
}

NS_MIA_END
