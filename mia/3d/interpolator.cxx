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

#include <mia/core/threadedmsg.hh>
#include <mia/core/parallel.hh>

NS_MIA_BEGIN

template <class T>
struct min_max_3d {
	static void get( const T3DDatafield<T>& data,
			 typename T3DDatafield<T>::value_type& min,
			 typename T3DDatafield<T>::value_type& max)
	{
		auto i = data.begin(); 
		auto e = data.end(); 
		min = max = *i++; 
		
		while (i != e) {
			if (*i > max) max = *i; 
			if (*i < min) min = *i; 
			++i; 
		}
	}
};

template <class  T>
struct min_max_3d<T3DVector<T> > {
	static void get( const T3DDatafield<T3DVector<T> >& data, T3DVector<T>& min, T3DVector<T>& max)
	{
		auto i = data.begin(); 
		auto e = data.end(); 
		min = max = *i++; 
		
		while (i != e) {
			if (i->x > max.x) max.x = i->x; 
			if (i->y > max.y) max.y = i->y; 
			if (i->z > max.z) max.z = i->z; 
			if (i->x < min.x) min.x = i->x; 
			if (i->y < min.y) min.y = i->y; 
			if (i->z < min.z) min.z = i->z; 
			++i; 
		}
	}
};


template <typename T>
T3DConvoluteInterpolator<T>::T3DConvoluteInterpolator(const T3DDatafield<T>& image, PSplineKernel  kernel):
	m_coeff(image.get_size()), 
	m_size2(image.get_size() + image.get_size()-C3DBounds(2,2,2)),
	m_kernel(kernel),
	m_xbc(produce_spline_boundary_condition("mirror")), 
	m_ybc(produce_spline_boundary_condition("mirror")),
	m_zbc(produce_spline_boundary_condition("mirror")),
	m_cache(kernel->size(), *m_xbc, *m_ybc, *m_zbc)
{

	prefilter(image); 
}
	
template <typename T>
T3DConvoluteInterpolator<T>::T3DConvoluteInterpolator(const T3DDatafield<T>& image, PSplineKernel  kernel, 
				 const CSplineBoundaryCondition& xbc,  
				 const CSplineBoundaryCondition& ybc, 
				 const CSplineBoundaryCondition& zbc):
	m_coeff(image.get_size()), 
	m_size2(image.get_size() + image.get_size()-C3DBounds(2,2,2)),
	m_kernel(kernel),
	m_xbc(xbc.clone()), 
	m_ybc(ybc.clone()),
	m_zbc(zbc.clone()),
	m_cache(kernel->size(), *m_xbc, *m_ybc, *m_zbc)
{
	prefilter(image); 
}


template <typename T>
void T3DConvoluteInterpolator<T>::prefilter(const T3DDatafield<T>& image) 
{

	m_xbc->set_width(image.get_size().x); 
	m_cache.x.reset(); 
	m_ybc->set_width(image.get_size().y); 
	m_cache.y.reset(); 
	m_zbc->set_width(image.get_size().z);
	m_cache.z.reset(); 

	min_max_3d<T>::get(image, m_min, m_max);
	// we always allow that a pixel is set to zero
	if (T() < m_min) 
		m_min = T(); 

	std::copy(image.begin(), image.end(), m_coeff.begin());


	auto poles = m_kernel->get_poles(); 
	if (poles.empty()) 
		return; 
	
	int cachXSize = image.get_size().x;	
	int cachYSize = image.get_size().y;
	int cachZSize = image.get_size().z;
	

	auto filter_x = [this, cachXSize, cachYSize, poles](const C1DParallelRange& range_z) {
		coeff_vector buffer(cachXSize);
		for (auto z = range_z.begin(); z != range_z.end() ; ++z){
			for (int y = 0; y < cachYSize; y++) {
				m_coeff.get_data_line_x(y,z,buffer);
				m_xbc->filter_line(buffer, poles);
				m_coeff.put_data_line_x(y,z,buffer);
			}
		}
	};
	pfor(C1DParallelRange(0, cachZSize, 1), filter_x); 
	
	
	auto filter_y = [this, cachXSize, cachYSize, poles](const C1DParallelRange& range_z) {
		coeff_vector buffer(cachYSize);
		for (auto z = range_z.begin(); z  != range_z.end() ; ++z){
			for (int x = 0; x < cachXSize; x++) {
				m_coeff.get_data_line_y(x,z,buffer);
				m_ybc->filter_line(buffer, poles);
				m_coeff.put_data_line_y(x,z,buffer);
			}
		}
	};
	pfor(C1DParallelRange(0, cachZSize, 1), filter_y); 
	

	auto filter_z = [this, cachXSize, cachZSize, poles](const C1DParallelRange& range_y) {
		coeff_vector buffer(cachZSize);
		for (auto y = range_y.begin(); y  != range_y.end() ; ++y){
			for (int x = 0; x < cachXSize; x++) {
				m_coeff.get_data_line_z(x,y,buffer);
				m_zbc->filter_line(buffer, poles);
				m_coeff.put_data_line_z(x,y,buffer);
			}
		}
	};
	pfor(C1DParallelRange(0, cachYSize, 1), filter_z); 

}

template <typename T>
C3DWeightCache T3DConvoluteInterpolator<T>::create_cache() const
{
	return C3DWeightCache(m_kernel->size(), *m_xbc, *m_ybc, *m_zbc); 
}

template <typename T>
T3DConvoluteInterpolator<T>::~T3DConvoluteInterpolator()
{
}
	
template <class T, class U>
struct bounded<T3DVector<T>, T3DVector<U> > {
	static void apply(T3DVector<T>& r, const T3DVector<U>& min, const T3DVector<U>& max)
	{
		r.x = (r.x >= min.x) ? ( (r.x <= max.x) ? r.x : max.x) : min.x;
		r.y = (r.y >= min.y) ? ( (r.y <= max.y) ? r.y : max.y) : min.y;
		r.z = (r.z >= min.z) ? ( (r.z <= max.z) ? r.z : max.z) : min.z;		
	}
};

template <class C, int size>
struct add_3d {
	typedef typename C::value_type U; 
	
	static typename C::value_type value(const C&  coeff, const C3DWeightCache& cache) 
	{
		U result = U();
		
		for (size_t z = 0; z < size; ++z) {
			U ry = U();
			for (size_t y = 0; y < size; ++y) {
				U rx = U();
				const U *p = &coeff(0, cache.y.index[y], cache.z.index[z]);
				for (size_t x = 0; x < size; ++x) {
					int xinx = cache.x.is_flat ? cache.x.start_idx + x : cache.x.index[x]; 
					rx += cache.x.weights[x] * p[xinx];
				}
				ry += cache.y.weights[y] * rx; 
			}
			result += cache.z.weights[z] * ry; 
		}
		return result; 
	}
};

template <typename T>
struct add_3d<T3DDatafield< T >, 1> {
	static T value(const T3DDatafield< T >&  coeff, const C3DWeightCache& cache) 
		{
			return cache.x.weights[0] *  cache.y.weights[0] * cache.z.weights[0] * 
				coeff(cache.x.index[0], cache.y.index[0], cache.z.index[0] ) ; 
		}
};


#ifdef __SSE2__
template <>
struct add_3d<T3DDatafield< double >, 2> {
	static double value(const T3DDatafield< double >&  coeff, const C3DWeightCache& cache); 
	
};

template <>
struct add_3d<T3DDatafield< double >, 4> {
	static double value(const T3DDatafield< double >&  coeff, const C3DWeightCache& cache); 
	
};
#endif

#ifdef __SSE__
template <>
struct add_3d<T3DDatafield< float >, 4> {
	static float value(const T3DDatafield< float >&  coeff, const C3DWeightCache& cache); 
	
};
template <>
struct add_3d<T3DDatafield< float >, 2> {
	static float value(const T3DDatafield< float >&  coeff, const C3DWeightCache& cache); 
	
};

#endif

template <typename T>
T  T3DConvoluteInterpolator<T>::operator () (const C3DFVector& x, C3DWeightCache& cache) const
{
	typedef typename TCoeff3D::value_type U; 
	
	// x will usually be the fastest changing index, therefore, it is of no use to use the cache 
	// at the same time it's access may be handled "flat" 
	m_kernel->get_uncached(x.x, cache.x);

	// the other two coordinates are changing slowly and caching makes sense 
	// however, the index set will always be fully evaluated 
	if (x.y != cache.y.x) 
		m_kernel->get_cached(x.y, cache.y);
	
	if (x.z != cache.z.x) 
		m_kernel->get_cached(x.z, cache.z);
	
	U result = U();
	// now we give the compiler a chance to optimize based on kernel size and data type.  
	// Some of these call also use template specialization to provide an optimized code path.  
	// With SSE and SSE2 available kernel sizes 2 and 4 and the use of float and double 
	// scalar fields are optimized.
	switch (m_kernel->size()) {
	case 1: result = add_3d<TCoeff3D,1>::value(m_coeff, cache); break; 
	case 2: result = add_3d<TCoeff3D,2>::value(m_coeff, cache); break; 
	case 3: result = add_3d<TCoeff3D,3>::value(m_coeff, cache); break; 
	case 4: result = add_3d<TCoeff3D,4>::value(m_coeff, cache); break; 
	case 5: result = add_3d<TCoeff3D,5>::value(m_coeff, cache); break; 
	case 6: result = add_3d<TCoeff3D,6>::value(m_coeff, cache); break; 
	default: {
		assert(0 && "kernel sizes above 5 are not implemented"); 
	}
	} // end switch 
	
	bounded<U, T>::apply(result, m_min, m_max);
	
	return round_to<U, T>::value(result); 
}

template <typename T>
T  T3DConvoluteInterpolator<T>::operator () (const C3DFVector& x) const
{
	CScopedLock lock(m_cache_lock);
	typedef typename TCoeff3D::value_type U; 
	
	// x will usually be the fastest changing index, therefore, it is of no use to use the cache 
	// at the same time it's access may be handled "flat" 
	m_kernel->get_uncached(x.x, m_cache.x);

	// the other two coordinates are changing slowly and caching makes sense 
	// however, the index set will always be fully evaluated 
	if (x.y != m_cache.y.x) 
		m_kernel->get_cached(x.y, m_cache.y);
	
	if (x.z != m_cache.z.x) 
		m_kernel->get_cached(x.z, m_cache.z);	
	
	U result = U();
	// now we give the compiler a chance to optimize based on kernel size and data type.  
	// Some of these call also use template specialization to provide an optimized code path.  
	// With SSE and SSE2 available kernel sizes 2 and 4 and the use of float and double 
	// scalar fields are optimized.
	switch (m_kernel->size()) {
	case 1: result = add_3d<TCoeff3D,1>::value(m_coeff, m_cache); break; 
	case 2: result = add_3d<TCoeff3D,2>::value(m_coeff, m_cache); break; 
	case 3: result = add_3d<TCoeff3D,3>::value(m_coeff, m_cache); break; 
	case 4: result = add_3d<TCoeff3D,4>::value(m_coeff, m_cache); break; 
	case 5: result = add_3d<TCoeff3D,5>::value(m_coeff, m_cache); break; 
	case 6: result = add_3d<TCoeff3D,6>::value(m_coeff, m_cache); break; 
	default: {
		assert(0 && "kernel sizes above 5 are not implemented"); 
	}
	} // end switch 
	
	bounded<U, T>::apply(result, m_min, m_max);
	
	return round_to<U, T>::value(result); 
}

NS_MIA_END



