/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <mia/3d/maskedcost/lncc.hh>

#include <mia/core/threadedmsg.hh>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>


NS_BEGIN(NS)

using namespace mia; 
using std::vector; 
using std::pair; 
using std::make_pair; 

CLNCC3DImageCost::CLNCC3DImageCost(int hw):
m_hwidth(hw)
{
}

inline pair<C3DBounds, C3DBounds> prepare_range(const C3DBounds& size, int cx, int cy, int cz, int hw) 
{
	int zb = cz - hw;
	if (zb < 0) zb = 0; 
	unsigned ze = cz + hw + 1; 
	if (ze > size.z) ze = size.z; 
	
	int yb = cy - hw;
	if (yb < 0) yb = 0; 
	unsigned ye = cy + hw + 1; 
	if (ye > size.y) ye = size.y; 
	
	int xb = cx - hw;
	if (xb < 0) xb = 0; 
	unsigned xe = cx + hw + 1; 
	if (xe > size.x) xe = size.x; 
	
	return make_pair(C3DBounds(xb,yb,zb), C3DBounds(xe,ye,ze)); 
}



class FEvalCost : public TFilter<float> {
	int m_hw;
	const C3DBitImage& m_mask; 
public:
	FEvalCost(int hw, const C3DBitImage& mask):
		m_hw(hw), 
		m_mask(mask)
		{}
	
	template <typename T, typename R> 
	float operator () ( const T& mov, const R& ref) const {
		auto evaluate_local_cost = [this, &mov, &ref](const tbb::blocked_range<size_t>& range, const pair<float, int>& result) -> pair<float, int> {
			CThreadMsgStream msks; 
			float lresult = 0.0; 
			int count = 0; 
			const int max_length = 2 * m_hw +1; 
			vector<float> a_buffer( max_length * max_length * max_length); 
			vector<float> b_buffer( max_length * max_length * max_length); 
			
			for (auto z = range.begin(); z != range.end(); ++z) {
				auto imask  = m_mask.begin_at(0,0,z);
			
				for (size_t y = 0; y < mov.get_size().y; ++y)
					for (size_t x = 0; x < mov.get_size().x; ++x, ++imask) {
						if (!*imask) 
							continue; 
						
						auto c_block = prepare_range(mov.get_size(), x, y, z, m_hw); 
						auto ia = mov.begin_range(c_block.first,c_block.second); 
						auto ea = mov.end_range(c_block.first,c_block.second); 
						auto ib = ref.begin_range(c_block.first,c_block.second); 
						auto im = m_mask.begin_range(c_block.first,c_block.second); 
						

						float suma = 0.0; 
						float sumb = 0.0; 
						float suma2 = 0.0; 
						float sumb2 = 0.0; 
						float sumab = 0.0; 
						long n = 0; 
						
						// make a local copy 
						while (ia != ea) {
							if (*im) {
								a_buffer[n] = *ia; 
								b_buffer[n] = *ib; 
								suma += *ia;
								sumb += *ib;
								++n;
							}
							++ia; ++ib; ++im; 
						}
						if (n > 1) {
							const float mean_a = suma/n; 
							const float mean_b = sumb/n;
							
							// strip mean and evaluate cross correlation 
							for (int i = 0; i < n; ++i) {
								const float a_ = a_buffer[i] - mean_a; 
								suma2 += a_ * a_; 
								const float b_ = b_buffer[i] - mean_b; 
								sumb2 += b_ * b_; 
								sumab += a_ * b_; 
							}
							float suma2_sumb2 = suma2 * sumb2;
							if (suma2_sumb2 > 1e-5) {
								lresult += sumab * sumab / suma2_sumb2; 
								++count;
								cvdebug() << "v=" << lresult
									  << ", c=" << count << ", n=" << n << "\n"; 
							}
						}
					}
			}
			return make_pair(result.first + lresult, result.second + count); 
		};
		
		pair<float,int> init{0, 0}; 
		auto r = parallel_reduce(tbb::blocked_range<size_t>(0, mov.get_size().z, 1), init, evaluate_local_cost, 
					 [](const pair<float,int>& x, const pair<float,int>& y){return make_pair(x.first + y.first, x.second + y.second);});	
		return r.second > 0 ? - r.first / r.second : 0.0; 
	}
}; 


double CLNCC3DImageCost::do_value(const Data& a, const Data& b, const Mask& m) const
{
	FEvalCost ecost(m_hwidth, m); 
	return mia::filter(ecost, a, b); 
}


class FEvalCostForce : public TFilter<float> {
	int m_hw;
	const C3DBitImage& m_mask; 
	C3DFVectorfield& m_force; 
public: 
	FEvalCostForce(int hw, const C3DBitImage& mask, C3DFVectorfield& force):
		m_hw(hw), 
		m_mask(mask), 
		m_force(force)
		{}
	
	template <typename T, typename R> 
	float operator () ( const T& mov, const R& ref) const {
		
		auto ag = get_gradient(mov); 

		auto evaluate_local_cost_force = [this, &mov, &ref, &ag](const tbb::blocked_range<size_t>& range, float result) {
			
			float lresult = 0.0; 
			int count = 0; 
			const int max_length = 2 * m_hw + 1;
			vector<float> a_buffer( max_length * max_length * max_length); 
			vector<float> b_buffer( max_length * max_length * max_length); 

			for (auto z = range.begin(); z != range.end(); ++z) {
                        
				auto iforce = m_force.begin_at(0,0,z);
				auto imask = m_mask.begin_at(0,0,z);
				auto ig = ag.begin_at(0,0,z);
				auto imov = mov.begin_at(0,0,z);
				auto iref = ref.begin_at(0,0,z);
                        
				for (size_t y = 0; y < mov.get_size().y; ++y)
					for (size_t x = 0; x < mov.get_size().x; ++x, ++iforce, ++imask, ++ig, ++iref, ++imov) {
						if (!*imask) 
							continue; 
                                        
						auto c_block = prepare_range(mov.get_size(), x, y, z, m_hw); 
						auto ia = mov.begin_range(c_block.first,c_block.second); 
						auto ea = mov.end_range(c_block.first,c_block.second); 
						auto ib = ref.begin_range(c_block.first,c_block.second); 
						auto im = m_mask.begin_range(c_block.first,c_block.second); 
                                        

						float suma = 0.0; 
						float sumb = 0.0; 
						float suma2 = 0.0; 
						float sumb2 = 0.0; 
						float sumab = 0.0; 
						long n = 0; 
						
						// make a local copy 
						while (ia != ea) {
							if (*im) {
								a_buffer[n] = *ia; 
								b_buffer[n] = *ib; 
								suma += *ia;
								sumb += *ib;
								++n;
							}
							++ia; ++ib; ++im; 
						}
						if (n > 1) {
							const float mean_a = suma/n; 
							const float mean_b = sumb/n;
							
							// strip mean and evaluate cross correlation 
							for (int i = 0; i < n; ++i) {
								const float a_ = a_buffer[i] - mean_a; 
								suma2 += a_ * a_; 
								const float b_ = b_buffer[i] - mean_b; 
								sumb2 += b_ * b_; 
								sumab += a_ * b_; 
							}
							
							float suma2_sumb2 = suma2 * sumb2;
							
							if (suma2_sumb2 > 1e-5) {
                                                        
								lresult += sumab * sumab / suma2_sumb2; 
								++count;
								const auto scale = static_cast<float>(2.0 * sumab / suma2_sumb2 * 
												      ( sumab / suma2 * *imov - *iref ));
								*iforce = scale * *ig; 
							}
						}
					}
			}
			if (count > 0) 
				return result - lresult / count; 
			else 
				return result; 
		};
		
		return parallel_reduce(tbb::blocked_range<size_t>(0, mov.get_size().z, 1), 0.0, 
				       evaluate_local_cost_force, [](double x, double y){return x+y;});
		
	}
	
};

double CLNCC3DImageCost::do_evaluate_force(const Data& a, const Data& b, const Mask& m, Force& force) const
{
	FEvalCostForce ecostforce(m_hwidth, m, force); 
	return mia::filter(ecostforce, a, b); 
}


void CLNCC3DImageCost::post_set_reference(const Data& ref)
{
}


CLNCC3DImageCostPlugin::CLNCC3DImageCostPlugin():
C3DMaskedImageCostPlugin("lncc"), 
	m_hw(5)
{
	this->add_parameter("w", new CUIntParameter(m_hw, 1, 256, false, 
						    "half width of the window used for evaluating the localized cross correlation")); 
}

C3DMaskedImageCost *CLNCC3DImageCostPlugin::do_create() const
{
	return new CLNCC3DImageCost(m_hw);
}

const std::string CLNCC3DImageCostPlugin::do_get_descr() const
{
	return "local normalized cross correlation with masking support."; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CLNCC3DImageCostPlugin();
}

NS_END
