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

#ifndef __mia_3d_criticalpoint_h
#define __mia_3d_criticalpoint_h

#include <list>

#include <mia/3d/matrix.hh>

NS_MIA_BEGIN

/** 
    \ingroup basic 
    
    \brief A class to hold a critical point.
    This class holds a critical point in a 3D vector field 
    It consists of a localtion 
     \f$ x_0 \in \mathbb{R}^3 \f$, Matrix \f$ A \in \mathbb{R}^{3 \times 3} \f$, and
    a magnitude \f$ \gamma \in \mathbb{R} \f$.
    It is used only as an intermidiate storage. 

    @remark partially untested 
*/


class C3DCriticalPoint {
public:

	/// A list of critical points 
	typedef std::vector< C3DCriticalPoint > List;
	
	/** The standart constructor.
	    It created a critical point with a magintude of zero, at location (0,0,0) with the 
	    Zero - Matrix as phase portrait.
	*/
	
	C3DCriticalPoint();

	/** 
	    Construct a critical point at a certain location. 
	    Magnitude and pahse portrait are set to zero. 
	    \param x0_ location of the new critical point
	*/

	
	C3DCriticalPoint(const C3DFVector& x0_);
	/**
	   Construct a critical point at a certain location. 
	   Magnitude and pahse portrait are set to zero. 
	   \param x x-coordinate of the new critical point
	   \param y y-coordinate of the new critical point
	   \param z z-coordinate of the new critical point
	*/
	C3DCriticalPoint(float x,float y, float z);
	
	/**
	   Copy constructor. 
	   \param org The critical point to be copied.
	 */
	C3DCriticalPoint(const C3DCriticalPoint& org);
	
	/**
	   Assignment operator. 
	   \param org The critical point to be copied.
	 */
	C3DCriticalPoint& operator = (const C3DCriticalPoint& org);
	
	/** \retval the (read only) location of the critical point.  */
	const C3DFVector get_point()const;
	
	/**  \retval the (read only) phase portrait of the critical point. */
	const C3DFMatrix get_a()const;
	
	/** \retval the magnitude of the critical point */
	float get_gamma()const;

	/** \retval the location of the critical point \a writable */
	C3DFVector& get_point();
	
	/** \retval the phase portrait of the critical point \a writable */
	C3DFMatrix& get_a();
	
	/** set the magnitude if the critical point 
	    \param gamma_ the new magnitude of the critical point
	 */
	
	void set_gamma(float gamma_);
	
	/** return the magniture of the critical point at location \a x according to 
	    \f[ \frac {\Gamma}{ \| \mathbf{x} - \mathbf{x_0} \| ^ 2} A \dot (x - x0) \f]
	    \param x location weher to evaluate the vector field 
	    \retval value of vector field created by this crtitical point at \a x
	*/
	C3DFVector at(const C3DFVector& x) const;
	
	C3DFVector at_alt(const C3DFVector& x) const;

	/** compare two critical points 
	- they are equal, if magnitude, phase portrait, and location are equal
	*/
	bool operator ==(const C3DCriticalPoint& cp) const;	
#ifdef UGLY_HACK	
	bool operator < (const C3DCriticalPoint& cp) const;	
#endif	


private:
	float gamma;
	C3DFVector x0;
	C3DFMatrix A;
};

/// typedef for convinience 
typedef C3DCriticalPoint::List C3DCriticalPointList;


typedef std::complex<float> fcomplex; 
typedef T3DVector<fcomplex> C3DCVector; 

/** 
    \ingroup basic 
    \brief A class to hold a criticalpoint with eigenvalues and eigenvectors.
    
    @remark untested 
*/

class C3DCriticalPointEigen {
	/// where is the critical point 
	C3DFVector location; 


	/// what is its phase portrait
	C3DFMatrix portrait; 
	
	/// first eigenvalue (always real)
	float eval1;
	/// second real eigenvalue, or real part of a the conjugated complex eigenvalues 
	float eval2;
	/// third real eigenvalue, or imaginary part of a the conjugated complex eigenvalues 	
	float eval3;
	
	/// first eigenvector (always real)
	C3DFVector evec1;
	
	/// second real eigenvector, or real part of a the conjugated complex eigenvectors
	C3DFVector evec2; 
	
	/// third real eigenvector, or imaginary part of a the conjugated complex eigenvectors
	C3DFVector evec3;
	
public:	
	/// types of critical points
	enum EVType {ev_zero, /*!< all eigenvalues are zero (more a placeholder) */ 
		     ev_real,  /*!< three real distinct eigenvalues */
		     ev_real_two_equal, /*!< three real eigenvalues, two equal */
		     ev_real_three_equal, /*!< three real eigenvalues, all equal */
		     ev_complex };   /*!< one real eigenvalue, two conjugated complex */
	
	/** Construtor to create a critical point with evaluated eigenvalues and - vectors from 
	    a critical point.
	    \param cp the critical point to construct from */
	C3DCriticalPointEigen(C3DCriticalPoint cp);
	
	/** Construtor to create a critical point with evaluated eigenvalues and - vectors .
	    \param l location of the critical point
	    \param m phase portrait of the critical point
	*/
	C3DCriticalPointEigen(const C3DFVector& l, const C3DFMatrix& m);
	
	/** Contruct a critical point from given data. 
	   \param location_ location of critical point
	   \param portrait_ phase portrait of critical point
	   \param eval1 first eigenvalue (always real)
	   \param eval2 second real eigenvalue or real part of conjugated complex ones
	   \param eval3 third real eigenvalue or imaginary part of conjugated complex ones
	   \param evec1 first eigenvector (always real)
	   \param evec2 second real eigenvector or real part of conjugated complex ones
	   \param evec3 third real eigenvector or imaginary part of conjugated complex ones
	   \param type how to interpret the second and third eigenvalues/-vectors
	 */
	C3DCriticalPointEigen(const C3DFVector& location_,
			    const C3DFMatrix& portrait_,
			    float eval1,float eval2,float eval3,
			    const C3DFVector& evec1,const C3DFVector& evec2, const C3DFVector& evec3,
			    EVType type);
			    
	
	/** \retval type of eigenvalues and eigenvectors */
	EVType get_type()const;
	
	/** \retval get first eigenvalue */
	float get_eval1()const; 
	/** \retval get second eigenvalue as real 
	    \remark use only for loading and storing
	*/	
	float get_eval2()const; 
	/** \retval get second eigenvalue as real 
	    \remark use only for loading and storing
	*/	
	float get_eval3()const;
	
	/** \retval get second eigenvalue as real 
	    \remark asserts whether eigenvalue is really real
	*/	
	float get_real_eval2()const; 
	/** \retval get third eigenvalue as real 
	    \remark asserts whether eigenvalue is really real
	*/	
	float get_real_eval3()const;
	
	/** \retval get second eigenvalue as complex
	    \remark asserts whether eigenvalue is really complex
	*/	
	fcomplex get_complex_eval2()const; 
	
	/** \retval get third eigenvalue as complex
	    \remark asserts whether eigenvalue is really complex
	*/	
	fcomplex get_complex_eval3()const;
	
	/** \retval a copy of the phase portrait
	 */
	const C3DFMatrix get_portrait()const; 
	
	/** \retval a copy of the location 
	 */
	const C3DFVector get_location()const; 
	
	/** \retval a copy of the first eigenvector 
	 */
	const C3DFVector get_evect1()const; 
	
	/** \retval a copy of the second eigenvector as real
	    \remark use only for loading and storing
	 */
	const C3DFVector get_evect2()const; 
	/** \retval a copy of the third eigenvector as real
	    \remark use only for loading and storing
	 */
	const C3DFVector get_evect3()const; 

	
	const C3DFVector get_real_evect2()const; 
	const C3DFVector get_real_evect3()const; 
	const C3DCVector get_complex_evect2()const; 
	const C3DCVector get_complex_evect3()const; 
	

private:
	EVType type;
	bool estimate();
};

typedef std::vector< C3DCriticalPointEigen > C3DCriticalPointEigenList;

inline C3DCriticalPointEigen::EVType C3DCriticalPointEigen::get_type()const
{
	return type; 
}
inline float C3DCriticalPointEigen::get_eval1()const
{
	return eval1; 
}     
inline float C3DCriticalPointEigen::get_eval2()const 
{
	return eval2; 
}     
inline float C3DCriticalPointEigen::get_eval3()const
{
	return eval3; 
}     

inline float C3DCriticalPointEigen::get_real_eval2()const
{
	assert(type != ev_complex);
	return eval2; 
}     
inline float C3DCriticalPointEigen::get_real_eval3()const
{
	assert(type != ev_complex);
	return eval3; 
}     
inline fcomplex C3DCriticalPointEigen::get_complex_eval2()const
{
	assert(type == ev_complex);
	return fcomplex(eval2,eval3); 
}
inline fcomplex C3DCriticalPointEigen::get_complex_eval3()const
{
	assert(type == ev_complex);
	return fcomplex(eval2,-eval3); 
}

inline const C3DFVector C3DCriticalPointEigen::get_evect1()const
{
	return evec1; 
}
inline const C3DFVector C3DCriticalPointEigen::get_evect2()const
{
	return evec2; 
}
inline const C3DFVector C3DCriticalPointEigen::get_evect3()const
{
	return evec3; 
}
inline const C3DFVector C3DCriticalPointEigen::get_real_evect2()const
{
	assert(type != ev_complex);	
	return evec2; 
}
inline const C3DFVector C3DCriticalPointEigen::get_real_evect3()const
{
	assert(type != ev_complex);
	return evec3; 
}
inline const C3DCVector C3DCriticalPointEigen::get_complex_evect2()const
{
	assert(type == ev_complex);
	return C3DCVector(fcomplex(evec2.x,evec3.x),
			  fcomplex(evec2.y,evec3.y),
			  fcomplex(evec2.z,evec3.z));
}
inline const C3DCVector C3DCriticalPointEigen::get_complex_evect3()const
{
	assert(type == ev_complex);
	return C3DCVector(fcomplex(evec2.x,-evec3.x),
			  fcomplex(evec2.y,-evec3.y),
			  fcomplex(evec2.z,-evec3.z));
}

inline const C3DFVector C3DCriticalPointEigen::get_location()const
{
	return location; 
}


inline const C3DFMatrix C3DCriticalPointEigen::get_portrait()const
{
	return portrait; 
}


//template implementation


inline const C3DFVector  C3DCriticalPoint::get_point()const
{
	return x0;
}

inline const C3DFMatrix  C3DCriticalPoint::get_a()const
{
	return A;
}

inline C3DFVector& C3DCriticalPoint::get_point()
{
	return x0;
}

inline C3DFMatrix& C3DCriticalPoint::get_a()
{
	return A;
}

inline  float C3DCriticalPoint::get_gamma()const
{
	return gamma;
}

inline void C3DCriticalPoint::set_gamma(float _gamma)
{
	gamma = _gamma;
}


inline bool C3DCriticalPoint::operator ==(const C3DCriticalPoint& cp) const
{
	return  gamma==cp.gamma && A == cp.A && x0 == cp.x0;
}

#ifdef UGLY_HACK
inline bool C3DCriticalPoint::operator < (const C3DCriticalPoint& cp) const
{
	// we shouldn't need this ...
	return x0 < cp.x0; 
}
#endif

#endif

NS_MIA_END
