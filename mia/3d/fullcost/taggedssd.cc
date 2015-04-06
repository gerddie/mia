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

#include <numeric>

#include <mia/core/filter.hh>
#include <mia/3d/filter.hh>
#include <mia/3d/fullcost/taggedssd.hh>

NS_BEGIN(taggedssd_3d)
NS_MIA_USE
using namespace std; 

C3DTaggedSSDCost::C3DTaggedSSDCost(const std::string& src_x, 
				   const std::string& ref_x, 
				   const std::string& src_y, 
				   const std::string& ref_y, 
				   const std::string& src_z, 
				   const std::string& ref_z,
				   double weight):
	C3DFullCost(weight)
	
{
	auto& imgio = C3DImageIOPluginHandler::instance();  

	m_src_key[0] = imgio.load_to_pool(src_x);
	m_src_key[1] = imgio.load_to_pool(src_y);
	m_src_key[2] = imgio.load_to_pool(src_z);
	
	m_ref_key[0] = imgio.load_to_pool(ref_x);
	m_ref_key[1] = imgio.load_to_pool(ref_y);
	m_ref_key[2] = imgio.load_to_pool(ref_z);

	add(property_gradient);
}

inline double sqd(double x, double y) {
	double h = x - y; 
	return h * h; 
}

struct FTaggedSSDAccumulatorX : public TFilter<double> {
	FTaggedSSDAccumulatorX(C3DFVectorfield& force, float scale):
		m_force(force), m_scale(scale) {} 
	
	template <typename T, typename S> 
	double operator () ( const T3DImage<T>& src, const T3DImage<S>& ref) {
		assert(src.size() == ref.size()); 
		
		double result = 0.0; 
		auto is = src.begin(); 
		auto ir = ref.begin(); 
		auto iforce = m_force.begin() + 1; 
		for (size_t z = 0; z < src.get_size().z; ++z) 
			for (size_t y = 0; y < src.get_size().y; ++y) {
				result += sqd(*is++, *ir++); 
				for (size_t x = 1; x < src.get_size().x - 1; ++x, is++, ir++, ++iforce) {
					double delta = *is - *ir; 
					iforce->x = m_scale * delta * (is[1] - is[-1]); 
					result += delta * delta; 
				}
				result += sqd(*is++, *ir++); 
				iforce += 2; 
			} 
		return result; 
	}; 
private: 
	C3DFVectorfield& m_force; 
	float m_scale; 
}; 


struct FTaggedSSDAccumulatorY : public TFilter<double> {
	FTaggedSSDAccumulatorY(C3DFVectorfield& force, float scale):
		m_force(force), m_scale(scale) {} 
	
	template <typename T, typename S> 
	double operator () ( const T3DImage<T>& src, const T3DImage<S>& ref) {
		assert(src.size() == ref.size()); 
		double result = 0.0; 
		
		int dx = src.get_size().x; 
		auto is = src.begin(); 
		auto ir = ref.begin(); 
		auto iforce = m_force.begin(); 
		for (size_t z = 0; z < src.get_size().z; ++z) {
			for (size_t x = 0; x < src.get_size().x; ++x, ++is, ++ir, ++iforce) {
				result += sqd(*is, *ir); 
			}
			for (size_t y = 1; y < src.get_size().y - 1; ++y) {
				for (size_t x = 0; x < src.get_size().x; ++x, is++, ir++, ++iforce) {
					double delta = *is - *ir; 
					iforce->y = m_scale * delta * (is[dx] - is[-dx]); 
					result += delta * delta; 
				}
			}
			for (size_t x = 0; x < src.get_size().x; ++x, ++is, ++ir, ++iforce) {
				result += sqd(*is, *ir); 
			}
		}
		return result; 
	}; 
private: 
	C3DFVectorfield& m_force;
	float m_scale; 
}; 


struct FTaggedSSDAccumulatorZ : public TFilter<double> {
	FTaggedSSDAccumulatorZ(C3DFVectorfield& force, float scale):
		m_force(force), m_scale(scale) {} 
	
	template <typename T, typename S> 
	double operator () ( const T3DImage<T>& src, const T3DImage<S>& ref) {
		assert(src.size() == ref.size()); 
		
		double result = 0.0; 
		int dxy = src.get_size().x * src.get_size().y; 
		auto is = src.begin(); 
		auto ir = ref.begin(); 
		auto iforce = m_force.begin(); 
		
		for (size_t y = 0; y < src.get_size().y; ++y) {
			for (size_t x = 0; x < src.get_size().x; ++x, ++is, ++ir, ++iforce) {
				result += sqd(*is, *ir); 
			}
		}

		for (size_t z = 1; z < src.get_size().z - 1; ++z) {
			for (size_t y = 0; y < src.get_size().y; ++y) {
				for (size_t x = 0; x < src.get_size().x; ++x, is++, ir++, ++iforce) {
					double delta = *is - *ir; 
					iforce->z = m_scale * delta * (is[dxy] - is[-dxy]); 
					result += delta * delta; 
				}
			}
		}
		for (size_t y = 0; y < src.get_size().y; ++y) {
			for (size_t x = 0; x < src.get_size().x; ++x, ++is, ++ir, ++iforce) {
				result += sqd(*is, *ir); 
			}
		}
		return result; 
	}; 
private: 
	C3DFVectorfield& m_force; 
	float m_scale; 
}; 


double C3DTaggedSSDCost::do_evaluate(const C3DTransformation& t, CDoubleVector& gradient) const
{
	C3DFVectorfield force(get_current_size()); 
	double value = 0.0; 
	float scale = 0.5f / m_ref_scaled[0]->size(); 
	{
		auto temp = t(*m_src_scaled[0]); 
		FTaggedSSDAccumulatorX acc(force, scale);
		double r = mia::accumulate(acc, *temp, *m_ref_scaled[0]);
		value += r; 
	}
	{
		auto temp = t(*m_src_scaled[1]); 
		FTaggedSSDAccumulatorY acc(force, scale);
		double r = mia::accumulate(acc, *temp, *m_ref_scaled[1]);
		value += r; 
	}
	{
		auto temp = t(*m_src_scaled[2]); 
		FTaggedSSDAccumulatorZ acc(force, scale);
		double r = mia::accumulate(acc, *temp, *m_ref_scaled[2]);
		value += r; 
	}
	
	t.translate(force, gradient); 
	return value / (6.0 * m_ref_scaled[0]->size()); 	
}

void C3DTaggedSSDCost::do_set_size()
{
	TRACE_FUNCTION; 
	assert(m_src[0]); 
	assert(m_src[1]); 
	assert(m_src[2]); 
	
	assert(m_ref[0]); 
	assert(m_ref[1]); 
	assert(m_ref[2]); 

	if (!m_src_scaled[0] || m_src_scaled[0]->get_size() != get_current_size() ||
	    !m_ref_scaled[0] || m_ref_scaled[0]->get_size() != get_current_size() ||
	    !m_src_scaled[1] || m_src_scaled[1]->get_size() != get_current_size() ||
	    !m_ref_scaled[1] || m_ref_scaled[1]->get_size() != get_current_size() ||
	    !m_src_scaled[2] || m_src_scaled[2]->get_size() != get_current_size() ||
	    !m_ref_scaled[2] || m_ref_scaled[2]->get_size() != get_current_size()) {
		
		if (m_src[0]->get_size() == get_current_size()) {
			for (int i = 0; i < 3; ++i) {
				m_src_scaled[i] = m_src[i]; 
				m_ref_scaled[i] = m_ref[i];
			}
		}else{
			stringstream filter_descr; 
			filter_descr << "scale:s=[" << get_current_size()<<"]"; 
			auto scaler = C3DFilterPluginHandler::instance().produce(filter_descr.str()); 
			assert(scaler); 
			cvdebug() << "C3DImageFullCost:scale images to " << get_current_size() << 
				" using '" << filter_descr.str() << "'\n"; 
			for (int i = 0; i < 3; ++i) {
				m_src_scaled[i] = scaler->filter(*m_src[i]); 
				m_ref_scaled[i] = scaler->filter(*m_ref[i]);
			}
		}
	}
}

P3DImage C3DTaggedSSDCost::get_from_pool(const C3DImageDataKey& key)
{
	C3DImageIOPlugin::PData in_image_list = key.get();
		
	if (!in_image_list || in_image_list->empty())
		throw invalid_argument("C3DImageFullCost: no image available in data pool");

	return (*in_image_list)[0];

}

	

struct FTaggedSSDAccumulator :public TFilter<bool> {
	FTaggedSSDAccumulator():m_value(0.0){} 
	
	template <typename T, typename S> 
	bool operator () ( const T3DImage<T>& src, const T3DImage<S>& ref) {
		assert(src.size() == ref.size()); 
		m_value += std::inner_product (src.begin(), src.end(), ref.begin(), 0.0, 
					       [](double x, double y) { return x + y;}, 
					       [](double x, double y) -> double { double d = x - y; return d*d; }); 
		return true; 
	}; 
	double get_value() const {
		return m_value; 
	}
private: 
	double m_value; 
}; 

double C3DTaggedSSDCost::do_value(const C3DTransformation& t) const
{
	P3DImage temp[3]; 
	FTaggedSSDAccumulator acc;
	
	for (int i = 0; i < 3; ++i) {
		temp[i] = t(*m_src_scaled[i]); 
		mia::accumulate(acc, *temp[i], *m_ref_scaled[i]); 
	}

	return acc.get_value() / (6.0 * m_ref_scaled[0]->size()); 

}

double C3DTaggedSSDCost::do_value() const
{
	FTaggedSSDAccumulator acc;
	for (int i = 0; i < 3; ++i)
		mia::accumulate(acc, *m_src_scaled[i], *m_ref_scaled[i]); 
	return acc.get_value() / (6.0 * m_ref_scaled[0]->size()); 
}

struct FAccumulateMeanVariance: public TFilter<int>  {
	FAccumulateMeanVariance(); 
	
	template <typename T> 
	int operator() (const T3DImage<T>& image); 

	P3DFilter get_intensity_scaling_filter() const; 
private: 
	double sum; 
	double sum2; 
	double n; 
};
	
FAccumulateMeanVariance::FAccumulateMeanVariance():
	sum(0.0), 
	sum2(0.0), 
	n(0.0)
{
}

template <typename T> 
int FAccumulateMeanVariance::operator() (const T3DImage<T>& image)
{
	n += image.size(); 
	for (auto i = image.begin(); i != image.end(); ++i) {
		sum += *i; 
		sum2 += *i * *i; 
	}
	return 0; 
}

P3DFilter FAccumulateMeanVariance::get_intensity_scaling_filter() const
{
	double mean = sum / n; 
	double sigma = sqrt((sum2 - sum * sum / n) / (n - 1));

	// both images are of the same single color 
	if (sigma == 0.0) 
		return P3DFilter(); 

	// I want a conversion filter, that makes the images together zero mean 
	// and diversion 1
	stringstream filter_descr; 
	filter_descr << "convert:repn=float,map=linear,b=" << -mean/sigma << ",a=" << 1.0/sigma; 
	cvinfo() << "Will convert using the filter:" << filter_descr.str() << "\n"; 
	
	return C3DFilterPluginHandler::instance().produce(filter_descr.str()); 
}


void C3DTaggedSSDCost::do_reinit()
{
	TRACE_FUNCTION; 

	FAccumulateMeanVariance filter_creator; 
	//cvmsg() << "C3DImageFullCost: read " << m_src_key << " and " << m_ref_key << "\n"; 
	for (int i = 0; i < 3; ++i) {
		m_src[i] = get_from_pool(m_src_key[i]);
		m_ref[i] = get_from_pool(m_ref_key[i]);
		
		mia::accumulate(filter_creator, *m_src[i]); 
		mia::accumulate(filter_creator, *m_ref[i]); 

		m_src_scaled[i].reset(); 
		m_ref_scaled[i].reset(); 

		// the loading code should chack for proper loading 
		assert(m_src[i]); 
		assert(m_ref[i]); 
	
		if (m_src[i]->get_size() != m_ref[i]->get_size()) 
			throw runtime_error("C3DImageFullCost only works with images of equal size"); 
		
	
		if (m_src[i]->get_voxel_size() != m_ref[i]->get_voxel_size()) {
			cverr() << "C3DImageFullCost: src and reference image are of differnet pixel dimensions."
				<< "This code doesn't honour this and a proper scaling should be applied first."; 
		}
	}
	if (m_ref[0]->get_size() != m_ref[1]->get_size() ||
	    m_ref[0]->get_size() != m_ref[2]->get_size()) {
		throw invalid_argument("Input reference images are of different sizes"); 
	}

	// run intensity normalization to zero mean, variation one. 
	auto filter = filter_creator.get_intensity_scaling_filter(); 
	if (!filter)
		throw invalid_argument("Input images have constant intensity"); 

	for (int i = 0; i < 3; ++i) {
		m_src[i] = filter->filter(*m_src[i]); 
		m_ref[i] = filter->filter(*m_ref[i]); 
	}
}

bool C3DTaggedSSDCost::do_get_full_size(C3DBounds& size) const
{
	TRACE_FUNCTION; 
	assert(m_ref[0]); 
	if (size == C3DBounds::_0) {
		size = m_ref[0]->get_size(); 
		return true; 
	}else
		return 	size == m_ref[0]->get_size(); 

}


// plugin implementation 
class C3DTaggedSSDCostPlugin: public C3DFullCostPlugin {
public: 
	C3DTaggedSSDCostPlugin(); 
private: 
	C3DFullCost *do_create(float weight) const;
	const std::string do_get_descr() const;
	std::string m_srcx_name;
	std::string m_refx_name;
	std::string m_srcy_name;
	std::string m_refy_name;
	std::string m_srcz_name;
	std::string m_refz_name;
}; 

C3DTaggedSSDCostPlugin::C3DTaggedSSDCostPlugin():
C3DFullCostPlugin("taggedssd"), 
      m_srcx_name("srcx.@"), 
      m_refx_name("refx.@"), 
      m_srcy_name("srcy.@"), 
      m_refy_name("refy.@"),
      m_srcz_name("srcz.@"),
      m_refz_name("refz.@")
										      
{
	const auto& io = C3DImageIOPluginHandler::instance(); 
	add_parameter("srcx", new CStringParameter(m_srcx_name, CCmdOptionFlags::input, "Study image X-tag", &io));
	add_parameter("refx", new CStringParameter(m_refx_name, CCmdOptionFlags::input, "Reference image  X-tag", &io));
	add_parameter("srcy", new CStringParameter(m_srcy_name, CCmdOptionFlags::input, "Study image Y-tag", &io));
	add_parameter("refy", new CStringParameter(m_refy_name, CCmdOptionFlags::input, "Reference image  Y-tag", &io));
	add_parameter("srcz", new CStringParameter(m_srcz_name, CCmdOptionFlags::input, "Study image Z-tag", &io));
	add_parameter("refz", new CStringParameter(m_refz_name, CCmdOptionFlags::input, "Reference image  Z-tag", &io));
}

C3DFullCost *C3DTaggedSSDCostPlugin::do_create(float weight) const
{
	return new C3DTaggedSSDCost(m_srcx_name, m_refx_name, 
				    m_srcy_name, m_refy_name, 
				    m_srcz_name, m_refz_name, 
				    weight); 
}

const std::string C3DTaggedSSDCostPlugin::do_get_descr() const
{
	return "Evaluates the Sum of Squared Differences similarity measure by using three tagged image pairs. "
		"The cost function value is evaluated based on all image pairs, but the gradient is composed by composing "
		"its component based on the tag direction."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DTaggedSSDCostPlugin();
}



NS_END


