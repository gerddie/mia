/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

NS_MIA_BEGIN

template <class T>
struct min_max_3d {
	static void get( const T3DDatafield<T>& data, T* min, T*max)
	{
		typename T3DDatafield<T>::const_iterator i = data.begin(); 
		typename T3DDatafield<T>::const_iterator e = data.end(); 
		*min = *max = *i++; 
		
		while (i != e) {
			if (*i > *max) *max = *i; 
			if (*i < *min) *min = *i; 
			++i; 
		}
	}
};

template <class  T>
struct min_max_3d<T3DVector<T> > {
	static void get( const T3DDatafield<T3DVector<T> >& data, T3DVector<T>* min, T3DVector<T>*max)
	{
		typename T3DDatafield<T3DVector<T> >::const_iterator i = data.begin(); 
		typename T3DDatafield<T3DVector<T> >::const_iterator e = data.end(); 
		*min = *max = *i++; 
		
		while (i != e) {
			if (i->x > max->x) max->x = i->x; 
			if (i->y > max->y) max->y = i->y; 
			if (i->z > max->z) max->z = i->z; 
			if (i->x < min->x) min->x = i->x; 
			if (i->y < min->y) min->y = i->y; 
			if (i->z < min->z) min->z = i->z; 
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
	m_x_cache(kernel->size(), *m_xbc, m_kernel->size() < 3), 
	m_y_cache(kernel->size(), *m_ybc, true), 
	m_z_cache(kernel->size(), *m_zbc, true)
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
	m_x_cache(kernel->size(), *m_xbc, m_kernel->size() < 3), 
	m_y_cache(kernel->size(), *m_ybc, true), 
	m_z_cache(kernel->size(), *m_zbc, true)
{
	prefilter(image); 
}


template <typename T>
void T3DConvoluteInterpolator<T>::prefilter(const T3DDatafield<T>& image) 
{

	m_xbc->set_width(image.get_size().x); 
	m_x_cache.reset(); 
	m_ybc->set_width(image.get_size().y); 
	m_y_cache.reset(); 
	m_zbc->set_width(image.get_size().z);
	m_z_cache.reset(); 

	min_max_3d<T>::get(image, &m_min, &m_max);
	std::copy(image.begin(), image.end(), m_coeff.begin());


	auto poles = m_kernel->get_poles(); 
	if (poles.empty()) 
		return; 
	
	int cachXSize = image.get_size().x;	
	int cachYSize = image.get_size().y;
	int cachZSize = image.get_size().z;
	

	auto filter_x = [this, cachXSize, cachYSize, poles](const tbb::blocked_range<size_t>& range_z) {
		coeff_vector buffer(cachXSize);
		for (auto z = range_z.begin(); z != range_z.end() ; ++z){
			for (int y = 0; y < cachYSize; y++) {
				m_coeff.get_data_line_x(y,z,buffer);
				m_xbc->filter_line(buffer, poles);
				m_coeff.put_data_line_x(y,z,buffer);
			}
		}
	};
	parallel_for(tbb::blocked_range<size_t>(0, cachZSize, 1), filter_x); 
	
	
	auto filter_y = [this, cachXSize, cachYSize, poles](const tbb::blocked_range<size_t>& range_z) {
		coeff_vector buffer(cachYSize);
		for (auto z = range_z.begin(); z  != range_z.end() ; ++z){
			for (int x = 0; x < cachXSize; x++) {
				m_coeff.get_data_line_y(x,z,buffer);
				m_ybc->filter_line(buffer, poles);
				m_coeff.put_data_line_y(x,z,buffer);
			}
		}
	};
	parallel_for(tbb::blocked_range<size_t>(0, cachZSize, 1), filter_y); 
	

	auto filter_z = [this, cachXSize, cachZSize, poles](const tbb::blocked_range<size_t>& range_y) {
		coeff_vector buffer(cachZSize);
		for (auto y = range_y.begin(); y  != range_y.end() ; ++y){
			for (int x = 0; x < cachXSize; x++) {
				m_coeff.get_data_line_z(x,y,buffer);
				m_zbc->filter_line(buffer, poles);
				m_coeff.put_data_line_z(x,y,buffer);
			}
		}
	};
	parallel_for(tbb::blocked_range<size_t>(0, cachYSize, 1), filter_z); 

}

template <typename T>
CWeightCache T3DConvoluteInterpolator<T>::create_cache() const
{
	return CWeightCache(m_kernel->size(), *m_xbc, *m_ybc, *m_zbc); 
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
	
	static typename C::value_type value(const C&  coeff, const CSplineKernel::SCache& xc, 
					    const CSplineKernel::SCache& yc,
					    const CSplineKernel::SCache& zc) 
	{
		U result = U();
		
		for (size_t z = 0; z < size; ++z) {
			U ry = U();
			for (size_t y = 0; y < size; ++y) {
				U rx = U();
				const U *p = &coeff(0, yc.index[y], zc.index[z]);
				for (size_t x = 0; x < size; ++x) {
					int xinx = xc.is_flat ? xc.start_idx +x : xc.index[x]; 
					rx += xc.weights[x] * p[xinx];
				}
				ry += yc.weights[y] * rx; 
			}
			result += zc.weights[z] * ry; 
		}
		return result; 
	}
};

template <typename T>
struct add_3d<T3DDatafield< T >, 1> {
	static T value(const T3DDatafield< T >&  coeff, 
		       const CSplineKernel::SCache& xc, 
		       const CSplineKernel::SCache& yc,
		       const CSplineKernel::SCache& zc) 
		{
			return xc.weights[0] *  yc.weights[0] * zc.weights[0] * 
				coeff(xc.index[0], yc.index[0], zc.index[0] ) ; 
		}
};


#ifdef __SSE2__
template <>
struct add_3d<T3DDatafield< double >, 2> {
	static double value(const T3DDatafield< double >&  coeff, 
			    const CSplineKernel::SCache& xc, 
			    const CSplineKernel::SCache& yc,
			    const CSplineKernel::SCache& zc); 
	
};

template <>
struct add_3d<T3DDatafield< double >, 4> {
	static double value(const T3DDatafield< double >&  coeff, 
			    const CSplineKernel::SCache& xc, 
			    const CSplineKernel::SCache& yc,
			    const CSplineKernel::SCache& zc); 
	
};
#endif

#ifdef __SSE__
template <>
struct add_3d<T3DDatafield< float >, 4> {
	static float value(const T3DDatafield< float >&  coeff, 
			    const CSplineKernel::SCache& xc, 
			    const CSplineKernel::SCache& yc,
			    const CSplineKernel::SCache& zc); 
	
};
template <>
struct add_3d<T3DDatafield< float >, 2> {
	static float value(const T3DDatafield< float >&  coeff, 
			    const CSplineKernel::SCache& xc, 
			    const CSplineKernel::SCache& yc,
			    const CSplineKernel::SCache& zc); 
	
};

#endif

template <typename T>
T  T3DConvoluteInterpolator<T>::operator () (const C3DFVector& x, CWeightCache& cache) const
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
	case 1: result = add_3d<TCoeff3D,1>::value(m_coeff, cache.x, cache.y, cache.z); break; 
	case 2: result = add_3d<TCoeff3D,2>::value(m_coeff, cache.x, cache.y, cache.z); break; 
	case 3: result = add_3d<TCoeff3D,3>::value(m_coeff, cache.x, cache.y, cache.z); break; 
	case 4: result = add_3d<TCoeff3D,4>::value(m_coeff, cache.x, cache.y, cache.z); break; 
	case 5: result = add_3d<TCoeff3D,5>::value(m_coeff, cache.x, cache.y, cache.z); break; 
	case 6: result = add_3d<TCoeff3D,6>::value(m_coeff, cache.x, cache.y, cache.z); break; 
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
	m_kernel->get_uncached(x.x, m_x_cache);

	// the other two coordinates are changing slowly and caching makes sense 
	// however, the index set will always be fully evaluated 
	if (x.y != m_y_cache.x) 
		m_kernel->get_cached(x.y, m_y_cache);
	
	if (x.z != m_z_cache.x) 
		m_kernel->get_cached(x.z, m_z_cache);	
	
	U result = U();
	// now we give the compiler a chance to optimize based on kernel size and data type.  
	// Some of these call also use template specialization to provide an optimized code path.  
	// With SSE and SSE2 available kernel sizes 2 and 4 and the use of float and double 
	// scalar fields are optimized.
	switch (m_kernel->size()) {
	case 1: result = add_3d<TCoeff3D,1>::value(m_coeff, m_x_cache, m_y_cache, m_z_cache); break; 
	case 2: result = add_3d<TCoeff3D,2>::value(m_coeff, m_x_cache, m_y_cache, m_z_cache); break; 
	case 3: result = add_3d<TCoeff3D,3>::value(m_coeff, m_x_cache, m_y_cache, m_z_cache); break; 
	case 4: result = add_3d<TCoeff3D,4>::value(m_coeff, m_x_cache, m_y_cache, m_z_cache); break; 
	case 5: result = add_3d<TCoeff3D,5>::value(m_coeff, m_x_cache, m_y_cache, m_z_cache); break; 
	case 6: result = add_3d<TCoeff3D,6>::value(m_coeff, m_x_cache, m_y_cache, m_z_cache); break; 
	default: {
		assert(0 && "kernel sizes above 5 are not implemented"); 
	}
	} // end switch 
	
	bounded<U, T>::apply(result, m_min, m_max);
	
	return round_to<U, T>::value(result); 
}

NS_MIA_END



