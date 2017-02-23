/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <mia/3d/combiner/ops.hh>
#include <mia/3d/imageio.hh>

NS_BEGIN(Combiner3d)

using namespace mia; 
using namespace std; 


template <typename BO>
template <typename T, typename S>
mia::P3DImage T3DImageCombiner<BO>::operator () ( const T3DImage<T>& a, const T3DImage<S>& b) const
{
	// there's got to be a better way ...
	BO bo; 
	typedef decltype(bo(a[0], b[0])) R; 
			
	auto  r = new T3DImage<R>(a.get_size(), a); 
	
	P3DImage result(r);

	transform(a.begin(), a.end(), b.begin(), r->begin(), bo); 
        return result; 
}

template <typename BO>
mia::P3DImage  T3DImageCombiner<BO>::do_combine( const C3DImage& a, const C3DImage& b) const
{
	if (a.get_size() != b.get_size()) {
		throw create_exception<invalid_argument>("C3DAddImageCombiner: input images have different size: ", 
					       a.get_size(), " vs. ", b.get_size());
	}
	return mia::filter(*this, a, b); 
}

template class T3DImageCombiner<CombineAdd>; 
template class T3DImageCombiner<CombineSub>; 
template class T3DImageCombiner<CombineTimes>; 
template class T3DImageCombiner<CombineDiv>; 
template class T3DImageCombiner<CombineAbsDiff>; 

NS_END

NS_MIA_BEGIN
using namespace Combiner3d; 

extern const char pl_add[] = "add"; 
extern const char pl_sub[] = "sub"; 
extern const char pl_mul[] = "mul"; 
extern const char pl_div[] = "div"; 
extern const char pl_absdiff[] = "absdiff"; 


template class  T3DImageCombinerPlugin<T3DImageCombiner<CombineAdd>, pl_add>; 
template class  T3DImageCombinerPlugin<T3DImageCombiner<CombineSub>, pl_sub>; 
template class  T3DImageCombinerPlugin<T3DImageCombiner<CombineTimes>, pl_mul>;
template class  T3DImageCombinerPlugin<T3DImageCombiner<CombineDiv>, pl_div>; 
template class  T3DImageCombinerPlugin<T3DImageCombiner<CombineAbsDiff>, pl_absdiff>; 

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *retval = new T3DImageCombinerPlugin<T3DImageCombiner<CombineAdd>, pl_add>();
	retval->append_interface(new T3DImageCombinerPlugin<T3DImageCombiner<CombineSub>, pl_sub>()); 
	retval->append_interface(new T3DImageCombinerPlugin<T3DImageCombiner<CombineTimes>, pl_mul>()); 
	retval->append_interface(new T3DImageCombinerPlugin<T3DImageCombiner<CombineDiv>, pl_div>()); 
	retval->append_interface(new T3DImageCombinerPlugin<T3DImageCombiner<CombineAbsDiff>, pl_absdiff>); 
	return retval;  
}

NS_MIA_END
