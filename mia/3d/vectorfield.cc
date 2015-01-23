#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>


#include <mia/core/threadedmsg.hh>
#include <mia/3d/vectorfield.hh>
#include <mia/3d/datafield.cxx>
#include <mia/2d/datafield.cxx>
#include <mia/3d/iterator.cxx>
#include <mia/2d/iterator.cxx>

#ifdef __SSE2__
#include <emmintrin.h>
#endif 

NS_MIA_BEGIN


EXPORT_3D C3DFVectorfield& operator += (C3DFVectorfield& a, const C3DFVectorfield& b)
{
	assert(a.get_size() == b.get_size());

	C3DFVectorfield help(a.get_size());
	std::copy(a.begin(), a.end(), help.begin());

	auto callback = [&a, &b, &help](const tbb::blocked_range<size_t>& range) {
		
		for (auto z = range.begin(); z != range.end();  ++z)  {
			C3DFVectorfield::iterator i = a.begin_at(0,0,z);
			C3DFVectorfield::const_iterator u = b.begin_at(0,0,z);
			for (size_t y = 0; y < a.get_size().y; ++y)  {
				for (size_t x = 0; x < a.get_size().x; ++x, ++i, ++u)  {
					C3DFVector xi = C3DFVector(x, y, z) - *u;
					*i = help.get_interpol_val_at(xi) +  *u;
				}
			}
		}
	}; 
	tbb::parallel_for( tbb::blocked_range<size_t>(0, a.get_size().z, 1), callback); 
	return a;
}

void C3DFVectorfield::update_as_inverse_of(const C3DFVectorfield& other, float tol, int maxiter)
{
	assert(get_size() == other.get_size()); 

	float tol2 = tol * tol; 

	C3DLinearVectorfieldInterpolator t(other); 

	tbb::blocked_range<size_t> range(0, get_size().z, 1); 
	
	auto callback = [this, &t, tol2, maxiter](const tbb::blocked_range<size_t>& range) {
		for (auto z = range.begin(); z != range.end();  ++z)  {
			auto i = begin_at(0,0,z);
			for (size_t y = 0; y < get_size().y; ++y)  {
				for (size_t x = 0; x < get_size().x; ++x, ++i)  {
					int iter = 0; 
					C3DFVector pos(x,y,z);
					float dnorm = 0.0; 
					while (iter++ < maxiter) {
						C3DFVector r = pos - *i; 
						C3DFVector ov = t(r); 
						C3DFVector i_delta = r - ov - pos; 
						dnorm = i_delta.norm2(); 
						if ( dnorm < tol2)
							break; 
						*i += 0.5 * i_delta; 
					}
				}
			}
		}
	};
	
	tbb::parallel_for( range, callback ); 
}


#ifdef __SSE__
C3DSSELinearVectorfieldInterpolator::C3DSSELinearVectorfieldInterpolator(const C3DFVectorfield& field):
        m_field(field)
{
	
}

C3DFVector C3DSSELinearVectorfieldInterpolator::operator()(const C3DFVector& x) const
{
	C3DFVector result = C3DFVector::_0;
	
	const C3DFVector h(floor(x.x), floor(x.y), floor(x.z)); 
	
	const C3DBounds ip(static_cast<unsigned int>(h.x), 
			   static_cast<unsigned int>(h.y), 
			   static_cast<unsigned int>(h.z));


	if (ip < m_field.get_size()) {
		size_t linear_index = ip.x + m_field.get_size().x * (ip.y  + ip.z * m_field.get_size().y); 
		
		const C3DFVector w1 = x - h;
		
		if (w1 == C3DFVector::_0) {
			// early exist for this special case
			result = m_field[linear_index]; 
		}else{
			
			// we don't generally check for boundaries, or if we are on grid. 
			// hitting the upper boundary when loading is very improbably 
			// so that tests would just slow down the execution and 
			// the interpolation weights will ensure that only the correct data 
			// is used 
			
			// also note that the 4th value of the SSE register contains garbage, 
			// i.e. loads the x value of the next vector in the field. 
			// 
			const C3DFVector w0 = C3DFVector::_1 - w1;
			
			const __m128 in000 = _mm_loadu_ps(&m_field[linear_index].x); 
			const __m128 in001 = _mm_loadu_ps(&m_field[linear_index + 1].x); 
			
			size_t linear_index_y1 = linear_index + m_field.get_size().x; 
			
			const __m128 in010 = _mm_loadu_ps(&m_field[linear_index_y1].x); 
			const __m128 in011 = _mm_loadu_ps(&m_field[linear_index_y1 + 1].x); 
			
			linear_index +=  m_field.get_plane_size_xy(); 
			const __m128 in100 = _mm_loadu_ps(&m_field[linear_index].x); 
			const __m128 in101 = _mm_loadu_ps(&m_field[linear_index + 1].x); 

			
			linear_index +=  m_field.get_size().x; 
			const __m128 in110 = _mm_loadu_ps(&m_field[linear_index].x); 
			
			// here we have to check, because otherwise we might access 
			// data outside the allocated memory. 
			++linear_index; 
			__m128 in111; 
			
			// we are somewhere inside the field, 
			if (linear_index < m_field.size() - 1) {
				in111 = _mm_loadu_ps(&m_field[linear_index].x); 
				
			} else 	if (linear_index < m_field.size()) {
				// this is really the last element, take care to not access 
				// outside the allowed range
				const C3DFVector& v = m_field[linear_index];
				float __attribute__((aligned(16))) help[4] = {v.x, v.y, v.z, 0.0f}; 
				in111 = _mm_loadu_ps(help);
			}else {
				in111 = _mm_loadu_ps(&m_field[linear_index - 1].x); 
			}
			
			const __m128 w0x = _mm_set1_ps(w0.x); 
			const __m128 w1x = _mm_set1_ps(w1.x); 
			
			const __m128 w0y = _mm_set1_ps(w0.y); 
			const __m128 w1y = _mm_set1_ps(w1.y); 
			
			const __m128 w0z = _mm_set1_ps(w0.z); 
			const __m128 w1z = _mm_set1_ps(w1.z); 

			const __m128 r = 
				w0z * (w0y * ( w0x * in000 + w1x * in001) + 
				       w1y * ( w0x * in010 + w1x * in011)) + 
				w1z * (w0y * ( w0x * in100 + w1x * in101) + 
				       w1y * ( w0x * in110 + w1x * in111)); 
			
			float __attribute__((aligned(16))) result_help[4]; 
			_mm_store_ps(result_help, r); 
			result.x = result_help[0]; 
			result.y = result_help[1]; 
			result.z = result_help[2]; 
		}
	}
	return result;
	
}
#endif 

C3DDefaultLinearVectorfieldInterpolator::C3DDefaultLinearVectorfieldInterpolator(const C3DFVectorfield& field):
        m_field(field)
{
	
}


C3DFVector C3DDefaultLinearVectorfieldInterpolator::operator ()(const C3DFVector& x) const
{
	C3DFVector result = C3DFVector::_0;
	
	const C3DFVector h(floor(x.x), floor(x.y), floor(x.z)); 
	
	const C3DBounds ip(static_cast<unsigned int>(h.x), 
			   static_cast<unsigned int>(h.y), 
			   static_cast<unsigned int>(h.z));


	if (ip < m_field.get_size()) {
		size_t linear_index = ip.x + m_field.get_size().x * (ip.y  + ip.z * m_field.get_size().y); 
		
		const C3DFVector& in000 = m_field[linear_index]; 
		
		const C3DFVector w1 = x - h;
		
		if (w1 == C3DFVector::_0) {
			// early exist for this special case
			result = in000; 
		}else{

			// we don't generally check for boundaries, or if we are on grid. 
			// hitting the upper boundary when loading is very improbably 
			// so that tests would just slow down the execution and 
			// the interpolation weights will ensure that only the correct data 
			// is used 
			
			const C3DFVector w0 = C3DFVector::_1 - w1;
			
			const C3DFVector& in001 = m_field[linear_index + 1]; 
			
			size_t linear_index_y1 = linear_index + m_field.get_size().x; 
			const C3DFVector& in010 = m_field[linear_index_y1]; 
			const C3DFVector& in011 = m_field[linear_index_y1 + 1]; 
			
			linear_index +=  m_field.get_plane_size_xy(); 
			const C3DFVector& in100 = m_field[linear_index]; 
			const C3DFVector& in101 = m_field[linear_index + 1]; 
			
			linear_index +=  m_field.get_size().x; 
			const C3DFVector& in110 = m_field[linear_index]; 
			
			// here we have to check, because otherwise we might access 
			// data outside the allocated memory. 
			++linear_index; 
			const C3DFVector& in111 = (linear_index < m_field.size()) ? 
				m_field[linear_index] : C3DFVector::_0; 
			
			// now interpolate
			result = w0.z * (w0.y * ( w0.x * in000 + w1.x * in001) + 
					 w1.y * ( w0.x * in010 + w1.x * in011)) + 
				w1.z * (w0.y * ( w0.x * in100 + w1.x * in101) + 
					w1.y * ( w0.x * in110 + w1.x * in111)); 
				
		}
	}
	return result;
}

#define INSTANCIATE(TYPE) \
	template class  T3DDatafield<TYPE>;			\
	template class  range3d_iterator<T3DDatafield<TYPE>::iterator>; \
	template class  range3d_iterator<T3DDatafield<TYPE>::const_iterator>; \
	template class  range3d_iterator_with_boundary_flag<T3DDatafield<TYPE>::iterator>; \
	template class  range3d_iterator_with_boundary_flag<T3DDatafield<TYPE>::const_iterator>; 

#define INSTANCIATE2D(TYPE)						\
	template class  EXPORT_3D T2DDatafield<TYPE>;			\
	template class  EXPORT_3D range2d_iterator<T2DDatafield<TYPE>::iterator>; \
	template class  EXPORT_3D range2d_iterator<T2DDatafield<TYPE>::const_iterator>;


INSTANCIATE2D(C3DFVector); 
INSTANCIATE2D(C3DDVector);

INSTANCIATE(C3DFVector);
INSTANCIATE(C3DDVector);
template class T3DVectorfield<C3DFVector>;
template class T3DVectorfield<C3DDVector>;



NS_MIA_END
