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

#include <cmath>
#include <iostream>
#include <memory>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

/**
   \cond INTERNAL 
   \ingroup traits
   if the compiler wants to instanciate this functions, A and B are different types
   the compiler will 
*/

template <typename A, typename B> 
void __assert_type_equal (A& a, B& b) 
{
	static_assert(sizeof(A) == 0); 
}

template <typename A> 
void __assert_type_equal (A& /*a*/, A& /*b*/) 
{
}

#define ASSERT_TYPE_EQUAL(A, B)		  \
	{ 				  \
		A *a; B *b; 		  \
		__assert_type_equal(a,b); \
	}

template <template <typename> class  Data, typename T> 
struct __eval {
	static Data<float> *apply(const Data<T> &input, double m, double v) {
		Data<float> *result = new Data<float>(input.get_size()); 
		double invv = 1.0/v; 
		transform(input.begin(), input.end(), result->begin(), 
			  [invv,m](T x){(x - m) * invv;}); 
		return result; 
	}
}; 


template <template <typename> class  Data> 
struct __eval<Data, bool> {
	static Data<float> *apply(const Data<bool> &input, double m, double v) {

		Data<float> *result = new Data<float>(input.get_size()); 
		float rtrue = (1.0 - m) / v; 
		float rfalse =  - m / v; 
		
		transform(input.begin(), input.end(), result->begin(),
			  [rtrue, rfalse](bool x){b ? rtrue : rfalse;});
		return result; 
	}
}; 



/**
   \ingroup templates 
   \brief Generic filter to normalize an image 

   Generic implementation of a filter that normalizes an Image to have a zero-mean intensity 
   and an intensity variation of one. 
   \tparam the image type 
 */
template <class Image> 
struct FNormalizer: public TFilter<Image *>
{
	template <typename T, template <typename> class  Data>
	typename FNormalizer::result_type operator ()(const Data<T> &image) const {
		ASSERT_TYPE_EQUAL(Image, typename Data<T>::Super); 
		double sum = 0.0; 
		double sum2 = 0.0; 
		typename Data<T>::const_iterator i = image.begin(); 
		typename Data<T>::const_iterator e = image.end(); 
		while ( i != e ) {
			sum += *i; 
			sum2 += *i * *i; 
			++i; 
		}
		double n = image.size(); 
		double m = sum / n; 
		double v = sqrt((sum2 - n * m * m) / (n - 1)); 

		mia::cvdebug() << "FNormalizer: avg = " << m << " var = " << v << "\n"; 

		if (v < 0.000001) 
			v = 1.0;
		
		return __eval<Data, T>::apply(image, m, v); 
		
	}
}; 

/// @endcond

/**
   \ingroup templates 
   \brief a normalizer for image intensities 
   
   The intensities of the input image are normalized to have a zero mean and a deviation of one. 
   The output image is of the same dimensions as the input image and has pixel type float. 
   \tparam Image the image type 
   \param image the input image 
   \returns the normalized image 
 */
template <class Image>
std::shared_ptr<Image > normalize(const Image& image)
{
	FNormalizer<Image> n; 
	return std::shared_ptr<Image >(mia::filter(n, image)); 
}

NS_MIA_END

