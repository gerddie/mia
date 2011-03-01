/*  -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2011
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


#include <mia/3d/quaternion.hh>
#include <mia/core/utils.hh>
#include <mia/core/msgstream.hh>
#include <vector>


NS_MIA_BEGIN 
using std::swap; 

Quaternion::Quaternion():
	m_w(0.0)
{
}

Quaternion::Quaternion(const Quaternion& other):
	m_v(other.m_v),
	m_w(other.m_w)
{
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
