/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#define VSTREAM_DOMAIN "CMEANS"

#include <mia/core/export_handler.hh>

#include <mia/core/cmeans.hh>

#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>

NS_MIA_BEGIN


using std::make_pair; 

struct CMeansImpl {

	CMeansImpl(double k, double epsilon);

	CMeans::SparseProbmap run(const CMeans::NormalizedHistogram& nh, CMeans::DVector& class_centers) const; 

private: 
	void evaluate_probabilities(const CMeans::DVector& classes, CMeans::SparseProbmap& pv, double k) const; 
	double update_class_centers(CMeans::DVector& class_center,
				    const CMeans::NormalizedHistogram& nh, 
				    const CMeans::SparseProbmap& pv) const; 
	
	double adjust_k(const CMeans::DVector& class_center,
			const CMeans::NormalizedHistogram& nh, 
			const CMeans::SparseProbmap& pv) const; 
		
	
	double m_k;
	double m_epsilon;
}; 


CMeansImpl::CMeansImpl(double k, double epsilon):
	m_k(k), 
	m_epsilon(epsilon)
{
}

const char *CMeans::Initializer::data_descr = "1d";
const char *CMeans::Initializer::type_descr = "cmeans"; 


CMeans::CMeans(double k, double epsilon, PInitializer class_center_initializer):
	m_cci(class_center_initializer)
{
	impl = new CMeansImpl(k, epsilon);
}

CMeans::~CMeans()
{
	delete impl; 
}


CMeans::SparseProbmap CMeans::run(const SparseHistogram& histogram,  DVector& class_centers) const
{
	// prepare input data 
	NormalizedHistogram nhist(histogram.size());
	
	const double bin_shift = histogram[0].first;
	const double bin_scale = 1.0 / double(histogram[histogram.size() - 1].first - bin_shift);

	size_t n = 0;
	for(auto h: histogram)
		n += h.second;

	double normalizer = 1.0/n; 

	transform(histogram.begin(), histogram.end(), nhist.begin(),
		  [bin_shift, bin_scale, normalizer](const SparseHistogram::value_type& x) -> NormalizedHistogram::value_type {
			  return make_pair((x.first - bin_shift) * bin_scale, x.second * normalizer);  
		  });
	

	class_centers = m_cci->run(nhist);
	cvmsg() << "Initial class centers =" << class_centers << "\n"; 
	SparseProbmap result = impl->run(nhist, class_centers);
	cvmsg() << "Finale class centers =" << class_centers << "\n"; 
	
	transform(result.begin(), result.end(), histogram.begin(), result.begin(),
		  [](const SparseProbmap::value_type& p, const SparseHistogram::value_type& h) -> SparseProbmap::value_type {
			  return make_pair(h.first, p.second); 
		  }); 

	
	
	return result; 
}

template<>  const char * const 
TPluginHandler<TFactory<CMeans::Initializer>>::m_help = 
	"These plug-ins provide methods to initialize the cmeans classification.";


template class TPlugin<CMeans::Initializer, CMeans::Initializer>;
template class TFactory<CMeans::Initializer>;
template class TFactoryPluginHandler<CMeansInitializerPlugin>;
template class EXPORT_CORE TPluginHandler<TFactory<CMeans::Initializer>>;
template class THandlerSingleton<TFactoryPluginHandler<CMeansInitializerPlugin>>;

void CMeansImpl::evaluate_probabilities(const CMeans::DVector& classes, CMeans::SparseProbmap& pv, double k) const
{
        for (auto p = pv.begin(); p != pv.end(); ++p) {
                double x = p->first;
                double sum = 0.0;

                for (size_t j = 0; j < classes.size(); ++j) {
                        double  val =  x - classes[j]; 
                        val = exp(- (val * val) / k);
                        p->second[j] = val; 
			sum += val;
                }
                
                assert(sum != 0.0);
                for (size_t j = 0; j < classes.size(); ++j)
			p->second[j] /= sum;

		cvdebug() << "Probs " << x << ":" << p->second << "\n"; 
        }
}

double CMeansImpl::update_class_centers(CMeans::DVector& class_center,
				     const CMeans::NormalizedHistogram& nh, 
				     const CMeans::SparseProbmap& pv)const
{
	double residuum = 0.0; 

	for (size_t i = 0; i < class_center.size(); ++i) {
		float cc = class_center[i]; 
		double sum_prob = 0.0; 
		double sum_weight = 0.0; 
			
		
		for (unsigned  k = 0; k < nh.size(); ++k) {
			auto prob = pv[k].second[i]; 
			if ( prob > 0.0) {
				auto v = prob * nh[k].second;
				sum_prob += v; 
				sum_weight += v * pv[k].first;
			}
		}
		
		if (sum_prob  != 0.0) // move slowly in the direction of new center
			cc += 0.1 * (sum_weight / sum_prob - cc); 
		else {
			cvwarn() << "class[" << i << "] has no probable members, keeping old value:" << 
				sum_prob << ":" <<sum_weight <<"\n"; 
			
		}
		double delta = cc - class_center[i]; 
		residuum += delta * delta; 
		class_center[i] =  cc; 
		
	}// end update class centers
	return sqrt(residuum); 
}

double CMeansImpl::adjust_k(const CMeans::DVector& class_center,
			     const CMeans::NormalizedHistogram& nh, 
			     const CMeans::SparseProbmap& pv)const
{
	cvwarn() << "CMeans: auto 'k' not implemented\n"; 
	return m_k; 
}


CMeans::SparseProbmap CMeansImpl::run(const CMeans::NormalizedHistogram& nh, CMeans::DVector& class_centers) const
{
	int csize = class_centers.size(); 
	CMeans::SparseProbmap pv(nh.size()); 

	transform(nh.begin(), nh.end(), pv.begin(), [csize](const CMeans::NormalizedHistogram::value_type& x) {
			return make_pair(x.first, CMeans::DVector(csize));
		}); 
        
	double k = m_k; 
	bool cont = true;

	// for now no k adjustment
	const bool auto_k = false; 
	
	while (cont) {
		evaluate_probabilities(class_centers, pv, k);
		double residuum = update_class_centers(class_centers, nh, pv);
		if (auto_k) {
			k = adjust_k(class_centers, nh, pv); 
		}
		cvmsg() << "Class centers: " << class_centers
			<<  ", res=" << residuum
			<<  ", k=" << k 
			<< "\n";
		cont = residuum > m_epsilon; 
	};
	return pv; 
}



NS_MIA_END
