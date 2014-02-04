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
#include <mia/3d/affine_matrix.hh>

NS_MIA_BEGIN

using std::vector; 
using std::runtime_error; 

CAffinTransformMatrix::CAffinTransformMatrix():m_matrix(16, 0.0f)
{
        identity();
}

CAffinTransformMatrix::CAffinTransformMatrix(float a11, float a12, float a13, float a14, 
					     float a21, float a22, float a23, float a24, 
					     float a31, float a32, float a33, float a34):m_matrix(16, 0.0f)
{
	m_matrix[0] = a11; 
	m_matrix[4] = a12; 
	m_matrix[8] = a13; 
	m_matrix[12]= a14; 

	m_matrix[1] = a21; 
	m_matrix[5] = a22; 
	m_matrix[9] = a23; 
	m_matrix[13]= a24; 

	m_matrix[2] = a31; 
	m_matrix[6] = a32; 
	m_matrix[10]= a33; 
	m_matrix[14]= a34; 

	m_matrix[15]=1.0f;

}

void CAffinTransformMatrix::rotate_x(float angle, const C3DFVector& center)
{
        float c, s; 
        sincosf(angle, &s, &c); 

	float ac[8]; 
	float as[8]; 

	const int a1 = 0; 
	const int a2 = 1;
	const int a5 = 2; 
	const int a6 = 3;
	const int a9 = 4; 
	const int aA = 5;
	const int aD = 6; 
	const int aE = 7;

	ac[a1] = m_matrix[1] * c; as[a1] = m_matrix[1] * s; 
	ac[a2] = m_matrix[2] * c; as[a2] = m_matrix[2] * s; 
	ac[a5] = m_matrix[5] * c; as[a5] = m_matrix[5] * s; 
	ac[a6] = m_matrix[6] * c; as[a6] = m_matrix[6] * s; 
	ac[a9] = m_matrix[9] * c; as[a9] = m_matrix[9] * s; 
	ac[aA] = m_matrix[10]* c; as[aA] = m_matrix[10] * s; 
	ac[aD] = m_matrix[13]* c; as[aD] = m_matrix[13] * s; 
	ac[aE] = m_matrix[14]* c; as[aE] = m_matrix[14] * s; 

	m_matrix[1] = ac[a1] - as[a2]; 
	m_matrix[5] = ac[a5] - as[a6]; 
	m_matrix[9] = ac[a9] - as[aA]; 
	m_matrix[13] = ac[aD] - as[aE] + center.z * s - center.y * c + center.y; 

	m_matrix[2] = as[a1] + ac[a2]; 
	m_matrix[6] = as[a5] + ac[a6]; 
	m_matrix[10]= as[a9] + ac[aA]; 
	m_matrix[14]= as[aD] + ac[aE] - center.y * s - center.z * c + center.z;
	

}

void CAffinTransformMatrix::rotate_y(float angle, const C3DFVector& center)
{
        float c, s; 
        sincosf(angle, &s, &c); 

	float ac[8]; 
	float as[8]; 

	const int a0 = 0; 
	const int a2 = 1;
	const int a4 = 2; 
	const int a6 = 3;
	const int a8 = 4; 
	const int aA = 5;
	const int aC = 6; 
	const int aE = 7;

	ac[a0] = m_matrix[0] * c; as[a0] = m_matrix[0] * s; 
	ac[a2] = m_matrix[2] * c; as[a2] = m_matrix[2] * s; 
	ac[a4] = m_matrix[4] * c; as[a4] = m_matrix[4] * s; 
	ac[a6] = m_matrix[6] * c; as[a6] = m_matrix[6] * s; 
	ac[a8] = m_matrix[8] * c; as[a8] = m_matrix[8] * s; 
	ac[aA] = m_matrix[10]* c; as[aA] = m_matrix[10] * s; 
	ac[aC] = m_matrix[12]* c; as[aC] = m_matrix[12] * s; 
	ac[aE] = m_matrix[14]* c; as[aE] = m_matrix[14] * s; 

	m_matrix[0] = ac[a0] - as[a2]; 
	m_matrix[4] = ac[a4] - as[a6]; 
	m_matrix[8] = ac[a8] - as[aA]; 
	m_matrix[12] = ac[aC] - as[aE] + center.z * s - center.x * c + center.x; 

	m_matrix[2] = as[a0] + ac[a2]; 
	m_matrix[6] = as[a4] + ac[a6]; 
	m_matrix[10]= as[a8] + ac[aA]; 
	m_matrix[14]= as[aC] + ac[aE] - center.x * s - center.z * c + center.z;

}

void CAffinTransformMatrix::rotate_z(float angle, const C3DFVector& center)
{
        float c, s; 
        sincosf(angle, &s, &c); 

	float ac[8]; 
	float as[8]; 

	const int a0 = 0; 
	const int a1 = 1;
	const int a4 = 2; 
	const int a5 = 3;
	const int a8 = 4; 
	const int a9 = 5;
	const int aC = 6; 
	const int aD = 7;

	ac[a0] = m_matrix[0] * c; as[a0] = m_matrix[0] * s; 
	ac[a1] = m_matrix[1] * c; as[a1] = m_matrix[1] * s; 
	ac[a4] = m_matrix[4] * c; as[a4] = m_matrix[4] * s; 
	ac[a5] = m_matrix[5] * c; as[a5] = m_matrix[5] * s; 
	ac[a8] = m_matrix[8] * c; as[a8] = m_matrix[8] * s; 
	ac[a9] = m_matrix[9] * c; as[a9] = m_matrix[9] * s; 
	ac[aC] = m_matrix[12]* c; as[aC] = m_matrix[12] * s; 
	ac[aD] = m_matrix[13]* c; as[aD] = m_matrix[13] * s; 

	m_matrix[0] = ac[a0] - as[a1]; 
	m_matrix[4] = ac[a4] - as[a5]; 
	m_matrix[8] = ac[a8] - as[a9]; 
	m_matrix[12] = ac[aC] - as[aD] + center.y * s - center.x * c + center.x; 

	m_matrix[1] = as[a0] + ac[a1]; 
	m_matrix[5] = as[a4] + ac[a5]; 
	m_matrix[9]= as[a8] + ac[a9]; 
	m_matrix[13]= as[aC] + ac[aD] - center.x * s - center.y * c + center.y;
}


void CAffinTransformMatrix::transform_centered(const C3DFMatrix& m, const C3DFVector& center)
{
	const auto shift = center - m * center; 
	vector<float> help(16,0.0f); 
	
	// multiplying from right side with m transposed 
	help[0] = m_matrix[0] * m.x.x + m_matrix[1] * m.x.y + m_matrix[2] * m.x.z;  
	help[1] = m_matrix[0] * m.y.x + m_matrix[1] * m.y.y + m_matrix[2] * m.y.z;  
	help[2] = m_matrix[0] * m.z.x + m_matrix[1] * m.z.y + m_matrix[2] * m.z.z;  

	help[4] = m_matrix[4] * m.x.x + m_matrix[5] * m.x.y + m_matrix[6] * m.x.z;  
	help[5] = m_matrix[4] * m.y.x + m_matrix[5] * m.y.y + m_matrix[6] * m.y.z;  
	help[6] = m_matrix[4] * m.z.x + m_matrix[5] * m.z.y + m_matrix[6] * m.z.z;  

	help[8] = m_matrix[8] * m.x.x + m_matrix[9] * m.x.y + m_matrix[10] * m.x.z;
	help[9] = m_matrix[8] * m.y.x + m_matrix[9] * m.y.y + m_matrix[10] * m.y.z;
	help[10]= m_matrix[8] * m.z.x + m_matrix[9] * m.z.y + m_matrix[10] * m.z.z;

	help[12] = m_matrix[12] * m.x.x + m_matrix[13] * m.x.y + m_matrix[14] * m.x.z + shift.x; 
	help[13] = m_matrix[12] * m.y.x + m_matrix[13] * m.y.y + m_matrix[14] * m.y.z + shift.y;
	help[14] = m_matrix[12] * m.z.x + m_matrix[13] * m.z.y + m_matrix[14] * m.z.z + shift.z;


	help[15] = 1.0f; 

	swap(help, m_matrix); 

}

void CAffinTransformMatrix::rotate(const Quaternion& q, const C3DFVector& center)
{
	const auto rot = q.get_rotation_matrix(); 
	const auto shift = center - rot * center; 
	vector<float> help(16,0.0f); 
	
	// multiplying from right side with rot transposed 
	help[0] = m_matrix[0] * rot.x.x + m_matrix[1] * rot.x.y + m_matrix[2] * rot.x.z;  
	help[1] = m_matrix[0] * rot.y.x + m_matrix[1] * rot.y.y + m_matrix[2] * rot.y.z;  
	help[2] = m_matrix[0] * rot.z.x + m_matrix[1] * rot.z.y + m_matrix[2] * rot.z.z;  

	help[4] = m_matrix[4] * rot.x.x + m_matrix[5] * rot.x.y + m_matrix[6] * rot.x.z;  
	help[5] = m_matrix[4] * rot.y.x + m_matrix[5] * rot.y.y + m_matrix[6] * rot.y.z;  
	help[6] = m_matrix[4] * rot.z.x + m_matrix[5] * rot.z.y + m_matrix[6] * rot.z.z;  

	help[8] = m_matrix[8] * rot.x.x + m_matrix[9] * rot.x.y + m_matrix[10] * rot.x.z;
	help[9] = m_matrix[8] * rot.y.x + m_matrix[9] * rot.y.y + m_matrix[10] * rot.y.z;
	help[10]= m_matrix[8] * rot.z.x + m_matrix[9] * rot.z.y + m_matrix[10] * rot.z.z;

	help[12] = m_matrix[12] * rot.x.x + m_matrix[13] * rot.x.y + m_matrix[14] * rot.x.z + shift.x; 
	help[13] = m_matrix[12] * rot.y.x + m_matrix[13] * rot.y.y + m_matrix[14] * rot.y.z + shift.y;
	help[14] = m_matrix[12] * rot.z.x + m_matrix[13] * rot.z.y + m_matrix[14] * rot.z.z + shift.z;


	help[15] = 1.0f; 

	swap(help, m_matrix); 

}

void CAffinTransformMatrix::scale(const C3DFVector& scale, const C3DFVector& center)
{
	const C3DFVector sh = (C3DFVector::_1 - scale) * center; 

	for (int i = 0; i < 4; ++i) {
		m_matrix[4*i] *= scale.x; 
		m_matrix[4*i + 1] *= scale.y; 
		m_matrix[4*i + 2] *= scale.z; 
	}
	m_matrix[12]  += sh.x; 
	m_matrix[13]  += sh.y; 
	m_matrix[14] += sh.z; 
	
}

const std::vector<float>& CAffinTransformMatrix::data() const
{
	return m_matrix; 
}

void CAffinTransformMatrix::translate(const C3DFVector& shift)
{
	m_matrix[12] += shift.x; 
	m_matrix[13] += shift.y; 
	m_matrix[14] += shift.z; 
}

void CAffinTransformMatrix::identity()
{
	fill(m_matrix.begin(), m_matrix.end(), 0.0f); 
        m_matrix[0] = m_matrix[5] = m_matrix[10] = m_matrix[15] = 1.0f; 
}

CAffinTransformMatrix& CAffinTransformMatrix::operator *= (const CAffinTransformMatrix& other)
{
	vector<float> help(16,0.0f); 

	help[0] = m_matrix[0] * other.m_matrix[0] + m_matrix[4] * other.m_matrix[1] + m_matrix[8] * other.m_matrix[2];  
	help[1] = m_matrix[1] * other.m_matrix[0] + m_matrix[5] * other.m_matrix[1] + m_matrix[9] * other.m_matrix[2];  
	help[2] = m_matrix[2] * other.m_matrix[0] + m_matrix[6] * other.m_matrix[1] + m_matrix[10]* other.m_matrix[2];

	help[4] = m_matrix[0] * other.m_matrix[4] + m_matrix[4] * other.m_matrix[5] + m_matrix[8] * other.m_matrix[6];  
	help[5] = m_matrix[1] * other.m_matrix[4] + m_matrix[5] * other.m_matrix[5] + m_matrix[9] * other.m_matrix[6];  
	help[6] = m_matrix[2] * other.m_matrix[4] + m_matrix[6] * other.m_matrix[5] + m_matrix[10] * other.m_matrix[6];

	help[8] = m_matrix[0] * other.m_matrix[8] + m_matrix[4] * other.m_matrix[9] + m_matrix[8] * other.m_matrix[10];  
	help[9] = m_matrix[1] * other.m_matrix[8] + m_matrix[5] * other.m_matrix[9] + m_matrix[9] * other.m_matrix[10];  
	help[10]= m_matrix[2] * other.m_matrix[8] + m_matrix[6] * other.m_matrix[9] + m_matrix[10]* other.m_matrix[10];

	help[12]= other.m_matrix[12] * m_matrix[0] + other.m_matrix[13] * m_matrix[4] + other.m_matrix[14] * m_matrix[8] + m_matrix[12]; 
	help[13]= other.m_matrix[12] * m_matrix[1] + other.m_matrix[13] * m_matrix[5] + other.m_matrix[14] * m_matrix[9] + m_matrix[13];
	help[14]= other.m_matrix[12] * m_matrix[2] + other.m_matrix[13] * m_matrix[6] + other.m_matrix[14] * m_matrix[10]+ m_matrix[14];

	help[15] = 1.0f; 

	swap(m_matrix, help); 
	return *this; 
}

C3DFVector CAffinTransformMatrix::rotate(const C3DFVector& x) const
{
	return C3DFVector(m_matrix[0] * x.x + m_matrix[4] * x.y + m_matrix[8] * x.z, 
			  m_matrix[1] * x.x + m_matrix[5] * x.y + m_matrix[9] * x.z, 
			  m_matrix[2] * x.x + m_matrix[6] * x.y + m_matrix[10] * x.z); 
}

const CAffinTransformMatrix CAffinTransformMatrix::inverse() const
{
	const float det = (m_matrix[0]*m_matrix[5]-m_matrix[4]*m_matrix[1])*m_matrix[10]+
		(m_matrix[8]*m_matrix[1]-m_matrix[0]*m_matrix[9])*m_matrix[6]+
		(m_matrix[4]*m_matrix[9]-m_matrix[8]*m_matrix[5])*m_matrix[2]; 

	if (::fabs(det) < 1e-8)
		throw runtime_error("CAffinTransformMatrix:inverse(): Matrix is (numerically) singular"); 
	
	const float inv_det = 1.0f/det; 
	
	CAffinTransformMatrix result; 

	result.m_matrix[0] = inv_det * (m_matrix[5]*m_matrix[10]- m_matrix[9]*m_matrix[6]); 
	result.m_matrix[4] = inv_det * (m_matrix[8]*m_matrix[6] - m_matrix[4]*m_matrix[10]); 
	result.m_matrix[8] = inv_det * (m_matrix[4]*m_matrix[9] - m_matrix[8]*m_matrix[5]); 
	result.m_matrix[12] = inv_det * ((m_matrix[8]*m_matrix[5] - m_matrix[4]*m_matrix[9])*m_matrix[14]+
					(m_matrix[4]*m_matrix[13] - m_matrix[12]*m_matrix[5])*m_matrix[10]+
					(m_matrix[12]*m_matrix[9] - m_matrix[8]*m_matrix[13])*m_matrix[6]); 
		
	result.m_matrix[1] = inv_det *  (m_matrix[9]*m_matrix[2] - m_matrix[1]*m_matrix[10]); 
	result.m_matrix[5] = inv_det *  (m_matrix[0]*m_matrix[10]- m_matrix[8]*m_matrix[2]); 
	result.m_matrix[9] = inv_det *  (m_matrix[8]*m_matrix[1] - m_matrix[0]*m_matrix[9]);
	result.m_matrix[13] = inv_det *  ((m_matrix[0]*m_matrix[9]-m_matrix[8]*m_matrix[1])*m_matrix[14]+
					 (m_matrix[12]*m_matrix[1]-m_matrix[0]*m_matrix[13])*m_matrix[10]+
					 (m_matrix[8]*m_matrix[13]-m_matrix[12]*m_matrix[9])*m_matrix[2]); 

	result.m_matrix[2] = inv_det *  (m_matrix[1]*m_matrix[6] - m_matrix[5]*m_matrix[2]); 
	result.m_matrix[6] = inv_det *  (m_matrix[4]*m_matrix[2] - m_matrix[0]*m_matrix[6]); 
	result.m_matrix[10]= inv_det *  (m_matrix[0]*m_matrix[5] - m_matrix[4]*m_matrix[1]); 
	result.m_matrix[14]= inv_det *  ((m_matrix[4]*m_matrix[1] - m_matrix[0]*m_matrix[5])*m_matrix[14]+
					 (m_matrix[0]*m_matrix[13] - m_matrix[12]*m_matrix[1])*m_matrix[6]+
					 (m_matrix[12]*m_matrix[5] - m_matrix[4]*m_matrix[13])*m_matrix[2]); 
	
	result.m_matrix[15] = 1.0f; 
	
	return result; 
}

void CAffinTransformMatrix::shear(const C3DFVector& shear, const C3DFVector& origin)
{
	vector<float> help(m_matrix); 
	
	help[ 0] += m_matrix[ 4] * shear.x; 
	help[ 4] += m_matrix[ 5] * shear.x; 
	help[ 8] += m_matrix[ 6] * shear.x; 

	help[ 1] += m_matrix[ 8] * shear.y; 
	help[ 5] += m_matrix[ 9] * shear.y; 
	help[ 9] += m_matrix[10] * shear.y; 

	help[ 2] += m_matrix[ 0] * shear.z; 
	help[ 6] += m_matrix[ 1] * shear.z; 
	help[10] += m_matrix[ 2] * shear.z; 

	if (origin != C3DFVector::_0) {
		help[12] += -(m_matrix[6] * shear.x + m_matrix[2]) * origin.z 
			-(m_matrix[5] * shear.x + m_matrix[1]) * origin.y 
			-(m_matrix[4] * shear.x + m_matrix[0]) * origin.x + m_matrix[13] * shear.x + origin.x; 
		
		help[13] += -(m_matrix[10] * shear.y + m_matrix[6]) * origin.z
			- (m_matrix[9] * shear.y + m_matrix[5]) * origin.y
			- (m_matrix[8] * shear.y + m_matrix[4]) * origin.x + m_matrix[14] * shear.y + origin.y; 
		
		help[14] += - (m_matrix[2] * shear.z + m_matrix[10]) * origin.z
			- (m_matrix[1] * shear.z + m_matrix[9]) * origin.y
			- (m_matrix[0] * shear.z + m_matrix[8]) *origin.x + m_matrix[12] * shear.z + origin.z;
	}else{
		help[12] += m_matrix[13] * shear.x; 
		
		help[13] += m_matrix[14] * shear.y; 
		
		help[14] += m_matrix[12] * shear.z;
	}
	swap(m_matrix, help); 
}



C3DFVector CAffinTransformMatrix::operator * (const C3DFVector& x) const
{
	return C3DFVector(
		m_matrix[0] * x.x + m_matrix[4] * x.y + m_matrix[8] * x.z + m_matrix[12], 
		m_matrix[1] * x.x + m_matrix[5] * x.y + m_matrix[9] * x.z + m_matrix[13], 
		m_matrix[2] * x.x + m_matrix[6] * x.y + m_matrix[10] * x.z + m_matrix[14]); 
}


CAffinTransformMatrix operator * (const CAffinTransformMatrix& lhs, const CAffinTransformMatrix& rhs)
{
	CAffinTransformMatrix h(lhs); 
	h*=rhs; 
	return h; 
}

NS_MIA_END
