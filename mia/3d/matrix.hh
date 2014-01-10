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
#ifndef __mia_3d_matrix_hh
#define __mia_3d_matrix_hh

#include <memory>

#include <mia/3d/vector.hh>
#include <mia/core/msgstream.hh>

#include <Eigen/Core>
#include <Eigen/Eigenvalues> 

NS_MIA_BEGIN

/**
   @ingroup basic 
   \brief a simple 3x3 matrix 
   
   This si a simple implementation of a 3x3 matrix that supports the evaluation of certain 
   properties and operations with vectors 

   \tparam T the data type of the elements of the matrix 
 */

template <typename T> 
class T3DMatrix: public T3DVector< T3DVector<T> > {

private: 
	typedef Eigen::Matrix<T, 3, 3> EMatrix3; 
	typedef Eigen::EigenSolver<EMatrix3
>  ESolver3; 
public:  
	
	T3DMatrix() = default; 
	T3DMatrix(const T3DMatrix<T>& o); 

	T3DMatrix<T>& operator = (const T3DMatrix<T>& o); 


	/**
	   Create a diagonal matrix 
	   \param value the value to set the diagonal elements to 
	   \returns a diagonal matrix with the gibe diagonal 
	*/
	static T3DMatrix<T> diagonal(T value); 

	/**
	   Create a diagonal matrix 
	   \param values the values to set the diagonal elements to a(0,0) = values.x, a(1,1) = values.y, ...
	   \returns a diagonal matrix with the gibe diagonal 
	*/
	static T3DMatrix<T> diagonal(const T3DVector<T>& values); 


	/**
	   Construct a matrix by copying from a differenty typed matrix 
	   \tparam I the element type of the original matrix 
	   \param o the matrix to be copied 
	 */
	template <typename I>
	T3DMatrix(const T3DMatrix<I>& o); 
	

	/**
	   Construct the matrix by giving a 3D vector of 3D vectors 
	   \remark This is needed to make transparent use of the T3DVector operators 
	   \param other  the input matrix 
	*/
	T3DMatrix(const T3DVector< T3DVector<T> >& other); 


	/**
	   Construct the matrix by giving the rows as 3D vectors 
	   \param x 1st row 
	   \param y 2st row 
	   \param z 3rd row 
	*/
	T3DMatrix(const T3DVector< T >& x, const T3DVector< T >& y, const T3DVector< T >& z ); 
	
	/**
	   inplace subtract 
	   \param other 
	   \returns 
	 */
	T3DMatrix<T>& operator -= (const T3DMatrix<T>& other);
	

	/**
	   print the matrix to an ostream 
	   \param os the output stream 
	*/
	void print( std::ostream& os) const; 


	/**
	   \returns the transposed of this matrix 
	 */
	T3DMatrix<T>  transposed()const; 


	/**
	   \returns the determinat of the matrix 
	 */
	T get_det()  const; 
	

	/**
	   \returns the rank of the matrix 
	 */
	int get_rank()const;
	
	/** calculated the eigenvalues of the matrix using the caracteristic polynome, and
	    Cardans formula 
	    \retval result stores the three eigenvalues, interprete dependend on returns
     
	    \returns 1 one real, two complex eigenvalues, real part = result->y, imaginary part = result->z
	             2 three real eigenvalues, at least two are equal
		     3 three distinct real eigenvalues
	*/	     

	int get_eigenvalues(C3DFVector& v)const; 

	/** Calculate the eigenvector to a given eigenvalues. If the eigenvalue is complex, the 
	    matrix has to be propagated to a complex one using the type converting copy constructor
	    \param i number of eigenvector 
	    \returns the requested eigenvector
	 */
	C3DFVector get_eigenvector(int i)const; 


	/// The unity matrix 
	static const T3DMatrix _1; 

	/// The zero matrix 
	static const T3DMatrix _0; 

private:
	void evaluate_ev() const; 
		
	mutable std::unique_ptr<ESolver3> m_esolver; 
	mutable int m_ev_order[3]; 
}; 


/// a simple 3x3 matrix with single precision floating point values 
typedef T3DMatrix<float> C3DFMatrix; 

/// a simple 3x3 matrix with double precision floating point values 
typedef T3DMatrix<double> C3DDMatrix; 


template <typename T> 
const T3DMatrix<T> T3DMatrix<T>::_1(T3DVector< T >(1,0,0), 
				 T3DVector< T >(0,1,0),
				 T3DVector< T >(0,0,1));

template <typename T> 
const T3DMatrix<T> T3DMatrix<T>::_0 = T3DMatrix<T>();

template <typename T> 
T3DMatrix<T>::T3DMatrix(const T3DMatrix<T>& o):
	T3DVector< T3DVector<T> >
(o.x, o.y, o.z)
{
}


template <typename T> 
T3DMatrix<T>& T3DMatrix<T>::operator = (const T3DMatrix<T>& o)
{
	this->x = o.x; 
	this->y = o.y; 
	this->z = o.z; 

	this->m_esolver.reset(nullptr); 
	return *this; 

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
template <typename I>
T3DMatrix<T>::T3DMatrix(const T3DMatrix<I>& o):
	T3DVector<T3DVector<T> >(T3DVector<T>(o.x), 
				 T3DVector<T>(o.y), 
				 T3DVector<T>(o.z))
{
}

template <typename T> 
T3DMatrix<T>::T3DMatrix(const T3DVector< T3DVector<T> >& other):
	T3DVector<T3DVector<T> >(other.x, other.y, other.z)
{
}

template <typename T> 
T3DMatrix<T>::T3DMatrix(const T3DVector< T >& x, const T3DVector< T >& y, const T3DVector< T >& z ):
	T3DVector<T3DVector<T> >(x, y, z)
{
}
template <typename T> 
void T3DMatrix<T>::print( std::ostream& os) const
{
	os << "<" << this->x << "; " << this->y << "; " << this->z << " >"; 
}

template <typename T> 
std::ostream& operator << (std::ostream& os, const T3DMatrix<T>& m) 
{
	m.print(os); 
	return os; 
}

template <typename T> 
T3DMatrix<T>& T3DMatrix<T>::operator -= (const T3DMatrix<T>& o)
{
	this->x -= o.x; 
	this->y -= o.y; 
	this->z -= o.z; 
	return *this; 
}

template <typename T> 
T3DMatrix<T>  T3DMatrix<T>::transposed()const
{
	return T3DMatrix<T>(T3DVector<T>(this->x.x, this->y.x, this->z.x), 
			    T3DVector<T>(this->x.y, this->y.y, this->z.y), 
			    T3DVector<T>(this->x.z, this->y.z, this->z.z)); 
}

template <typename T> 
T3DVector<T> operator * (const T3DMatrix<T>& m, const T3DVector<T>& x)
{
	return T3DVector<T>(dot(m.x, x), dot(m.y, x), dot(m.z, x)); 
}

template <typename T> 
T3DVector<T> operator * (const T3DVector<T>& x, const T3DMatrix<T>& m )
{
	return T3DVector<T>(m.x.x * x.x + m.y.x * x.y + m.z.x * x.z, 
			    m.x.y * x.x + m.y.y * x.y + m.z.y * x.z,
			    m.x.z * x.x + m.y.z * x.y + m.z.z * x.z);
}

template <typename T> 
T3DMatrix<T> operator * (const T3DMatrix<T>& m, const T3DMatrix<T>& x  )
{
	return T3DMatrix<T>(T3DVector<T>(m.x.x * x.x.x + m.x.y * x.y.x + m.x.z * x.z.x, 
					 m.x.x * x.x.y + m.x.y * x.y.y + m.x.z * x.z.y, 
					 m.x.x * x.x.z + m.x.y * x.y.z + m.x.z * x.z.z), 
			    T3DVector<T>(m.y.x * x.x.x + m.y.y * x.y.x + m.y.z * x.z.x, 
					 m.y.x * x.x.y + m.y.y * x.y.y + m.y.z * x.z.y, 
					 m.y.x * x.x.z + m.y.y * x.y.z + m.y.z * x.z.z), 
			    T3DVector<T>(m.z.x * x.x.x + m.z.y * x.y.x + m.z.z * x.z.x, 
					 m.z.x * x.x.y + m.z.y * x.y.y + m.z.z * x.z.y, 
					 m.z.x * x.x.z + m.z.y * x.y.z + m.z.z * x.z.z));
}

template <typename T> 
void T3DMatrix<T>::evaluate_ev() const
{
	EMatrix3  matrix; 
	matrix << this->x.x, this->x.y, this->x.z, /**/ this->y.x, this->y.y, this->y.z, /**/  this->z.x, this->z.y, this->
z.z; 
	m_esolver.reset( new ESolver3(matrix, true)); 

	auto eval = m_esolver->eigenvalues(); 

	// check if there are complex eigenvalues
	bool complex = false; 
	for (int i = 0; i < 3 && !complex; ++i) {
		if (eval[i].imag() != 0.0) {
			complex = true; 
		}
	}
	
	if (complex) {
		for (int i = 0; i < 3; ++i) {
			if (eval[i].imag() == 0.0) {
				m_ev_order[0] = i; 
				// in the complex case, the two complex evalues are 
				// conjugated complex 
				m_ev_order[1] = i+1 % 3; 
				m_ev_order[2] = i+1 % 3; 
				return; 
			}
		}
	}

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

template <typename T> 
int T3DMatrix<T>::get_rank()const
{
	C3DFVector ev; 
	auto type = this->get_eigenvalues(ev); 
	cvdebug()<< "Matrix = "<< *this <<", Rank: eigenvalues: " << ev << "\n"; 
	
	switch (type) {
	case 1: return (ev.x != 0.0) ? 3 : 2; 
	case 3: return (ev.z != 0.0) ? 3 : 2; 
	default: {
		int rank = 0; 
		if (ev.x != 0.0)  
			rank++; 
		if (ev.y != 0.0)  
			rank++; 
		if (ev.z != 0.0)  
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
	if (!m_esolver) 
		evaluate_ev(); 

	auto eval = m_esolver->eigenvalues(); 

	result.x = eval[m_ev_order[0]].real(); 
	
	if (m_ev_order[1] == m_ev_order[2]) {
		// complex case
		result.y = eval[m_ev_order[1]].real(); 
		result.y = eval[m_ev_order[2]].imag(); 
		return 1;
	}

	result.y = eval[m_ev_order[1]].real(); 
	result.z = eval[m_ev_order[2]].real();

	if (result.x == result.y || result.y == result.z) 
		return 2; 
	return 3; 
}

template <typename T> 
C3DFVector T3DMatrix<T>::get_eigenvector(int i)const
{
	if (!m_esolver) 
		evaluate_ev(); 
	

	const auto evec = m_esolver->eigenvectors().col
(m_ev_order[i]); 
	return C3DFVector(evec(0).real(), evec(1).real(), evec(2).real()); 
}




NS_MIA_END

#endif
