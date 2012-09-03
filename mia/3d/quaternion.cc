/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>


#include <mia/3d/quaternion.hh>
#include <mia/core/utils.hh>
#include <mia/core/msgstream.hh>
#include <vector>
#include <memory>


NS_MIA_BEGIN 
using std::swap; 
using std::unique_ptr; 

Quaternion::Quaternion():
	m_w(0.0)
{
}

Quaternion::Quaternion(const C3DFMatrix& m)
{
	double data[16] = {
		(m.x.x - m.y.y - m.z.z) / 3.0, 
		(m.y.x + m.x.y) / 3.0, 
		(m.z.x + m.x.z) / 3.0, 
		(m.y.z - m.z.y) / 3.0, 

		(m.y.x + m.x.y) / 3.0, 
		(m.y.y - m.x.x - m.z.z) / 3.0, 
		(m.y.z + m.z.y) / 3.0, 
		(m.z.x - m.x.z) / 3.0, 

		(m.z.x + m.x.z) / 3.0, 
		(m.y.z + m.z.y) / 3.0, 
		(m.z.z - m.x.x - m.y.y) / 3.0, 
		(m.z.y - m.y.z) / 3.0, 

		(m.y.z - m.z.y) / 3.0, 
		(m.z.x - m.x.z) / 3.0, 
		(m.z.y - m.y.z) / 3.0, 
		(m.z.z + m.x.x + m.y.y) / 3.0
	}; 
	
	gsl_matrix_view gslm  = gsl_matrix_view_array (data, 4, 4);
	
	
	auto gsl_vector_delete = [](gsl_vector * p) { gsl_vector_free(p); };
	unique_ptr<gsl_vector, decltype(gsl_vector_delete)> eval(gsl_vector_alloc (4), gsl_vector_delete ); 

	auto gsl_matrix_delete = [](gsl_matrix * p) { gsl_matrix_free(p); };
	unique_ptr<gsl_matrix, decltype(gsl_matrix_delete)> evec(gsl_matrix_alloc (4, 4), gsl_matrix_delete );

	auto gsl_eigen_symmv_delete = [](gsl_eigen_symmv_workspace * p) { gsl_eigen_symmv_free(p); };
	unique_ptr<gsl_eigen_symmv_workspace, decltype(gsl_eigen_symmv_delete)> ws(gsl_eigen_symmv_alloc (4), gsl_eigen_symmv_delete);

	gsl_eigen_symmv (&gslm.matrix, eval.get(), evec.get(), ws.get());
	
	gsl_eigen_symmv_sort (eval.get(), evec.get(), GSL_EIGEN_SORT_ABS_ASC);
	

	m_w = gsl_matrix_get(evec.get(), 0, 3); 
	m_v.x = gsl_matrix_get(evec.get(), 1, 3); 
	m_v.y = gsl_matrix_get(evec.get(), 2, 3); 
	m_v.z = gsl_matrix_get(evec.get(), 3, 3);

}

Quaternion::Quaternion(const C3DDVector& rot):
	m_v(0.0, 0.0, 0.0), 
	m_w(1.0)

{
	double cos_phi, sin_phi, cos_psi, sin_psi, cos_theta, sin_theta; 
	sincos(0.5 * rot.x, &sin_phi,   &cos_phi); 
	sincos(0.5 * rot.y, &sin_psi,   &cos_psi); 
	sincos(0.5 * rot.z, &sin_theta, &cos_theta); 


	m_w   = cos_phi * cos_psi* cos_theta + sin_phi * sin_psi * sin_theta;  
	m_v.x = sin_phi * cos_psi* cos_theta - cos_phi * sin_psi * sin_theta;  
	m_v.y = cos_phi * sin_psi* cos_theta + sin_phi * cos_psi * sin_theta;  
	m_v.z = cos_phi * cos_psi* sin_theta - sin_phi * sin_psi * cos_theta; 

}

Quaternion::Quaternion(double w, double  x, double y, double z):
	m_v(x,y,z), 
	m_w(w)
{
}

double Quaternion::norm() const
{
	return sqrt(m_v.norm2() + m_w * m_w); 
}

void Quaternion::normalize()
{
	double n = norm(); 
	if (n > 0) {
		m_v /= n; 
		m_w /= n; 
	}
}

C3DDVector Quaternion::get_euler_angles() const 
{
	FUNCTION_NOT_TESTED; 
	const double sqw = m_w   * m_w;
	const double sqx = m_v.x * m_v.x;
	const double sqy = m_v.y * m_v.y;
	const double sqz = m_v.z * m_v.z;
	const double norm2 = sqx + sqy + sqz + sqw; 
	const double test = m_v.x * m_v.y + m_v.z * m_w;

	if (test > 0.499 * norm2) 
		return C3DDVector(2 * atan2(m_v.x,m_w), M_PI/2.0, 0.0); 

	if (test < -0.499 * norm2)
		return C3DDVector(-2 * atan2(m_v.x,m_w), -M_PI/2.0, 0.0); 
	
	return C3DDVector(atan2(2.0 * (m_v.y * m_w - m_v.x * m_v.z), sqx - sqy - sqz + sqw), 
			  asin(2*test/norm2), 
			  atan2(2.0 * (m_v.x * m_w - m_v.y * m_v.z),-sqx + sqy - sqz + sqw)); 
}

Quaternion Quaternion::inverse() const
{
	return Quaternion(m_w, -m_v.x, -m_v.y, -m_v.z); 
}


const C3DFMatrix Quaternion::get_rotation_matrix() const
{
	const double a2 = m_w   * m_w;
	const double b2 = m_v.x * m_v.x;
	const double c2 = m_v.y * m_v.y;
	const double d2 = m_v.z * m_v.z;

	const double bc = 2.0 * m_v.x * m_v.y; 
	const double ad = 2.0 * m_w * m_v.z; 
	const double bd = 2.0 * m_v.x * m_v.z; 
	const double ac = 2.0 * m_w * m_v.y; 
	const double cd = 2.0 * m_v.y * m_v.z; 
	const double ab = 2.0 * m_w * m_v.x; 
	
	return C3DFMatrix(C3DFVector(a2 + b2 - c2 - d2, bc - ad, bd + ac), 
			  C3DFVector(bc + ad, a2 - b2 + c2 - d2, cd - ab), 
			  C3DFVector(bd - ac, cd + ab, a2 - b2 - c2 + d2));
	
}

void Quaternion::print(std::ostream& os) const
{
	os << "(" << m_w << "," << m_v << ")"; 
}

bool operator == (const Quaternion& a, const Quaternion& b)
{
	return (a.m_v == b.m_v) && (a.m_w == b.m_w); 
}

bool operator != (const Quaternion& a, const Quaternion& b)
{
	return !(a == b); 
}

Quaternion& Quaternion::operator += (const Quaternion& other)
{
	m_v += other.m_v; 
	m_w += other.m_w; 
	return *this; 
}

Quaternion& Quaternion::operator -= (const Quaternion& other)
{
	m_v -= other.m_v; 
	m_w -= other.m_w; 
	return *this; 
}


Quaternion& Quaternion::operator *= (const Quaternion& o)
{
	const double a = m_w * o.m_w - m_v.x * o.m_v.x - m_v.y * o.m_v.y - m_v.z*o.m_v.z;  
	const double b = m_v.x * o.m_w + m_w * o.m_v.x + m_v.y * o.m_v.z - m_v.z * o.m_v.y; 
	const double c = m_w  * o.m_v.y - m_v.x * o.m_v.z + m_v.y*o.m_w + m_v.z*o.m_v.x; 
	const double d = m_w * o.m_v.z + m_v.x * o.m_v.y - m_v.y * o.m_v.x + m_v.z * o.m_w;
	m_w = a; 
	m_v = C3DFVector(b,c,d); 
	return *this; 
}



NS_MIA_END

