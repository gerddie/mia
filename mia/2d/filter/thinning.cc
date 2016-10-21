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

#include <mia/2d/filter/thinning.hh>

NS_BEGIN(thinning_2dimage_filter) 
using namespace mia;
using namespace std;

C2DThinningImageFilter::C2DThinningImageFilter(int max_iterations):
	C2D2MaskMorphImageFilter(max_iterations)
{

	C2DMorphShape s1; 
	s1.add_pixel(-1, -1, false); 
	s1.add_pixel( 0, -1, false); 
	s1.add_pixel( 1, -1, false); 
	s1.add_pixel( 0,  0, true); 
	s1.add_pixel(-1,  1, true); 
	s1.add_pixel( 0,  1, true); 
	s1.add_pixel( 1,  1, true); 

	C2DMorphShape s2; 
	s2.add_pixel( 0, -1, false); 
	s2.add_pixel( 1, -1, false); 
	s2.add_pixel( 1,  0, false); 
	s2.add_pixel( 0,  0, true); 
	s2.add_pixel(-1,  0, true); 
	s2.add_pixel(-1,  1, true); 
	s2.add_pixel( 0,  1, true); 

	set_shapes(s1,s2); 
	
}

C2DPruningImageFilter::C2DPruningImageFilter(int max_iterations):
	C2D2MaskMorphImageFilter(max_iterations)
{

	cvdebug() << "Initialize pruning with " << max_iterations << " iterations\n"; 

	C2DMorphShape s1; 
	s1.add_pixel(-1, -1, false); 
	s1.add_pixel( 0, -1, false); 
	s1.add_pixel( 1, -1, false); 
	s1.add_pixel(-1,  0, false); 
	s1.add_pixel( 0,  0, true ); 
	s1.add_pixel( 1,  0, false); 
	s1.add_pixel(-1,  1, false); 

	C2DMorphShape s2; 
	s2.add_pixel(-1, -1, false); 
	s2.add_pixel( 0, -1, false); 
	s2.add_pixel( 1, -1, false); 

	s2.add_pixel(-1,  0, false); 
	s2.add_pixel( 0,  0, true ); 
	s2.add_pixel( 1,  0, false); 

	s2.add_pixel( 1,  1, false); 

	set_shapes(s1,s2); 
	
}


C2D2MaskMorphImageFilter::C2D2MaskMorphImageFilter(int max_iterations):
	m_shape(8), 
	m_max_iterations(max_iterations)
{
}

void C2D2MaskMorphImageFilter::set_shapes(const mia::C2DMorphShape& s1, const mia::C2DMorphShape& s2)
{
	m_shape[0] = s1; 
	m_shape[1] = s2;
	
	for(int i = 0; i < 3; ++i) {
		m_shape[2*i + 2] = m_shape[2*i].rotate_by_90(); 
		m_shape[2*i + 3] = m_shape[2*i+1].rotate_by_90(); 
	}
}


template <typename Image> 
struct __dispatch_thinning{
	static P2DImage apply(const Image& , const vector<C2DMorphShape>& , int ) {
		throw invalid_argument("Thinning can only be applied to binary images"); 
	}
}; 

template <> 
struct __dispatch_thinning<C2DBitImage> {
	
	static size_t run_pass(C2DBitImage& target, const C2DBitImage& source, 
			       const vector<C2DMorphShape>& shape) {
		size_t changed_pixels = 0; 
		C2DBitImage tmp(source.get_size()); 

		changed_pixels += morph_thinning_2d(target, source, shape[0]);
		cvdebug() << "0: changed_pixels = " <<changed_pixels << "\n"; 
		for (int i = 1; i < 8; ++i) {
			copy(target.begin(), target.end(), tmp.begin()); 
			changed_pixels += morph_thinning_2d(target, tmp, shape[i]);
			cvdebug() << i << ": changed_pixels = " <<changed_pixels << "\n"; 
		}
		
		return changed_pixels; 
	}

	static P2DImage apply(const C2DBitImage& image, const vector<C2DMorphShape>& shapes, int m_max_iterations) {
		C2DBitImage temp1(image); 
		C2DBitImage *temp2 = new C2DBitImage(image.get_size(), image); 

		int i = 0; 
		while ((!m_max_iterations  || i < m_max_iterations) && run_pass(*temp2, temp1, shapes)) {
			++i; 
			copy(temp2->begin(), temp2->end(), temp1.begin()); 
			cvdebug() << "run pass " << i << " of " << m_max_iterations << "\n"; 
		}
		return P2DImage(temp2); 
	}
}; 

template <typename T>
typename C2DFilter::result_type C2D2MaskMorphImageFilter::operator () (const T2DImage<T>& result)const
{
	return __dispatch_thinning<T2DImage<T>>::apply(result, m_shape, m_max_iterations); 

}

P2DImage C2D2MaskMorphImageFilter::do_filter(const C2DImage& src) const
{
	return mia::filter(*this, src); 
}


C2DThinningFilterFactory::C2DThinningFilterFactory():
	C2DFilterPlugin("thinning"),
	m_max_iterations(0)
{
	add_parameter("iter", make_ci_param(m_max_iterations, 1, 1000000, false,  
					    "Number of iterations to run, 0=until convergence"));
}

mia::C2DFilter *C2DThinningFilterFactory::do_create()const
{
	return new C2DThinningImageFilter(m_max_iterations);
}

const std::string C2DThinningFilterFactory::do_get_descr()const
{
	return "Morphological thinning. Thinning until convergence will result in a 8-connected skeleton"; 
}


C2DPruningFilterFactory::C2DPruningFilterFactory():
	C2DFilterPlugin("pruning"),
	m_max_iterations(0)
{
	add_parameter("iter", make_ci_param(m_max_iterations, 1, 1000000, false,  
					    "Number of iterations to run, 0=until convergence"));
}

mia::C2DFilter *C2DPruningFilterFactory::do_create()const
{
	return new C2DPruningImageFilter(m_max_iterations);
}

const std::string C2DPruningFilterFactory::do_get_descr()const
{
	return "Morphological pruning. Pruning until convergence will erase all pixels but closed loops.";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	auto result = new C2DThinningFilterFactory(); 
	result->append_interface(new C2DPruningFilterFactory()); 
	return result; 
}


NS_END

