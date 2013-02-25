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

#include <mia/3d/vector.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

/**
   @ingroup basic 
   \brief a simple 3x3 matrix 
   
   This si a simple implementation of a 3x3 matrix that supports the evaluation of certain 
   properties and operations with vectors 

   \tparam T the data type of the elements of the matrix 
 */

template <typename T> 
struct T3DMatrix: public T3DVector< T3DVector<T> > {
	
	T3DMatrix() = default; 
	T3DMatrix(const T3DMatrix<T>& o) = default; 


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
	    \param[in] ev the eigenvalue
	    \param[out] v the estimated eigenvector 
	    \returns 0 eigenvector is valid
	             2 no eigenvector found
	 */
	int get_eigenvector(float ev, C3DFVector& v)const; 


	/// The unity matrix 
	static const T3DMatrix _1; 

	/// The zero matrix 
	static const T3DMatrix _0; 

}; 


/// a simple 3x3 matrix 
typedef T3DMatrix<float> C3DFMatrix; 


template <typename T> 
const T3DMatrix<T> T3DMatrix<T>::_1(T3DVector< T >(1,0,0), 
				 T3DVector< T >(0,1,0),
				 T3DVector< T >(0,0,1));

template <typename T> 
const T3DMatrix<T> T3DMatrix<T>::_0 = T3DMatrix<T>();


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
	T3DVector<T3DVector<T> >(o.x, o.y, o.z)
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
	os << "<" << this->x << ", " << this->y << ", " << this->z << " >"; 
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
T3DVector<T> operator * (const T3DVector<T>& x, const T3DMatrix<T>& m)
{
	return T3DVector<T>(dot(m.x, x), dot(m.y, x), dot(m.z, x)); 
}

template <typename T> 
T3DVector<T> operator * (const T3DMatrix<T>& m, const T3DVector<T>& x  )
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
int T3DMatrix<T>::get_rank()const
{
	C3DFVector ev; 
	this->get_eigenvalues(ev); 
	cvdebug()<< "Matrix = "<< *this <<", Rank: eigenvalues: " << ev << "\n"; 
	int rank = 0; 
	if (ev.x != 0.0)  
		rank++; 
	if (ev.y != 0.0)  
		rank++; 
	if (ev.z != 0.0)  
		rank++; 
	return rank; 
}

inline double cubrt(double a) 
{
	if ( a == 0.0 )
		return 0.0;
	return  a > 0.0 ? pow(a,1.0/3.0) : - pow(-a, 1.0/ 3.0); 
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
	int retval = 0; 
	
	double t = - get_det();
	double s =   this->x.x * this->y.y + this->z.z * this->y.y + this->x.x * this->z.z -
		this->x.y * this->y.x - this->x.z * this->z.x - this->y.z * this->z.y;
	
	double r =  - this->x.x - this->y.y - this->z.z;
	
	//	 a = 1; 
	
	double p = s - r * r / 3.0; 
	double q = ( 27.0 * t - 9.0 * r * s + 2.0 * r * r * r ) / 27.0;
	
	double diskr = q *q / 4.0 + p * p * p / 27.0;
	
	cvdebug() << "discr =" << diskr << "\n"; 
	if ( diskr > 1e-6 ) {
		// complex solution
		double sqrt_discr = sqrt(diskr);
		double u = cubrt( - q/2.0 + sqrt_discr );
		double v = cubrt( - q/2.0 - sqrt_discr );
		result.x = u + v - r / 3.0;
		result.y = -(u + v) / 2.0 - r / 3.0; // real part 
		result.z = (u - v)/2.0 * sqrt(3.0f);    // imag part
		return 1;
		
	}

	std::vector<double> res(3); 
	if ( diskr < -1e-6) {
		double rho = sqrt(-p*p*p/ 27.0); 
		double cphi = - q / ( 2.0 * rho); 
		double phi = acos(cphi)/3.0;
		double sqrt_p = 2 * cubrt(rho);
		res[0] = sqrt_p * cos(phi)- r/3.;
		res[1] = sqrt_p * cos(phi + M_PI * 2.0 / 3.0) - r/3.; 
		res[2] = sqrt_p * cos(phi + M_PI * 4.0 / 3.0) - r/3.; 
		retval = 3;		
	} else  { // at least two values are equal, all real  
		double u = cubrt( - q/2.0 );
		res[0] = 2.0 * u - r / 3.0;
		res[1] =  - u - r / 3.0; 
		res[2] = res[1]; 
		retval = 2; 
	}
	
	std::sort(res.begin(), res.end(),  [](double x, double y){ return std::fabs(x) > std::fabs(y);}); 
	
	for_each(res.begin(), res.end(), [](double& x){ if (std::fabs(x) < 1e-12) x = 0.0;}); 

	result.x = res[0]; 
	result.y = res[1]; 
	result.z = res[2];
	return retval;

}

/** solve a 2x2 system of equations 
      a11 * x1 + a12 * x2 = b1
      a21 * x1 + a22 * x2 = b2
    \param a11
    \param a12
    \param b1
    \param a21
    \param a22
    \param b2    
    \param x1
    \param x2
    \returns true if system was solved, false otherwise
*/
template<class T> 
bool solve_2x2(T a11, T a12,T b1,T a21, T a22,T b2,T *x1,T *x2)
{
	T h1 = a11 * a22 - a12 * a21; 
	if (h1 == T())
		return false;
	
	*x1 = (b1 * a22 - b2 * a12) / h1; 
	*x2 = (b2 * a11 - b1 * a21) / h1; 
	return true;
}

/** some struct to help solving the 3x3 Matrix eigenvalue problem */
struct solve_lines_t {
	int a,b; 
};


template <typename T> 
int T3DMatrix<T>::get_eigenvector(float ev, C3DFVector& v)const
{
	const solve_lines_t l[3] = { {0,1}, {1,2}, {2,0}};
	//	T b1,b2,a11,a12,a21,a22; 
	if (ev == 0.0) {
		return 1;
	}
	
	T3DMatrix<T> M = *this - T3DMatrix<T>::diagonal(ev);

	float x = std::abs(M.x.x)+std::abs(M.y.x)+std::abs(M.z.x);
	float y = std::abs(M.x.y)+std::abs(M.y.y)+std::abs(M.z.y);
	float z = std::abs(M.x.z)+std::abs(M.y.z)+std::abs(M.z.z);
	
	if (x+y+z == 0.0) {
		v = T3DVector<T>(1,0,0);
		return 0;
	}

	T3DVector<int> col;
	T *rx;
	T *ry;

	// thats tricky: 
	// 1st col index is 1st column in solver 
	// 2nd col index is 2nd column in solver
	// 3th col index is right side 
	// the respective result value is 0=x,1=y,2=z
	// the right side presenting value of result is preset with 1.0
	// the others get a pointer
	
	if (x < y) {
		if (x < z){
			col = T3DVector<int>(1,2,0);
			rx = &v.y;
			ry = &v.z;
			v.x = 1.0;
		}else{
			col = T3DVector<int>(0,1,2);
			rx = &v.x;
			ry = &v.y;
			v.z = 1.0; 
		}
	}else{
		if (y < z){
			col = T3DVector<int>(0,2,1);
			rx = &v.x;
			ry = &v.z;
			v.y = 1.0; 

		}else{
			col = T3DVector<int>(0,1,2);			
			rx = &v.x;
			ry = &v.y;
			v.z= 1.0;
		}
	}
	
	bool good = false; 
	for (int i = 0; i < 3 && !good; i++) {
		good = solve_2x2(M[l[i].a][col.x],M[l[i].a][col.y],-M[l[i].a][col.z],
				 M[l[i].b][col.x],M[l[i].b][col.y],-M[l[i].b][col.z],
				 rx,ry); 
	}
	// seems there is no solution
	if (!good) 
		return 2; 
	
	
	if ((M * v).norm2() > 1e-5) {
		
		// a solution for only two rows is not a solution
		// but there is no better
		fprintf(stderr,"WARNING: rank of A-ev*I\n numerical > 2");
		return 0;
	}
	v /= v.norm();	
	return 0;
}




NS_MIA_END

#endif
