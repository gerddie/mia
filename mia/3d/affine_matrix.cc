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
	m_matrix[1] = a12; 
	m_matrix[2] = a13; 
	m_matrix[3] = a14; 

	m_matrix[4] = a21; 
	m_matrix[5] = a22; 
	m_matrix[6] = a23; 
	m_matrix[7] = a24; 

	m_matrix[8] = a31; 
	m_matrix[9] = a32; 
	m_matrix[10]= a33; 
	m_matrix[11]= a34; 

	m_matrix[15]=1.0f;

}


void CAffinTransformMatrix::rotate_x(const C3DFVector& center, float angle)
{
        float c, s; 
        sincosf(angle, &s, &c); 

	float ac[8]; 
	float as[8]; 

	for (int i = 0; i < 8; ++i) {
		ac[i] = m_matrix[4+i] * c; 
		as[i] = m_matrix[4+i] * s; 
	}

	m_matrix[4] = ac[0] - as[4]; 
	m_matrix[5] = ac[1] - as[5]; 
	m_matrix[6] = ac[2] - as[6]; 
	m_matrix[7] = ac[3] - as[7] + center.z * s - center.y * c + center.y; 

	m_matrix[8] = as[0] + ac[4]; 
	m_matrix[9] = as[1] + ac[5]; 
	m_matrix[10] = as[2] + ac[6]; 
	m_matrix[11] = as[3] + ac[7] - center.y * s - center.z * c + center.z;
	

}

void CAffinTransformMatrix::rotate_y(const C3DFVector& center, float angle)
{
        float c, s; 
        sincosf(angle, &s, &c); 

	float ac[8]; 
	float as[8]; 

	for (int i = 0; i < 4; ++i) {
		ac[i] = m_matrix[i] * c; 
		as[i] = m_matrix[i] * s; 
	}

	for (int i = 4; i < 8; ++i) {
		ac[i] = m_matrix[4+i] * c; 
		as[i] = m_matrix[4+i] * s; 
	}


	m_matrix[0] = ac[0] - as[4]; 
	m_matrix[1] = ac[1] - as[5]; 
	m_matrix[2] = ac[2] - as[6]; 
	m_matrix[3] = ac[3] - as[7] + center.z * s - center.x * c + center.x; 

	m_matrix[8] = as[0] + ac[4]; 
	m_matrix[9] = as[1] + ac[5]; 
	m_matrix[10] = as[2] + ac[6]; 
	m_matrix[11] = as[3] + ac[7] - center.x * s - center.z * c + center.z;

}

void CAffinTransformMatrix::rotate_z(const C3DFVector& center, float angle)
{
        float c, s; 
        sincosf(angle, &s, &c); 

	float ac[8]; 
	float as[8]; 

	for (int i = 0; i < 8; ++i) {
		ac[i] = m_matrix[i] * c; 
		as[i] = m_matrix[i] * s; 
	}


	m_matrix[0] = ac[0] - as[4]; 
	m_matrix[1] = ac[1] - as[5]; 
	m_matrix[2] = ac[2] - as[6]; 
	m_matrix[3] = ac[3] - as[7] + center.y * s - center.x * c + center.x; 

	m_matrix[4] = as[0] + ac[4]; 
	m_matrix[5] = as[1] + ac[5]; 
	m_matrix[6] = as[2] + ac[6]; 
	m_matrix[7] = as[3] + ac[7] - center.x * s - center.y * c + center.y;
}

void CAffinTransformMatrix::rotate(const C3DFVector& center, const Quaternion& q)
{
	const auto rot = q.get_rotation_matrix(); 
	const auto shift = center - rot * center; 
	vector<float> help(16,0.0f); 
	
	// multiplying from left side with rot
	help[0] = m_matrix[0] * rot.x.x + m_matrix[4] * rot.x.y + m_matrix[8] * rot.x.z;  
	help[1] = m_matrix[1] * rot.x.x + m_matrix[5] * rot.x.y + m_matrix[9] * rot.x.z;  
	help[2] = m_matrix[2] * rot.x.x + m_matrix[6] * rot.x.y + m_matrix[10] * rot.x.z;
	help[3] = m_matrix[3] * rot.x.x + m_matrix[7] * rot.x.y + m_matrix[11] * rot.x.z + shift.x; 

	help[4] = m_matrix[0] * rot.y.x + m_matrix[4] * rot.y.y + m_matrix[8] * rot.y.z;  
	help[5] = m_matrix[1] * rot.y.x + m_matrix[5] * rot.y.y + m_matrix[9] * rot.y.z;  
	help[6] = m_matrix[2] * rot.y.x + m_matrix[6] * rot.y.y + m_matrix[10] * rot.y.z;
	help[7] = m_matrix[3] * rot.y.x + m_matrix[7] * rot.y.y + m_matrix[11] * rot.y.z + shift.y;

	help[8] = m_matrix[0] * rot.z.x + m_matrix[4] * rot.z.y + m_matrix[8] * rot.z.z;  
	help[9] = m_matrix[1] * rot.z.x + m_matrix[5] * rot.z.y + m_matrix[9] * rot.z.z;  
	help[10] = m_matrix[2] * rot.z.x + m_matrix[6] * rot.z.y + m_matrix[10] * rot.z.z;
	help[11] = m_matrix[3] * rot.z.x + m_matrix[7] * rot.z.y + m_matrix[11] * rot.z.z + shift.z;
	help[15] = 1.0f; 

	swap(help, m_matrix); 

}

void CAffinTransformMatrix::scale(const C3DFVector& center, const C3DFVector& scale)
{
	const C3DFVector sh = (C3DFVector::_1 - scale) * center; 

	for (int i = 0; i < 4; ++i) {
		m_matrix[i] *= scale.x; 
		m_matrix[i + 4] *= scale.y; 
		m_matrix[i + 8] *= scale.z; 
	}
	m_matrix[3]  += sh.x; 
	m_matrix[7]  += sh.y; 
	m_matrix[11] += sh.z; 
	
}

const std::vector<float>& CAffinTransformMatrix::data() const
{
	return m_matrix; 
}

void CAffinTransformMatrix::translate(const C3DFVector& shift)
{
	m_matrix[3] += shift.x; 
	m_matrix[7] += shift.y; 
	m_matrix[11] += shift.z; 
}

void CAffinTransformMatrix::identity()
{
        m_matrix[0] = m_matrix[5] = m_matrix[10] = m_matrix[15] = 1.0f; 
}

CAffinTransformMatrix& CAffinTransformMatrix::operator *= (const CAffinTransformMatrix& other)
{
	vector<float> help(16,0.0f); 

	for(int i = 0; i < 4; ++i) {
		for(int k = 0; k < 4; ++k) {
			for(int l = 0; l < 4; ++l) {
				help[4*i + k] += m_matrix[4*i + l] * other.m_matrix[4*l + k]; 
			}
		}
	}
	swap(m_matrix, help); 
	return *this; 
}

const CAffinTransformMatrix CAffinTransformMatrix::inverse() const
{
	const float det = (m_matrix[0]*m_matrix[5]-m_matrix[1]*m_matrix[4])*m_matrix[10]+
		(m_matrix[2]*m_matrix[4]-m_matrix[0]*m_matrix[6])*m_matrix[9]+
		(m_matrix[1]*m_matrix[6]-m_matrix[2]*m_matrix[5])*m_matrix[8]; 

	if (::fabs(det) < 1e-8)
		throw runtime_error("CAffinTransformMatrix:inverse(): Matrix is (numerically) singular"); 
	
	const float inv_det = 1.0f/det; 
	
	CAffinTransformMatrix result; 

	result.m_matrix[0] = inv_det * (m_matrix[5]*m_matrix[10]- m_matrix[6]*m_matrix[9]); 
	result.m_matrix[1] = inv_det * (m_matrix[2]*m_matrix[9] - m_matrix[1]*m_matrix[10]); 
	result.m_matrix[2] = inv_det * (m_matrix[1]*m_matrix[6] - m_matrix[2]*m_matrix[5]); 
	result.m_matrix[3] = inv_det * ((m_matrix[2]*m_matrix[5] - m_matrix[1]*m_matrix[6])*m_matrix[11]+
					(m_matrix[1]*m_matrix[7] - m_matrix[3]*m_matrix[5])*m_matrix[10]+
					(m_matrix[3]*m_matrix[6] - m_matrix[2]*m_matrix[7])*m_matrix[9]); 
		
	result.m_matrix[4] = inv_det *  (m_matrix[6]*m_matrix[8] - m_matrix[4]*m_matrix[10]); 
	result.m_matrix[5] = inv_det *  (m_matrix[0]*m_matrix[10]- m_matrix[2]*m_matrix[8]); 
	result.m_matrix[6] = inv_det *  (m_matrix[2]*m_matrix[4] - m_matrix[0]*m_matrix[6]);
	result.m_matrix[7] = inv_det *  ((m_matrix[0]*m_matrix[6]-m_matrix[2]*m_matrix[4])*m_matrix[11]+
					 (m_matrix[3]*m_matrix[4]-m_matrix[0]*m_matrix[7])*m_matrix[10]+
					 (m_matrix[2]*m_matrix[7]-m_matrix[3]*m_matrix[6])*m_matrix[8]); 

	result.m_matrix[8] = inv_det *  (m_matrix[4]*m_matrix[9] - m_matrix[5]*m_matrix[8]); 
	result.m_matrix[9] = inv_det *  (m_matrix[1]*m_matrix[8] - m_matrix[0]*m_matrix[9]); 
	result.m_matrix[10]= inv_det *  (m_matrix[0]*m_matrix[5] - m_matrix[1]*m_matrix[4]); 
	result.m_matrix[11]= inv_det *  ((m_matrix[1]*m_matrix[4] - m_matrix[0]*m_matrix[5])*m_matrix[11]+
					 (m_matrix[0]*m_matrix[7] - m_matrix[3]*m_matrix[4])*m_matrix[9]+
					 (m_matrix[3]*m_matrix[5] - m_matrix[1]*m_matrix[7])*m_matrix[8]); 
	
	result.m_matrix[15] = 1.0f; 
	
	return result; 
}


C3DFVector CAffinTransformMatrix::operator * (const C3DFVector& x) const
{
	return C3DFVector(
		m_matrix[0] * x.x + m_matrix[1] * x.y + m_matrix[2] * x.z + m_matrix[3], 
		m_matrix[4] * x.x + m_matrix[5] * x.y + m_matrix[6] * x.z + m_matrix[7], 
		m_matrix[8] * x.x + m_matrix[9] * x.y + m_matrix[10] * x.z + m_matrix[11]); 
}

CAffinTransformMatrix operator * (const CAffinTransformMatrix& lhs, const CAffinTransformMatrix& rhs)
{
	CAffinTransformMatrix h(lhs); 
	h*=rhs; 
	return h; 
}

NS_MIA_END
