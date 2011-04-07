/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
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

  The filter routines for splines and omoms is based on code by 
  Philippe Thevenaz http://bigwww.epfl.ch/thevenaz/interpolation/
   
*/

#include <cmath>

NS_MIA_BEGIN

#if 0
template <typename T>
T T1DInterpolator<T>::operator () (const double& x) const
{
	return T(); 
}
#endif

template <typename T>
T1DDirectInterpolator<T>::T1DDirectInterpolator(const std::vector<T>& data):
       _M_data(data)
{
}

template <typename T>
T1DNNInterpolator<T>::T1DNNInterpolator(const std::vector<T>& image):
	T1DDirectInterpolator<T>(image)
{
}

template <typename T>
T
T1DNNInterpolator<T>::operator () (const double& x)const
{
	size_t ix( size_t(x + 0.5f));
	if (ix < this->data().size())
		return this->data()[ix];
	else
		return T();
}

template <typename T>
typename coeff_map<T>::coeff_type
T1DNNInterpolator<T>::derivative_at (const double& /*x*/) const
{
	throw std::invalid_argument("Nearest Neighbor interpolator doesn't support a derivative"); 
}


template <typename T>
T1DLinearInterpolator<T>::T1DLinearInterpolator(const std::vector<T>& image):
	T1DDirectInterpolator<T>(image), 
	_M_size(image.size()),
	_M_sizeb(image.size()-1)
		
{
}

template <typename T> 
struct lin_dispatch {
	static T apply(const std::vector<T>& data, const double& p, const double& sizeb) {
	// Calculate the coordinates and the distances
		const double x  = floor(p);
		const double fx = p - x;
		const double dx = 1.0f - fx;
		size_t ux = x; 

		if ( x < sizeb && 0.0 <= x )
			return T ( (dx * data[ux] + fx * data[ux + 1]) );
		else if (ux < data.size())
			return data[ux]; 
		else
			return T(); 
	}
	static typename coeff_map<T>::coeff_type 
	apply_derivative(const std::vector<T>& data, const double& p, const double& /*sizeb*/) {
		const size_t ux = floor(p); 
		if (ux < data.size() - 1)
			return typename coeff_map<T>::coeff_type(data[ux+1] - data[ux]); 
		else 
			return typename coeff_map<T>::coeff_type(); 
	}
};

template <> 
struct lin_dispatch<bool> {
	static bool apply(const std::vector<bool>& data, const double& p, const double& /*sizeb*/) {
		const double x  = floor(p);
		const double fx = p - x;
		const double dx = 1.0f - fx;
		
		const size_t ux = (size_t)x;  
		const double  a1 = dx * data[ux] + fx * data[ux+1];

		return  a1 > 0.5;
		
	}
	static coeff_map<bool>::coeff_type 
	apply_derivative(const std::vector<bool>& data, const double& p, const double& /*sizeb*/) {
		const size_t ux = floor(p); 
		if (ux < data.size() - 1) {
			if (data[ux]  ==  data[ux+1]) 
				return 0.0; 
			if (data[ux] && !data[ux+1]) 
				return -1;
			else 
				return 1;
		}
		return 0.0; 
	}
};
	
template <typename T>
T  T1DLinearInterpolator<T>::operator () (const double& p)const
{
	return lin_dispatch<T>::apply(this->data(), p, this->_M_sizeb); 
}


template <typename T>
typename coeff_map<T>::coeff_type
T1DLinearInterpolator<T>::derivative_at (const double& x) const
{
	return lin_dispatch<T>::apply_derivative(this->data(), x, this->_M_sizeb); 
}

template <typename T>
void __dispatch_min_max<T>::apply(const T i, T& min, T &max) 
{
	if (i > max) max = i; 
	if (i < min) min = i; 
}


template <class I>
struct min_max {
	typedef typename std::iterator_traits<I>::value_type T;
	static void get( I i, I e, T& min, T& max)
	{
		min = max = *i++; 
		while (i != e) {
			__dispatch_min_max<T>::apply(*i, min, max); 
			++i; 
		}
	}
};

template <typename I, typename O> 
void __dispatch_copy<I,O>::apply(const I& input, O& output)
{
	std::copy(input.begin(), input.end(), output.begin());
}
 
template <typename T>
T1DConvoluteInterpolator<T>::T1DConvoluteInterpolator(const std::vector<T>& data, 
						      std::shared_ptr<CBSplineKernel >  kernel):
	_M_coeff(data.size()), 
	_M_size2(data.size() + data.size() - 2),
	_M_kernel(kernel),
	_M_x_index(kernel->size()),
	_M_x_weight(kernel->size())
{
	min_max<typename std::vector<T>::const_iterator>::get(data.begin(), data.end(), _M_min, _M_max);
	
	// copy the data
	__dispatch_copy<std::vector<T>, TCoeff1D >::apply(data, _M_coeff); 
	
	_M_kernel->filter_line(_M_coeff);
}

template <typename T>
T1DConvoluteInterpolator<T>::~T1DConvoluteInterpolator()
{
}
	
template <class In, class Out>
struct round_to {
	
	static Out value(In x) {
		return (Out)x;
	}
};

template <class In>
struct round_to<In, unsigned char> {	
	static unsigned char value(In x) {
		return (unsigned char)floor(x + 0.5);
	}
};

template <class In>
struct round_to<In, signed char> {	
	static signed char value(In x) {
		return (signed char)floor(x + 0.5);
	}
};

template <class In>
struct round_to<In, unsigned short> {	
	static unsigned short value(In x) {
		return (unsigned short)floor(x + 0.5);
	}
};

template <class In>
struct round_to<In, signed short> {	
	static signed short value(In x) {
		return (signed short)floor(x + 0.5);
	}
};


template <class In>
struct round_to<In, unsigned int> {	
	static unsigned long value(In x) {
		return (unsigned long)floor(x + 0.5);
	}
};

template <class In>
struct round_to<In, signed int> {	
	static signed long value(In x) {
		return (signed long)floor(x + 0.5);
	}
};
template <class In>
struct round_to<In, unsigned long> {	
	static unsigned long value(In x) {
		return (unsigned long)floor(x + 0.5);
	}
};

template <class In>
struct round_to<In, signed long> {	
	static signed long value(In x) {
		return (signed long)floor(x + 0.5);
	}
};

template <class T, class U>
struct bounded {
	static void apply(T& r, const U& min, const U& max)
	{
		r = (r >= min) ? ( (r <= max) ? r : max) : min;
	}
};

template <class C, int size>
struct add_1d {
	typedef typename C::value_type U; 
	
	static typename C::value_type value(const C&  coeff, const std::vector<double>& xweight, 
					    const std::vector<int>& xindex) 
	{
		U result = U();
		
		for (size_t x = 0; x < size; ++x) {
			result += xweight[x] * coeff[xindex[x]];
		}
		return result; 
	}
};


template <typename T>
T  T1DConvoluteInterpolator<T>::operator () (const double& x) const
{
	typedef typename TCoeff1D::value_type U; 
	
	// cut at boundary
	if (x < 0.0 || x >= _M_coeff.size())
		return T();
	
	(*_M_kernel)(x, _M_x_weight, _M_x_index);
	mirror_boundary_conditions(_M_x_index, _M_coeff.size(), _M_size2);

	U result = U();
	
	switch (_M_kernel->size()) {
	case 2: result = add_1d<TCoeff1D,2>::value(_M_coeff, _M_x_weight, _M_x_index); break; 
	case 3: result = add_1d<TCoeff1D,3>::value(_M_coeff, _M_x_weight, _M_x_index); break; 
	case 4: result = add_1d<TCoeff1D,4>::value(_M_coeff, _M_x_weight, _M_x_index); break; 
	case 5: result = add_1d<TCoeff1D,5>::value(_M_coeff, _M_x_weight, _M_x_index); break; 
	case 6: result = add_1d<TCoeff1D,6>::value(_M_coeff, _M_x_weight, _M_x_index); break; 
	default: {
		/* perform interpolation */
		
		U result = U();
		
		for (size_t x = 0; x < _M_kernel->size(); ++x) {
			result += _M_x_weight[x] * _M_coeff[_M_x_index[x]];
		}
		
	}
	} // end switch 
	
	bounded<U, T>::apply(result, _M_min, _M_max);
	
	return round_to<U, T>::value(result); 
}

template <typename T> 
typename coeff_map<T>::coeff_type
T1DConvoluteInterpolator<T>::derivative_at (const double& x) const
{
	typedef typename TCoeff1D::value_type U; 
	
	// cut at boundary
	if (x < 0.0 || x >= _M_coeff.size())
		return typename coeff_map<T>::coeff_type();
	
	_M_kernel->derivative(x, _M_x_weight, _M_x_index);
	mirror_boundary_conditions(_M_x_index, _M_coeff.size(), _M_size2);

	U result = U();
	
	switch (_M_kernel->size()) {
	case 2: result = add_1d<TCoeff1D,2>::value(_M_coeff, _M_x_weight, _M_x_index); break; 
	case 3: result = add_1d<TCoeff1D,3>::value(_M_coeff, _M_x_weight, _M_x_index); break; 
	case 4: result = add_1d<TCoeff1D,4>::value(_M_coeff, _M_x_weight, _M_x_index); break; 
	case 5: result = add_1d<TCoeff1D,5>::value(_M_coeff, _M_x_weight, _M_x_index); break; 
	case 6: result = add_1d<TCoeff1D,6>::value(_M_coeff, _M_x_weight, _M_x_index); break; 
	default: {
		/* perform interpolation */
		
		U result = U();
		
		for (size_t x = 0; x < _M_kernel->size(); ++x) {
			result += _M_x_weight[x] * _M_coeff[_M_x_index[x]];
		}
		
	}
	} // end switch 
	
	return result; 
	
}


NS_MIA_END



