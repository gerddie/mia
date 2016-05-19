/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <algorithm>
#include <mia/core/export_handler.hh>

#include <mia/core/cmeans.hh>
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>

NS_MIA_BEGIN


using namespace std; 


struct CMeansImpl {

	typedef vector<pair<double, CMeans::DVector>> Probmap; 
	
	CMeansImpl(double epsilon);

	Probmap run(const CMeans::NormalizedHistogram& nh, CMeans::DVector& class_centers) const; 

private: 
	void evaluate_probabilities(const CMeans::DVector& classes, Probmap& pv) const; 
	double update_class_centers(CMeans::DVector& class_center,
				    const CMeans::NormalizedHistogram& nh, 
				    const Probmap& pv) const; 
	
	double m_epsilon;
}; 


CMeansImpl::CMeansImpl(double epsilon):
	m_epsilon(epsilon)
{
}

const char *CMeans::Initializer::data_descr = "1d";
const char *CMeans::Initializer::type_descr = "cmeans"; 


CMeans::CMeans(double epsilon, PInitializer class_center_initializer):
	m_cci(class_center_initializer)
{
	impl = new CMeansImpl(epsilon);
}

CMeans::~CMeans()
{
	delete impl; 
}


CMeans::SparseProbmap CMeans::run(const SparseHistogram& histogram,  DVector& class_centers) const
{
	return run(histogram, class_centers, true);
}

CMeans::SparseProbmap CMeans::run(const SparseHistogram& histogram,  DVector& class_centers, bool de_normalize_results) const
{
	FUNCTION_NOT_TESTED; 
		
	// prepare input data 
	NormalizedHistogram nhist(histogram.size());

	const double bin_shift = histogram[0].first < 0 ? histogram[0].first : 0;
	const double bin_scale = 1.0 / double(histogram[histogram.size() - 1].first - bin_shift);
	const double inv_bin_scale = double(histogram[histogram.size() - 1].first - bin_shift);

	cvinfo() << "Histogram conversion: shift = " <<  bin_shift << ", scale = " << bin_scale << "\n"; 
	
	size_t n = 0;
	for(auto h: histogram)
		n += h.second;

	if (n == 0) {
		throw create_exception<invalid_argument>("CMeans::run: the provided histogram was empty"); 
	}
	
	double normalizer = 1.0/n; 

	transform(histogram.begin(), histogram.end(), nhist.begin(),
		  [bin_shift, bin_scale, normalizer](const SparseHistogram::value_type& x) -> NormalizedHistogram::value_type {
			  return make_pair((x.first - bin_shift) * bin_scale, x.second * normalizer);  
		  });

	cvinfo() << "CMeans: normalized histogram range:[" <<  nhist[0].first
		 << ", " << nhist[nhist.size()-1].first << "]\n"; 

	class_centers = m_cci->run(nhist);
	cvmsg() << "Initial class centers =" << class_centers << "\n"; 
	auto internal_result = impl->run(nhist, class_centers);
	cvmsg() << "Finale class centers =" << class_centers << "\n"; 

	SparseProbmap result(internal_result.size()); 
	
	transform(internal_result.begin(), internal_result.end(), histogram.begin(), result.begin(),
		  [](const pair<double, CMeans::DVector>& p, const SparseHistogram::value_type& h) ->
		  SparseProbmap::value_type {
			  return make_pair(h.first, p.second); 
		  }); 

	// scale class centers to actual intensity range (if requested)
	if (de_normalize_results) {
		transform(class_centers.begin(), class_centers.end(), class_centers.begin(),
			  [inv_bin_scale, bin_shift](double x) {
				  return inv_bin_scale * x +  bin_shift; 
			  });
		cvmsg() << "Finale class rescaled =" << class_centers << "\n"; 
	}
	
	return result; 
}



template<>  const char * const 
TPluginHandler<TFactory<CMeans::Initializer>>::m_help = 
	"These plug-ins provide methods to initialize the cmeans classification.";


template class TPlugin<CMeans::Initializer, CMeans::Initializer>;
template class TFactory<CMeans::Initializer>;
template class TFactoryPluginHandler<CMeansInitializerPlugin>;
template class TPluginHandler<TFactory<CMeans::Initializer>>;
template class THandlerSingleton<TFactoryPluginHandler<CMeansInitializerPlugin>>;

void CMeansImpl::evaluate_probabilities(const CMeans::DVector& classes,
					Probmap& pv) const
{
        for (auto p = pv.begin(); p != pv.end(); ++p) {
                double x = p->first;
		fill(p->second.begin(), p->second.end(), 0.0); 
		
		if ( x < classes[0]) {
			p->second[0] = 1.0;
		} else {
			unsigned j = 1;
			bool value_set = false; 
			while (!value_set && (j < classes.size()) ) {
				// between two centers
				if (x < classes[j]) {
					double p0 = x - classes[j-1];
					double p1 = x - classes[j];
					double p02 = p0 * p0;
					double p12 = p1 * p1;
					double normalizer = 1.0/(p02 + p12); 
					
					p->second[j] = p02  * normalizer;
					p->second[j - 1] = p12  * normalizer;
					value_set = true;
				}
				++j; 
			}
			if (!value_set)
				p->second[classes.size() - 1] = 1.0; 
		}

	}
}

double CMeansImpl::update_class_centers(CMeans::DVector& class_center,
				     const CMeans::NormalizedHistogram& nh, 
				     const Probmap& pv)const
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
			cc = sum_weight / sum_prob; 
		else {
			cvwarn() << "class[" << i << "] has no probable members, keeping old value:" << 
				sum_prob << ":" <<sum_weight <<"\n"; 
			
		}
		double delta = (cc - class_center[i]) * 0.5; 
		residuum += delta * delta; 
		class_center[i] +=  delta; 
		
	}// end update class centers
	return sqrt(residuum); 
}


CMeansImpl::Probmap
CMeansImpl::run(const CMeans::NormalizedHistogram& nh, CMeans::DVector& class_centers) const
{
	int csize = class_centers.size(); 
	Probmap pv(nh.size()); 

	transform(nh.begin(), nh.end(), pv.begin(), [csize](const CMeans::NormalizedHistogram::value_type& x) {
			return make_pair(x.first, CMeans::DVector(csize));
		}); 
        
	bool cont = true;

	while (cont) {
		evaluate_probabilities(class_centers, pv);
		double residuum = update_class_centers(class_centers, nh, pv);
		cvmsg() << "Class centers: " << class_centers
			<<  ", res=" << residuum
			<< "\n";
		cont = residuum > m_epsilon; 
	};
	
	return pv; 
}


// the class that has only the size as a parameter
CMeansInitializerSizedPlugin::CMeansInitializerSizedPlugin(const char *name):
        CMeansInitializerPlugin(name)
{
        add_parameter("nc", make_lo_param(m_size, 2, true, "Number of classes to use for the fuzzy-cmeans classification")); 
}

size_t CMeansInitializerSizedPlugin::get_size_param() const
{
        return m_size; 
}


static const string map_signature("#sparse-probability-map");


CMeans::SparseProbmap::SparseProbmap(size_t size):m_map(size)
{
	if (!size)  {
		throw invalid_argument("CMeans::SparseProbmap must have at least one element"); 
	}
}


bool CMeans::SparseProbmap::save(const std::string& filename) const
{
	ostream* os = &std::cout; 

	unique_ptr<ofstream> ofs;
		
	if (filename != "-" ) {
		ofs.reset(new ofstream(filename)); 
		if (!ofs->good()) {
			throw create_exception<runtime_error>("CMeans::SparseProbmap::save:",
							      "Unable to open file ", filename,
							      " for saving\n");   
		}
		os = ofs.get(); 
	}
	
	*os << map_signature <<"\n";
	*os << m_map.size() << " " << m_map[0].second.size() << '\n';

	for (size_t i = 0; i < m_map.size(); ++i) {
		auto & row = m_map[i];
		*os << row.first << " : "; 
		
		for
			(auto d: row.second) {
			*os << d << " ";
		}
		*os <<'\n';
	}
	return os->good();

}

CMeans::SparseProbmap::SparseProbmap(const std::string& filename)
{
	istream* is = &std::cin; 

	unique_ptr<ifstream> ifs;
		
	if (filename != "-" ) {
		ifs.reset(new ifstream(filename)); 
		if (!ifs->good()) {
			throw create_exception<runtime_error>("CMeans::SparseProbmap::load:",
							      "Unable to open file ", filename,
							      " for reading\n");   
		}
		is = ifs.get(); 
	}
	
	string buf;
	*is >> buf;
	size_t hsize, nclasses;

	if (buf != map_signature) {
		throw create_exception<runtime_error>("CMeans::SparseProbmap::load: Input file '",
						      filename, "'is not a sparse probability map: signature:'",
						      buf, "'");
	}

	*is  >> hsize >> nclasses;

	if (!is->good())
		throw create_exception<runtime_error>("CMeans::SparseProbmap::load: error reading from input file '",
						      filename, "'");

	m_map.resize(hsize);

	char c; 
	for(auto& r: m_map) {
		*is >> r.first >> c;
		r.second.resize(nclasses); 
		for(auto& prob: r.second) {
			*is >> prob; 
		}
	}

	if (!is->good())
		throw create_exception<runtime_error>("CMeans::SparseProbmap::load: error reading from input file '",
						      filename, "'");
}

CMeans::DVector CMeans::SparseProbmap::get_fuzzy(double x) const
{
	if (x <= m_map[0].first)
		return m_map[0].second;
	if (x >= m_map[m_map.size() - 1].first)
		return m_map[m_map.size() - 1].second; 
	
	int idx = m_map.size() / 2;
	int next_step = idx / 4; 
	
	while (next_step > 0 ) {
		if (m_map[idx].first == x)
			return m_map[idx].second;  
		
		if ( x < m_map[idx].first) {
			idx -= next_step;
		}else{
			idx += next_step;
		}

		next_step /= 2; 
	}
	
	if (m_map[idx].first == x)
		return m_map[idx].second;  
	
	if ( x < m_map[idx].first) {
		--idx;
	}

	double base = m_map[idx].first; 
	double delta = m_map[idx+1].first - base;

	double f = (x - base) / delta;

	CMeans::DVector result( m_map[idx].second.size());

	transform(m_map[idx].second.begin(), m_map[idx].second.end(),
		  m_map[idx+1].second.begin(), result.begin(), 
		  [f](double l, double h) {
			  return f * (h-l) + l;
		  });
	
	return result; 
}

		


NS_MIA_END
