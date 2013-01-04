/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */



/*
  type traits are required for pixel type based dispatching of the filter operator 
*/
#include <type_traits> 
#include <mia/2d/filter/simple.hh>


NS_BEGIN(simple_2dimage_filter)
NS_MIA_USE; 
/*
  The constructor takes all the necessary parameters. Usually this means that after construction, the 
  filter does not change anymore. As an exception to this rule, a data key may be provided that makes 
  it possible to load data from the internal memory that was created after construction of the filter. 
  This may be the case when specifying a long pipeline of filters. 
*/

C2DSimple::C2DSimple(double a):m_a(a)
{
}

/*
  This dispatcher structure takes care of the different handling of the input data based on whether  the pixel 
  type is floating point or not. 
*/
template <typename T, bool is_float> 
struct __dispatch_simple {
	static P2DImage apply(double a, const T2DImage<T>& data) {
		T2DImage<T> *result = new T2DImage<T>(data.get_size(), data); 
		transform(data.begin(), data.end(), result->begin(), [&a](T x){return a * x;}); 
		return P2DImage(result); 
	}
}; 

/*
  The filter doesn't support non-floating point valued pixles, hence throw. 
 */

template <typename T> 
struct __dispatch_simple<T, false> {
	static P2DImage apply(double /*a*/, const T2DImage<T>& /*data*/) {
		throw invalid_argument("2DFilter:simple only supported for floating point valued pixels"); 
	}
}; 

/*
  This templated operator does the actual work of the filter. The operator must support all pixel types 
  that are currently provided by MIA. If some input pixel type is nor supported, e.g. because the filter 
  just doesn't define an operation on the pixel type, then you should use (partial) template specialization 
  to dispatch the work based on the pixel type, and if an image with an unsupported type is provided as input then 
  an exception should be thrown.
  The result type is a P2Dimage. 
  
  In this example, images with non-floating point types are not supported. 
*/

template <typename T>
C2DSimple::result_type C2DSimple::operator () (const T2DImage<T>& data) const
{
	return __dispatch_simple<T, is_floating_point<T>::value>::apply(m_a,data); 
}


/*
  This method implements always the same code - redirect to dispatching the call 
  based on input pixel type.   
*/
P2DImage C2DSimple::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image); 
}


/*
  The plug-in constructor defines the name of the plug-in 'simple' and initialized the parameter(s) that 
  are accepted by this plug-in." 
 */

C2DSimpleImageFilterPlugin::C2DSimpleImageFilterPlugin():
	C2DFilterPlugin("simple")
{
	add_parameter("a", new CDoubleParameter(m_a, -std::numeric_limits<float>::max(), 
					       std::numeric_limits<float>::max(), false, 
					       "linear conversion parameter a")); 
}
	
/*
  The creator method returns the actual filter instance created with parameters given to the plug-in handler.
*/

C2DFilter *C2DSimpleImageFilterPlugin::do_create()const
{
	return new C2DSimple(m_a); 
}


/*
  This method gives a description of the plug-in that will be given in the automatically created documentation. 
*/
const std::string C2DSimpleImageFilterPlugin::do_get_descr()const
{
	return "This plug-in is a skeleton for a 2D image filter plug-in. "
		"The filter only accepts floating point pixel valued images and will throw "
		"and invalid_argument exception if the pixel type is not supported. "
		"With supported pixel types the input pixels will be multiplied by the constant factor a."; 
}


/*
  This function is the entry point to get the plug-in interface when loading the plug-in dynamically. 
  It needs to be extern "C" to avoid C++ name mangeling.  
*/
extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DSimpleImageFilterPlugin();
}




NS_END
