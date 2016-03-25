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


#include <mia/core/probmap.hh>
#include <mia/core/factory.hh>

NS_MIA_BEGIN



class EXPORT_CORE CMeans {
public:
	typedef std::vector<double> DVector;
	typedef std::vector<std::pair<int, unsigned long>> SparseHistogram;
	typedef std::vector<std::pair<double, double>> NormalizedHistogram;

	class EXPORT_CORE SparseProbmap { 
	public: 
		typedef std::pair<double, DVector> value_type;  
		typedef std::vector<std::pair<double, DVector>> Map;

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
	
	CMeans(double k, double epsilon, PInitializer class_center_initializer);

	~CMeans();
	
	SparseProbmap run(const SparseHistogram& histogram,  DVector& class_centers) const;
	
private:
	PInitializer m_cci; 
	struct CMeansImpl *impl;
	
};



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

extern template class EXPORT_CORE TPlugin<CMeans::Initializer, CMeans::Initializer>; 
extern template class EXPORT_CORE TFactory<CMeans::Initializer>;
extern template class EXPORT_CORE TFactoryPluginHandler<TFactory<CMeans::Initializer>>;
extern template class EXPORT_CORE THandlerSingleton<TFactoryPluginHandler<TFactory<CMeans::Initializer>> >;

template <>  const char * const TPluginHandler<TFactory<CMeans::Initializer>>::m_help;


typedef THandlerSingleton<TFactoryPluginHandler<CMeansInitializerPlugin> >CMeansInitializerPluginHandler;

/// @cond NEVER
FACTORY_TRAIT(CMeansInitializerPluginHandler); 
/// @endcond


NS_MIA_END

