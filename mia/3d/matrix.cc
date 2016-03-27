/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/3d/matrix.hh>

#ifdef __GNUC__
#pragma GCC diagnostic push
#ifndef __clang__ 
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#else
#pragma clang diagnostic ignored "-Wdeprecated-register"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif 
#endif 

#include <Eigen/Core>
#include <Eigen/Eigenvalues> 

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif 


NS_MIA_BEGIN


template <typename T> 
T3DMatrix<T>::T3DMatrix():
        m_ev_type(0),
	m_evectors(3),
	m_ev_order(3)
{
}


template <typename T> 
T3DMatrix<T> T3DMatrix<T>::diagonal(T v)
{
	return T3DMatrix<T>(T3DVector< T >(v,0,0), 
			    T3DVector< T >(0,v,0),
			    T3DVector< T >(0,0,v));
}

template <typename T> 
T3DMatrix<T> T3DMatrix<T>::diagonal(const T3DVector<T>& v)
{
	return T3DMatrix<T>(T3DVector< T >(v.x,0,0), 
			    T3DVector< T >(0,v.y,0),
			    T3DVector< T >(0,0,v.z));
}

template <typename T> 
T3DMatrix<T>::T3DMatrix(const T3DVector< T3DVector<T> >& other):
	T3DVector<T3DVector<T> >(other.x, other.y, other.z),
	m_ev_type(0),
	m_evectors(3),
	m_ev_order(3)
{
}

template <typename T> 
T3DMatrix<T>::T3DMatrix(const T3DVector< T >& x, const T3DVector< T >& y, const T3DVector< T >& z ):
	T3DVector<T3DVector<T> >(x, y, z),
	m_ev_type(0),
	m_evectors(3),
	m_ev_order(3)
{
}
template <typename T> 
void T3DMatrix<T>::print( std::ostream& os) const
{
	os  << this->x << "; " << this->y << "; " << this->z; 
}

template <typename T> 
T3DMatrix<T>  T3DMatrix<T>::transposed()const
{
	return T3DMatrix<T>(T3DVector<T>(this->x.x, this->y.x, this->z.x), 
			    T3DVector<T>(this->x.y, this->y.y, this->z.y), 
			    T3DVector<T>(this->x.z, this->y.z, this->z.z)); 
}

template <typename T> 
void T3DMatrix<T>::evaluate_ev() const
{
	typedef Eigen::Matrix<T, 3, 3> EMatrix3; 
	typedef Eigen::EigenSolver<EMatrix3>  ESolver3;

	EMatrix3  matrix; 
	// fill the matrix with the 
	matrix << this->x.x, this->x.y, this->x.z, /**/
		this->y.x, this->y.y, this->y.z, /**/
		this->z.x, this->z.y, this->z.z; 

	ESolver3 esolver(matrix, true); 
	
	auto eval = esolver.eigenvalues(); 

	// check if there are complex eigenvalues
	bool complex = false; 
	for (int i = 0; i < 3 && !complex; ++i) {
		if (eval[i].imag() != 0.0) {
			complex = true; 
		}
	}
	
	if (complex) {
		m_ev_type = 1; 
		for (int i = 0; i < 3; ++i) {
			if (eval[i].imag() == 0.0) {
				m_ev_order[0] = i; 
				// in the complex case, the two complex evalues are 
				// conjugated complex 
				m_ev_order[1] = i+1 % 3; 
				m_ev_order[2] = i+1 % 3;  
			}
		}
	}else {
	       
		// not complex, just sort the eval indices
		double evnorms[3]; 
		for (int i = 0; i < 3; ++i)
			evnorms[i] = std::norm(eval(i)); 
		
		
		
		if (evnorms[0] < evnorms[1]) {
			if (evnorms[0] < evnorms[2]) {
				m_ev_order[2] = 0; 
				if (evnorms[1] < evnorms[2]) {
					m_ev_order[0] = 2; 
					m_ev_order[1] = 1; 
					
				}else{
					m_ev_order[0] = 1; 
					m_ev_order[1] = 2; 
				}
			}else {
				m_ev_order[0] = 1; 
				m_ev_order[1] = 0; 
				m_ev_order[2] = 2; 
			}
                        
		} else { 
			
			if (evnorms[0] > evnorms[2]) {
				m_ev_order[0] = 0; 
				
				if (evnorms[1] < evnorms[2]) {
					m_ev_order[1] = 2; 
					m_ev_order[2] = 1; 
				}else{
					m_ev_order[1] = 1; 
					m_ev_order[2] = 2; 
				}
				
			}else{ 
				m_ev_order[0] = 2; 
				m_ev_order[1] = 0; 
				m_ev_order[2] = 1; 
			}
		}
	}
	 
	
	// copy results over
	m_evalues.x = eval[m_ev_order[0]].real(); 
	
	if (m_ev_order[1] == m_ev_order[2]) {
		// complex case
		m_evalues.y = eval[m_ev_order[1]].real(); 
		m_evalues.z = eval[m_ev_order[2]].imag(); 
	} else {

		m_evalues.y = eval[m_ev_order[1]].real(); 
		m_evalues.z = eval[m_ev_order[2]].real();
		
		if (m_evalues.x == m_evalues.y || m_evalues.y == m_evalues.z) 
			m_ev_type = 2;
		else 
			m_ev_type = 3; 
	}
	assert(m_ev_type);
	for (int i = 0; i < 3; ++i) {
		const auto evec = esolver.eigenvectors().col(m_ev_order[i]);
		m_evectors[i] = C3DFVector(evec(0).real(), evec(1).real(), evec(2).real());
	}
}

template <typename T> 
int T3DMatrix<T>::get_rank()const
{
	
	if (!m_ev_type)
		evaluate_ev(); 
		
	switch (m_ev_type) {
	case 1: return (m_evalues.x != 0.0) ? 3 : 2; 
	case 3: return (m_evalues.z != 0.0) ? 3 : 2;
	default: {
		int rank = 0; 
		if (m_evalues.x != 0.0)  
			rank++; 
		if (m_evalues.y != 0.0)  
			rank++; 
		if (m_evalues.z != 0.0)  
			rank++; 
		return rank; 
	}

	}
}

template <class T>
T T3DMatrix<T>::get_det()  const 
{
	return  dot(this->x,T3DVector<T>(this->y.y * this->z.z - this->z.y * this->y.z, 
					 this->y.z * this->z.x - this->y.x * this->z.z, 
					 this->y.x * this->z.y - this->y.y * this->z.x)); 
}


template <typename T> 
int T3DMatrix<T>::get_eigenvalues(C3DFVector& result)const
{
	if (m_ev_type == 0) 
		evaluate_ev(); 

	result = m_evalues;
	return m_ev_type;
}

template <typename T> 
C3DFVector T3DMatrix<T>::get_eigenvector(int i)const
{
	if (m_ev_type == 0) 
		evaluate_ev(); 
	
	return m_evectors[i];
}

template class T3DMatrix<float>; 
template class T3DMatrix<double>; 

NS_MIA_END
