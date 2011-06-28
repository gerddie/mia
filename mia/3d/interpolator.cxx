/* -*- mia-c++ -*-
** Copyrigh (C) 2004 MPI of Human Cognitive and Brain Sience
**                    Gert Wollny <wollny@cbs.mpg.de>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  The filter routines for splines and omoms is based on code by 
  Philippe Thevenaz http://bigwww.epfl.ch/thevenaz/interpolation/
   
*/

#include <cmath>

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
T3DConvoluteInterpolator<T>::T3DConvoluteInterpolator(const T3DDatafield<T>& image, std::shared_ptr<CBSplineKernel >  kernel):
	m_coeff(image.get_size()), 
	m_size2(image.get_size() + image.get_size() - C3DBounds(2,2,2)),
	m_kernel(kernel),
	m_x_cache(kernel->size(), m_coeff.get_size().x, m_size2.x, false), 
	m_y_cache(kernel->size(), m_coeff.get_size().y, m_size2.y, true), 
	m_z_cache(kernel->size(), m_coeff.get_size().z, m_size2.z, true)
{
	min_max_3d<T>::get(image, &m_min, &m_max);
	
	// copy the data
	std::copy(image.begin(), image.end(), m_coeff.begin());
	
	int cachXSize = image.get_size().x;	
	int cachYSize = image.get_size().y;
	int cachZSize = image.get_size().z;
	
	{
		coeff_vector buffer(cachXSize);
		for (int z = 0; z < cachZSize; z++){
			for (int y = 0; y < cachYSize; y++) {
				m_coeff.get_data_line_x(y,z,buffer);
				m_kernel->filter_line(buffer);
				m_coeff.put_data_line_x(y,z,buffer);
			}
		}
	}
	
	{
		coeff_vector buffer(cachYSize);
		for (int z = 0; z < cachZSize; z++){
			for (int x = 0; x < cachXSize; x++) {
				m_coeff.get_data_line_y(x,z,buffer);
				m_kernel->filter_line(buffer);
				m_coeff.put_data_line_y(x,z,buffer);
			}
		}
	}
	
	{
		coeff_vector buffer(cachZSize);
		for (int y = 0; y < cachYSize; y++){
			for (int x = 0; x < cachXSize; x++) {
				m_coeff.get_data_line_z(x,y,buffer);
				m_kernel->filter_line(buffer);
				m_coeff.put_data_line_z(x,y,buffer);
			}
		}
	}

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
	
	static typename C::value_type value(const C&  coeff, const CBSplineKernel::SCache& xc, 
					    const CBSplineKernel::SCache& yc,
					    const CBSplineKernel::SCache& zc) 
	{
		U result = U();
		
		for (size_t z = 0; z < size; ++z) {
			U ry = U();
			for (size_t y = 0; y < size; ++y) {
				U rx = U();
				const U *p = &coeff(0, yc.index[y], zc.index[z]);
				for (size_t x = 0; x < size; ++x) {
					int xinx = !xc.is_mirrored ? xc.start_idx +x : xc.index[x]; 
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
		       const CBSplineKernel::SCache& xc, 
		       const CBSplineKernel::SCache& yc,
		       const CBSplineKernel::SCache& zc) 
		{
			return coeff(xc.index[0], yc.index[0], zc.index[0] ) ; 
		}
};


#ifdef __SSE2__
template <>
struct add_3d<T3DDatafield< double >, 4> {
	static double value(const T3DDatafield< double >&  coeff, 
			    const CBSplineKernel::SCache& xc, 
			    const CBSplineKernel::SCache& yc,
			    const CBSplineKernel::SCache& zc); 
	
};
#endif

template <typename T>
T  T3DConvoluteInterpolator<T>::operator () (const C3DFVector& x) const
{
	typedef typename TCoeff3D::value_type U; 
	
	(*m_kernel)(x.x, m_x_cache);
	(*m_kernel)(x.y, m_y_cache);
	(*m_kernel)(x.z, m_z_cache);	
	
	U result = U();
	
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



