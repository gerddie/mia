/*
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

#if 0
template <typename T>
T T3DInterpolator<T>::operator () (const C3DFVector& x) const
{
	return T(); 
}
#endif

template <typename T>
T3DDirectInterpolator<T>::T3DDirectInterpolator(const T3DDatafield<T>& data):
       _M_data(data)
{
}

template <typename T>
T3DNNInterpolator<T>::T3DNNInterpolator(const T3DDatafield<T>& image):
	T3DDirectInterpolator<T>(image)
{
}

template <typename T>
T
T3DNNInterpolator<T>::operator () (const C3DFVector& x)const
{
	C3DBounds ix( size_t(x.x + 0.5f), size_t(x.y + 0.5f), size_t(x.z + 0.5f));
	if (ix.x < this->data().get_size().x && 
	    ix.y < this->data().get_size().y && 
	    ix.z < this->data().get_size().z)
		return this->data()(ix);
	else
		return T();
}

template <typename T>
T3DTrilinearInterpolator<T>::T3DTrilinearInterpolator(const T3DDatafield<T>& image):
	T3DDirectInterpolator<T>(image), 
	_M_xy(image.get_plane_size_xy()), 
	_M_size(image.get_size()),
	_M_sizeb(image.get_size())
		
{
	_M_sizeb.x -= 1.0f;
	_M_sizeb.y -= 1.0f; 
	_M_sizeb.z -= 1.0f; 
}

template <typename T> 
struct trinlin_dispatch {
	static T apply(const T3DDatafield<T>& data, const C3DFVector& p, const C3DFVector& sizeb) {
	// Calculate the coordinates and the distances
		const float x  = floorf(p.x);
		const float fx = p.x - x;
		const float dx = 1.0f - fx;
		
		const float y = floorf(p.y);
		const float fy = p.y - y;
		const float  dy = 1.0f - fy;
		
		const float z = floorf(p.z);
		const float fz = p.z - z;
		const float dz = 1.0f - fz;
		
		if ( x < sizeb.x && y  < sizeb.y && z < sizeb.z && x > 0.0 && y  > 0.0 && z > 0.0 ) {
			const size_t ux = data.get_size().x; 
			const T *ptr = &data[size_t(x + data.get_size().x * ( y +  data.get_size().y * z ))];
			const T *ptr_h = &ptr[ux];
			const T *ptr2 = &ptr[data.get_size().x * data.get_size().y];
			const T *ptr2_h = &ptr2[ux];
			return T (dz * ( dy * (dx * ptr[0]    + fx * ptr[1]) + 
					 fy * (dx * ptr_h[0]  + fx * ptr_h[1])) +  
				  fz * ( dy * (dx * ptr2[0]   + fx * ptr2[1]) + 
					 fy * (dx * ptr2_h[0] + fx * ptr2_h[1])));
		} else {
			const size_t ux = (size_t)x, uy = (size_t)y, uz = (size_t)z;  
			const T  a1 = T(dx * data(ux  , uy  , uz  ) + fx * data(ux+1, uy  , uz  ));
			const T  a3 = T(dx * data(ux  , uy+1, uz  ) + fx * data(ux+1, uy+1, uz  ));
			const T  a5 = T(dx * data(ux  , uy  , uz+1) + fx * data(ux+1, uy  , uz+1));
			const T  a7 = T(dx * data(ux  , uy+1, uz+1) + fx * data(ux+1, uy+1, uz+1));
			register T b1 = T(dy * a1 + fy * a3);
			register T b2 = T(dy * a5 + fy * a7);
			return  T(dz * b1 + fz * b2);
		}
		
	}
};

// for booleans we go the slow way ...
template <> 
struct trinlin_dispatch<bool> {
	static bool apply(const T3DDatafield<bool>& data, const C3DFVector& p, const C3DFVector& /*sizeb*/) {
		const float x  = floorf(p.x);
		const float fx = p.x - x;
		const float dx = 1.0f - fx;
		
		const float y = floorf(p.y);
		const float fy = p.y - y;
		const float  dy = 1.0f - fy;
		
		const float z = floorf(p.z);
		const float fz = p.z - z;
		const float dz = 1.0f - fz;
		
		const size_t ux = (size_t)x, uy = (size_t)y, uz = (size_t)z;  
		const float  a1 = dx * data(ux  , uy  , uz  ) + fx * data(ux+1, uy  , uz  );
		const float  a3 = dx * data(ux  , uy+1, uz  ) + fx * data(ux+1, uy+1, uz  );
		const float  a5 = dx * data(ux  , uy  , uz+1) + fx * data(ux+1, uy  , uz+1);
		const float  a7 = dx * data(ux  , uy+1, uz+1) + fx * data(ux+1, uy+1, uz+1);
		const float  b1 = dy * a1 + fy * a3;
		const float  b2 = dy * a5 + fy * a7;
		return  dz * b1 + fz * b2 > 0.5;
		
	}
};
	
template <typename T>
T  T3DTrilinearInterpolator<T>::operator () (const C3DFVector& p)const
{
	return trinlin_dispatch<T>::apply(this->data(), p, this->_M_sizeb); 
}


template <class T>
struct min_max {
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
struct min_max<T3DVector<T> > {
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
	_M_coeff(image.get_size()), 
	_M_size2(image.get_size() + image.get_size() - C3DBounds(2,2,2)),
	_M_kernel(kernel),
	_M_x_index(kernel->size()),
	_M_y_index(kernel->size()),
	_M_z_index(kernel->size()),
	_M_x_weight(kernel->size()),
	_M_y_weight(kernel->size()),
	_M_z_weight(kernel->size())
{
	min_max<T>::get(image, &_M_min, &_M_max);
	
	// copy the data
	std::copy(image.begin(), image.end(), _M_coeff.begin());
	
	int cachXSize = image.get_size().x;	
	int cachYSize = image.get_size().y;
	int cachZSize = image.get_size().z;
	
	{
		coeff_vector buffer(cachXSize);
		for (int z = 0; z < cachZSize; z++){
			for (int y = 0; y < cachYSize; y++) {
				_M_coeff.get_data_line_x(y,z,buffer);
				_M_kernel->filter_line(buffer);
				_M_coeff.put_data_line_x(y,z,buffer);
			}
		}
	}
	
	{
		coeff_vector buffer(cachYSize);
		for (int z = 0; z < cachZSize; z++){
			for (int x = 0; x < cachXSize; x++) {
				_M_coeff.get_data_line_y(x,z,buffer);
				_M_kernel->filter_line(buffer);
				_M_coeff.put_data_line_y(x,z,buffer);
			}
		}
	}
	
	{
		coeff_vector buffer(cachZSize);
		for (int y = 0; y < cachYSize; y++){
			for (int x = 0; x < cachXSize; x++) {
				_M_coeff.get_data_line_z(x,y,buffer);
				_M_kernel->filter_line(buffer);
				_M_coeff.put_data_line_z(x,y,buffer);
			}
		}
	}

}

template <typename T>
T3DConvoluteInterpolator<T>::~T3DConvoluteInterpolator()
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
	
	static typename C::value_type value(const C&  coeff, const std::vector<double>& xweight, 
					    const std::vector<double>& yweight,
					    const std::vector<double>& zweight,
					    const std::vector<int>& xindex, 
					    const std::vector<int>& yindex, 				
					    const std::vector<int>& zindex) 
	{
		U result = U();
		
		for (size_t z = 0; z < size; ++z) {
			U ry = U();
			for (size_t y = 0; y < size; ++y) {
				U rx = U();
				const U *p = &coeff(0, yindex[y], zindex[z]);
				
				for (size_t x = 0; x < size; ++x) {
					rx += xweight[x] * p[xindex[x]];
				}
				ry += yweight[y] * rx; 
			}
			result += zweight[z] * ry; 
		}
		return result; 
	}
};


template <typename T>
T  T3DConvoluteInterpolator<T>::operator () (const C3DFVector& x) const
{
	typedef typename TCoeff3D::value_type U; 
	
	// cut at boundary
	if (x.x < 0.0 || x.y < 0.0 || x.z < 0.0 ||
	    x.x >= _M_coeff.get_size().x || x.y >= _M_coeff.get_size().y || x.z >= _M_coeff.get_size().z)
		return T();
	
	(*_M_kernel)(x.x, _M_x_weight, _M_x_index);
	(*_M_kernel)(x.y, _M_y_weight, _M_y_index);
	(*_M_kernel)(x.z, _M_z_weight, _M_z_index);	
	
	mirror_boundary_conditions(_M_x_index, _M_coeff.get_size().x, _M_size2.x);
	mirror_boundary_conditions(_M_y_index, _M_coeff.get_size().y, _M_size2.y);
	mirror_boundary_conditions(_M_z_index, _M_coeff.get_size().z, _M_size2.z);


	U result = U();
	
	switch (_M_kernel->size()) {
	case 2: result = add_3d<TCoeff3D,2>::value(_M_coeff, _M_x_weight, _M_y_weight, _M_z_weight, 
					    _M_x_index, _M_y_index, _M_z_index); break; 
	case 3: result = add_3d<TCoeff3D,3>::value(_M_coeff, _M_x_weight, _M_y_weight, _M_z_weight, 
					    _M_x_index, _M_y_index, _M_z_index); break; 
	case 4: result = add_3d<TCoeff3D,4>::value(_M_coeff, _M_x_weight, _M_y_weight, _M_z_weight, 
					    _M_x_index, _M_y_index, _M_z_index); break; 
	case 5: result = add_3d<TCoeff3D,5>::value(_M_coeff, _M_x_weight, _M_y_weight, _M_z_weight, 
					    _M_x_index, _M_y_index, _M_z_index); break; 
	case 6: result = add_3d<TCoeff3D,6>::value(_M_coeff, _M_x_weight, _M_y_weight, _M_z_weight, 
					    _M_x_index, _M_y_index, _M_z_index); break; 
	default: {
		/* perform interpolation */
		for (size_t z = 0; z < _M_kernel->size(); ++z) {
			U ry = U();
			for (size_t y = 0; y < _M_kernel->size(); ++y) {
				U rx = U();
				const typename  TCoeff3D::value_type *p = &_M_coeff(0, _M_y_index[y], _M_z_index[z]);
				
				for (size_t x = 0; x < _M_kernel->size(); ++x) {
					rx += _M_x_weight[x] * p[_M_x_index[x]];
				}
				ry += _M_y_weight[y] * rx; 
			}
			result += _M_z_weight[z] * ry; 
		}
	}
	} // end switch 
	
	bounded<U, T>::apply(result, _M_min, _M_max);
	
	return round_to<U, T>::value(result); 
}

NS_MIA_END



