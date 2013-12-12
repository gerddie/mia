/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#define VSTREAM_DOMAIN "NR-REG"

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

	void set_refinement_minimizer(PMinimizer minimizer); 
private:

	FullCostList& m_costs;
	PMinimizer m_minimizer;
	PMinimizer m_refinement_minimizer;
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
	
	typedef std::shared_ptr<TNonrigRegGradientProblem<dim> > PNonrigRegGradientProblem; 
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
void TNonrigidRegister<dim>::set_refinement_minimizer(PMinimizer minimizer)
{
	impl->set_refinement_minimizer(minimizer); 
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
		std::stringstream filter_descr; 
		filter_descr << "convert:repn=float,map=linear,b=" << -mean/sigma << ",a=" << 1.0/sigma; 
		cvinfo() << "Will convert using the filter:" << filter_descr.str() << "\n"; 
		
		return FilterPluginHandler::instance().produce(filter_descr.str()); 
		
	}; 
	
}; 

template <int dim> 
void TNonrigidRegisterImpl<dim>::set_refinement_minimizer(PMinimizer minimizer)
{
	m_refinement_minimizer = minimizer; 
}

template <int dim> 
typename TNonrigidRegisterImpl<dim>::PTransformation 
TNonrigidRegisterImpl<dim>::run(PImage src, PImage ref) const
{
	assert(src);
	assert(ref);
	assert(src->get_size() == ref->get_size());

	PTransformation transform;

	// convert the images to float ans scale to a mean=0, sigma=1 intensity distribution
	// this should be replaced by some kind of general pre-filter plug-in 
	FScaleFilterCreator<dim> fc; 
	auto tofloat_converter = ::mia::filter(fc, *src, *ref); 
	
	if (tofloat_converter) {
		src = tofloat_converter->filter(*src); 
		ref = tofloat_converter->filter(*ref); 
	}
	else // both images have only one value, and are, therefore, already registered
		return m_transform_creator->create(src->get_size());

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


	save_image(src_name, src);
	save_image(ref_name, ref);
	m_costs.reinit(); 
	
	Size global_size; 
	if (!m_costs.get_full_size(global_size))
		throw std::invalid_argument("Nonrigidregister: the given combination of cost functions doesn't"
					    "agree on the size of the registration problem"); 
	
	do {

		shift--;

		int scale_factor = 1 << shift; 
		Size local_size = global_size / scale_factor; 
		
		cvinfo() << "scale_factor = " << scale_factor << " from shift " << shift 
			 << ", global size = " << global_size << "\n"; 

		if (transform) {
			cvinfo() << "Upscale transform to " << local_size << " \n"; 
			transform = transform->upscale(local_size);
			cvinfo() << "done\n"; 
		}else{
			cvinfo() << "Create transform with size " << local_size << "\n"; 
			transform = m_transform_creator->create(local_size);
			cvinfo() << "done\n"; 
		}

		m_costs.set_size(local_size); 
		
		std::shared_ptr<TNonrigRegGradientProblem<dim> > 
			gp(new TNonrigRegGradientProblem<dim>( m_costs, *transform));
		
		m_minimizer->set_problem(gp);

		auto x = transform->get_parameters();
		cvmsg() << "Registration at " << local_size << " with " << x.size() <<  " parameters\n";

		m_minimizer->run(x);
		if (m_refinement_minimizer) {
			m_refinement_minimizer->set_problem(gp);
			m_refinement_minimizer->run(x);
		}
		cvmsg() << "\ndone\n";
		transform->set_parameters(x);
	
		// run the registration at refined splines 


		if (transform->refine()) {
			gp->reset_counters(); 
			m_minimizer->set_problem(gp);
			x = transform->get_parameters();
			cvmsg() << "Registration at " << local_size << " with " << x.size() <<  " parameters\n";
			m_minimizer->run(x);
			if (m_refinement_minimizer) {
				m_refinement_minimizer->set_problem(gp);
				m_refinement_minimizer->run(x);
			}
			cvmsg() << "\ndone\n";
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
		throw std::invalid_argument("Nonrigidregister: the given combination of cost functions doesn't"
					    "agree on the size of the registration problem"); 

	int shift = m_mg_levels;

	do {

		// this should be replaced by a per-dimension scale that honours a minimum size of the 
		// downscaled images -  this is especially important in 3D
		shift--;
		int scale_factor = 1 << shift; 
		Size local_size = global_size / scale_factor; 
		
		if (transform) {
			cvinfo() << "Upscale transform\n"; 
			transform = transform->upscale(local_size);
			cvinfo() << "done\n"; 
		}else{
			cvinfo() << "Create transform\n"; 
			transform = m_transform_creator->create(local_size);
			cvinfo() << "done\n"; 
		}

		m_costs.set_size(local_size); 
		
		std::shared_ptr<TNonrigRegGradientProblem<dim> > 
			gp(new TNonrigRegGradientProblem<dim>( m_costs, *transform));
		
		m_minimizer->set_problem(gp);

		auto x = transform->get_parameters();

		m_minimizer->run(x);
		cvmsg() << "\ndone\n";
		transform->set_parameters(x);
		
		// run the registration at refined splines 
		if (transform->refine()) {
			m_minimizer->set_problem(gp);
			x = transform->get_parameters();
			cvmsg() << "Registration at " << local_size << " with " << x.size() <<  " parameters\n";
			m_minimizer->run(x);
			cvmsg() << "\ndone\n";
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
	if (m_transf.has_energy_penalty())
		result += m_transf.get_energy_penalty();
	
	if (!m_func_evals && !m_grad_evals) 
		m_start_cost = result; 
	
	char endline = (cverb.get_level() < vstream::ml_message) ? '\n' : '\r'; 
	m_func_evals++; 
	cvmsg() << "Cost[fg=" << std::setw(4) << m_grad_evals 
		<< ",fe=" << std::setw(4) << m_func_evals<<"]=" 
		<< std::setw(20) << std::setprecision(12) << result 
		<< "ratio:" << std::setw(20) << std::setprecision(12) 
		<< result / m_start_cost  << endline; 
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
	std::fill(g.begin(), g.end(), 0.0); 

	double result = m_costs.evaluate(m_transf, g);

	if (m_transf.has_energy_penalty()) {
		CDoubleVector help(g.size()); 
		const double penalty = m_transf.get_energy_penalty_and_gradient(help);
		result += penalty; 
		std::transform(help.begin(), help.end(), g.begin(), g.begin(), 
			  [](double x, double y){return x+y;}); 
		cvinfo() << "Penalty=" << std::setw(20) << std::setprecision(12) << penalty << "\n"; 
	}

	if (!m_func_evals && !m_grad_evals) 
		m_start_cost = result; 

	char endline = (cverb.get_level() < vstream::ml_message) ? '\n' : '\r'; 

	cvmsg() << "Cost[fg="<<std::setw(4)<<m_grad_evals 
		<< ",fe="<<std::setw(4)<<m_func_evals<<"]= with " 
		<< std::setw(20) << std::setprecision(12) << result 
		<< " ratio:" << std::setw(20) << std::setprecision(12) << result / m_start_cost <<  endline; 
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
