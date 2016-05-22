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


#include <mia/core/probmap.hh>
#include <mia/core/sparse_histogram.hh>
#include <mia/core/factory.hh>

NS_MIA_BEGIN



class EXPORT_CORE CMeans {
public:
	typedef std::vector<double> DVector;
	typedef CSparseHistogram::Compressed SparseHistogram;
	typedef std::vector<std::pair<double, double>> NormalizedHistogram;

	class EXPORT_CORE SparseProbmap { 
	public: 
		typedef std::pair<unsigned short, DVector> value_type;  
		typedef std::vector<value_type> Map;

		SparseProbmap() = delete; 
		SparseProbmap (size_t size);
		SparseProbmap (const std::string& filename); 
		
		value_type& operator [](int i){
			return m_map[i]; 
		}

		const value_type& operator [](int i) const{
			return m_map[i]; 
		}

		Map::const_iterator begin() const {
			return m_map.begin(); 
		}
		Map::iterator begin() {
			return m_map.begin(); 
		}

		Map::const_iterator end() const {
			return m_map.end(); 
		}
		Map::iterator end() {
			return m_map.end(); 
		}
		
		bool save(const std::string& filename) const;

	   
		DVector get_fuzzy(double x) const; 

		size_t size() const {
			return m_map.size(); 
		}
	private: 
		Map m_map; 

	}; 
        

	class EXPORT_CORE Initializer : public CProductBase {
	public:
		typedef Initializer plugin_data;
		typedef Initializer plugin_type;
		static const char *data_descr;
		static const char *type_descr;
		virtual DVector run(const NormalizedHistogram& nh) const = 0; 
	}; 
	typedef std::shared_ptr<Initializer> PInitializer; 
	
	CMeans(double epsilon, PInitializer class_center_initializer);

	~CMeans();
	
	SparseProbmap run(const SparseHistogram& histogram,  DVector& class_centers) const;

	SparseProbmap run(const SparseHistogram& histogram,  DVector& class_centers, bool de_normalize_results) const;
	
private:
	PInitializer m_cci; 
	struct CMeansImpl *impl;
	
};

/**
   \brief evaluate the probabilities for a c-means classification with gain field 
   
   This function evaluates the per-pixel class probabilities for a c-means 
   classification with gain field correction. 

   With n classes the evalaution is done aoocrding to 

   \f[
   p_{k,i} := \left{\begin{array}{lcl}
   I_k < c_0 & & p_{k,0} = 1, p_{k,i} = 0 \forall i \in [1, n-1]
   c_j < I_k < c_{j+1} & & p_{k,j} = 
        \frac{(I_k - g_k * c_{j+1})^2}{(I_k - g_k * c_{j+1})^2 + (I_k - g_k * c_{j})^2} \\
   I_k > c_{n-1} & & p_{k,n} = 1,  p_{k,i} =  \forall i \in [0, n-2]
   \f]

   \tparam T input pixel type of the data to be classified 
   \tparam Field template of the data field type 
   \param[in] image image the classification is applied to 
   \param[in] gain multiplicative gain field 
   \param[in] class_centers 
   \param[out] pv probability fields containing the evaluated probabilities 
   
*/


template <typename T, template <class > typename  Field> 
void cmeans_evaluate_probabilities(const Field<T>& image, const Field<float>& gain,
				   const std::vector<double>& class_centers,
				   std::vector<Field<float>>& pv)
{
	assert(image.size() == gain.size());
	assert(class_centers.size() == pv.size());
#ifndef NDEBUG
	for (auto i: pv)
		assert(image.size() == i.size());
#endif 	
	
	auto ii = image.begin();
	auto ie = image.end();
	auto ig = gain.begin();
	typedef typename Field<float>::iterator prob_iterator; 

	std::vector<prob_iterator> ipv(pv.size());
	transform(pv.begin(), pv.end(), ipv.begin(), [](Field<float>& p){return p.begin();});

	std::vector<double> gain_class_centers(class_centers.size());
	
	while (ii != ie) {
		double x = *ii; 
		for(auto iipv: ipv)
			*iipv = 0.0;

		const double  vgain = *ig; 
		transform(class_centers.begin(), class_centers.end(), gain_class_centers.begin(),
			  [vgain](double x){return vgain * x;}); 
		
		if ( x < gain_class_centers[0]) {
			*ipv[0] = 1.0; 
		} else {
			unsigned j = 1;
			bool value_set = false; 
			while (!value_set && (j < class_centers.size()) ) {
				// between two centers
				if (x < gain_class_centers[j]) {

					double p0 = x - gain_class_centers[j-1];
					double p1 = x - gain_class_centers[j];
					double p02 = p0 * p0;
					double p12 = p1 * p1;
					double normalizer = 1.0/(p02 + p12); 
					*ipv[j] = p02  * normalizer;
					*ipv[j - 1] = p12  * normalizer;
					value_set = true;
				}
				++j; 
			}
			if (!value_set)
				*ipv[class_centers.size() - 1] = 1.0; 
		}
		++ii; ++ig;
		for (unsigned i = 0; i < class_centers.size(); ++i)
			++ipv[i];
	}
}

/**
   Evaluate the new clas centers from 
   \f[
   \sum_{k,i} (p_{i,k} I_k - g_k c_i)^2 \rightarrow \min
   \f]
   In order to avoid a ping-pong effect, the actual class center update is evaluated 
   according to 
   \f[
   c_i^{(t+1)} = \frac{1}{5} \left( \sum_{k} \frac{\p_{i,k}^2 g_k I_k}{ \p_{i,k}^2 g_k^2 } - c_i^{(t)}\right)
   \f]
   
   \tparam T input pixel type of the data to be classified 
   \tparam Field template of the data field type 
   \param[in] image image the classification is applied to 
   \param[in] gain multiplicative gain field 
   \param[in] pv probability fields 
   \param[in,out] class_centers 
   \returns sum of absolute change applied to the class centers 

*/

template <typename T, template <class> typename Field> 
double cmeans_update_class_centers(const Field<T>& image, const Field<float>& gain,
				 const std::vector<Field<float>>& pv, 
				 std::vector<double>& class_centers)
{
	double residuum = 0.0; 

	for (size_t i = 0; i < class_centers.size(); ++i) {
		double cc = class_centers[i]; 
		double sum_prob = 0.0; 
		double sum_weight = 0.0; 

		auto ie = image.end();
		auto ii = image.begin();
		auto ig = gain.begin();
		auto ip = pv[i].begin();

		while (ii != ie)  {
			if (*ip > 0.0) {
				auto v = *ip * *ip * *ig;
				sum_prob += v * *ig; 
				sum_weight += v * *ii;
			}
			++ii;
			++ig;
			++ip;
		}
			
		
		if (sum_prob  != 0.0) // move slowly in the direction of new center
			cc = sum_weight / sum_prob; 
		else {
			cvwarn() << "class[" << i << "] has no probable members, keeping old value:" << 
				sum_prob << ":" <<sum_weight <<"\n"; 
			
		}
		double delta = (cc - class_centers[i]) * 0.5; 
		residuum += delta * delta; 
		class_centers[i] +=  delta;
		
	}// end update class centers
	return sqrt(residuum); 
}


typedef TFactory<CMeans::Initializer> CMeansInitializerPlugin;

// the class that has only the size as a parameter
class EXPORT_CORE CMeansInitializerSizedPlugin : public CMeansInitializerPlugin {
public: 
        CMeansInitializerSizedPlugin(const char *name);
protected:
        size_t get_size_param() const; 
private:
        size_t m_size; 
       
}; 

/// @cond NEVER
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif 
extern template class EXPORT_CORE TPlugin<CMeans::Initializer, CMeans::Initializer>; 
extern template class EXPORT_CORE TFactory<CMeans::Initializer>;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif 
extern template class EXPORT_CORE TFactoryPluginHandler<TFactory<CMeans::Initializer>>;
extern template class EXPORT_CORE THandlerSingleton<TFactoryPluginHandler<TFactory<CMeans::Initializer>> >;
/// @endcond

template <>  const char * const TPluginHandler<TFactory<CMeans::Initializer>>::m_help;


typedef THandlerSingleton<TFactoryPluginHandler<CMeansInitializerPlugin> >CMeansInitializerPluginHandler;

/// @cond NEVER
FACTORY_TRAIT(CMeansInitializerPluginHandler); 
/// @endcond


NS_MIA_END

