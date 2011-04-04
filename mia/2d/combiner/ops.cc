/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2011
 *
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <boost/lambda/lambda.hpp>
#include <mia/2d/combiner/ops.hh>

NS_BEGIN(Combiner2d)

using boost::lambda::_1; 
using boost::lambda::_2; 



using namespace mia; 

template <typename BO>
template <typename T, typename S>
P2DImage T2DImageCombiner<BO>::operator () ( const T2DImage<T>& a, const T2DImage<S>& b) const
{
	// there's got to be a better way ...
	typedef decltype(a[0] + b[0]) R; 
			
	auto  r = new T2DImage<R>(a.get_size()); 
	
	P2DImage result(r);
	BO bo; 
	transform(a.begin(), a.end(), b.begin(), r->begin(), bo); 
	return result; 
}

template <typename BO>
P2DImage T2DImageCombiner<BO>::do_combine( const C2DImage& a, const C2DImage& b) const
{
	if (a.get_size() != b.get_size()) {
		THROW(invalid_argument, "C2DAddImageCombiner: input images have different size: " 
		      << a.get_size() << " vs. "<< b.get_size() << "\n"); 
	}
	return mia::filter(*this, a, b); 
}


template <typename Combiner, const char * const name> 
T2DImageCombinerPlugin<Combiner, name>::T2DImageCombinerPlugin():
	C2DImageCombinerPlugin(name) 
{
}
	
template <typename Combiner, const char * const name> 
C2DImageCombinerPlugin::ProductPtr 
T2DImageCombinerPlugin<Combiner, name>::do_create()const
{
	return C2DImageCombinerPlugin::ProductPtr (new Combiner); 
}

template <typename Combiner, const char name[]> 
const std::string T2DImageCombinerPlugin<Combiner, name>::do_get_descr() const
{
	return string("Imgage combiner '") +  string(name) + string("'"); 
}

template class  T2DImageCombiner<BinaryAdd>; 
template class  T2DImageCombiner<BinarySub>; 
template class  T2DImageCombiner<BinaryTimes>; 
template class  T2DImageCombiner<BinaryDiv>; 

extern const char pl_add[4] = "add"; 
extern const char pl_sub[4] = "sub"; 
extern const char pl_mul[4] = "mul"; 
extern const char pl_div[4] = "div"; 


template class  T2DImageCombinerPlugin<T2DImageCombiner<BinaryAdd>, pl_add>; 
template class  T2DImageCombinerPlugin<T2DImageCombiner<BinarySub>, pl_sub>; 
template class  T2DImageCombinerPlugin<T2DImageCombiner<BinaryTimes>, pl_mul>;
template class  T2DImageCombinerPlugin<T2DImageCombiner<BinaryDiv>, pl_div>; 

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *retval = new T2DImageCombinerPlugin<T2DImageCombiner<BinaryAdd>, pl_add>();
	retval->append_interface(new T2DImageCombinerPlugin<T2DImageCombiner<BinarySub>, pl_sub>()); 
	retval->append_interface(new T2DImageCombinerPlugin<T2DImageCombiner<BinaryTimes>, pl_mul>()); 
	retval->append_interface(new T2DImageCombinerPlugin<T2DImageCombiner<BinaryDiv>, pl_div>()); 
	return retval;  
}

NS_END
