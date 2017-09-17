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

NS_MIA_BEGIN

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
T1DConvoluteInterpolator<T>::T1DConvoluteInterpolator(const std::vector<T>& data, PSplineKernel  kernel, 
						      const CSplineBoundaryCondition& boundary_conditions):
	m_coeff(data.size()), 
	m_kernel(kernel),
	m_boundary_conditions(boundary_conditions.clone()),
	m_x_index(kernel->size()),
	m_x_weight(kernel->size())
{
	m_boundary_conditions->set_width(data.size()); 

	min_max<typename std::vector<T>::const_iterator>::get(data.begin(), data.end(), m_min, m_max);

	
	// copy the data
	__dispatch_copy<std::vector<T>, TCoeff1D >::apply(data, m_coeff); 
	
	m_boundary_conditions->filter_line(m_coeff, m_kernel->get_poles());
}

template <typename T>
T1DConvoluteInterpolator<T>::~T1DConvoluteInterpolator()
{
}
	


template <class In, class Out, bool must_round>
struct __dispatch_round_to {	
	static Out value(In x) {
		return Out(x); 
	}
};

template <class In, class Out>
struct __dispatch_round_to<In, Out, true> {
	static Out value(In x) {
		return static_cast<Out>(rint(x)); 
	}
};


template <typename In, typename Out>
struct round_to {
	static Out value(In x) {
		const bool out_is_int = std::is_integral<Out>::value; 
		const bool in_is_int = std::is_integral<In>::value; 
		const bool must_round = out_is_int && !in_is_int; 
		
		return __dispatch_round_to<In, Out, must_round>::value(x); 
	}
};



template <class T, class U>
struct bounded {
	static void apply(T& r, const U& min, const U& max)
	{
		if (r > min) {
			if (r < max)
				return;
			else
				r = max;
		}else
			r = min; 
	}
};

template <class C, int size>
struct add_1d {
	typedef typename C::value_type U; 
	
	static typename C::value_type value(const C&  coeff, const CSplineKernel::VWeight& xweight, 
					    const CSplineKernel::VIndex& xindex) 
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
	

	(*m_kernel)(x, m_x_weight, m_x_index);
	m_boundary_conditions->apply(m_x_index, m_x_weight);

	U result = U();
	
	switch (m_kernel->size()) {
	case 1: result = add_1d<TCoeff1D,1>::value(m_coeff, m_x_weight, m_x_index); break; 
	case 2: result = add_1d<TCoeff1D,2>::value(m_coeff, m_x_weight, m_x_index); break; 
	case 3: result = add_1d<TCoeff1D,3>::value(m_coeff, m_x_weight, m_x_index); break; 
	case 4: result = add_1d<TCoeff1D,4>::value(m_coeff, m_x_weight, m_x_index); break; 
	case 5: result = add_1d<TCoeff1D,5>::value(m_coeff, m_x_weight, m_x_index); break; 
	case 6: result = add_1d<TCoeff1D,6>::value(m_coeff, m_x_weight, m_x_index); break; 
	default: {
		/* perform interpolation */
		for (size_t i = 0; i < m_kernel->size(); ++i) {
			result += m_x_weight[i] * m_coeff[m_x_index[i]];
		}
		
	}
	} // end switch 
	
	bounded<U, T>::apply(result, m_min, m_max);
	
	return round_to<U, T>::value(result); 
}

template <typename T> 
typename coeff_map<T>::coeff_type
T1DConvoluteInterpolator<T>::derivative_at (const double& x) const
{
	typedef typename TCoeff1D::value_type U; 
	
	// cut at boundary
	if (x < 0.0 || x >= m_coeff.size())
		return typename coeff_map<T>::coeff_type();
	
	m_kernel->derivative(x, m_x_weight, m_x_index);
	m_boundary_conditions->apply(m_x_index, m_x_weight); 

	U result = U();
	
	switch (m_kernel->size()) {
	case 2: result = add_1d<TCoeff1D,2>::value(m_coeff, m_x_weight, m_x_index); break; 
	case 3: result = add_1d<TCoeff1D,3>::value(m_coeff, m_x_weight, m_x_index); break; 
	case 4: result = add_1d<TCoeff1D,4>::value(m_coeff, m_x_weight, m_x_index); break; 
	case 5: result = add_1d<TCoeff1D,5>::value(m_coeff, m_x_weight, m_x_index); break; 
	case 6: result = add_1d<TCoeff1D,6>::value(m_coeff, m_x_weight, m_x_index); break; 
	default: {
		/* perform interpolation */
		
		for (size_t x = 0; x < m_kernel->size(); ++x) {
			result += m_x_weight[x] * m_coeff[m_x_index[x]];
		}
		
	}
	} // end switch 
	
	return result; 
	
}


NS_MIA_END



