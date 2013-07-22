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

        const float ac_1 = m_matrix[1] * c; 
        const float ac_2 = m_matrix[2] * c; 
        const float ac_5 = m_matrix[5] * c; 
        const float ac_6 = m_matrix[6] * c; 
        const float ac_9 = m_matrix[9] * c; 
        const float ac_A = m_matrix[10]* c; 

        const float as_1 = m_matrix[1] * s; 
        const float as_2 = m_matrix[2] * s; 
        const float as_5 = m_matrix[5] * s; 
        const float as_6 = m_matrix[6] * s; 
        const float as_9 = m_matrix[9] * s; 
        const float as_A = m_matrix[10]* s; 

	const float cys = center.y * s; 
	const float czs = center.z * s; 
	
	const float cyc = center.y * c; 
	const float czc = center.z * c; 
	
	const float c1 = -czs + cyc - center.y; 
	const float c2 = cys + czc - center.z; 

	m_matrix[3] += m_matrix[1] * c1 + m_matrix[2] * c2; 
	m_matrix[7] += m_matrix[5] * c1 + m_matrix[6] * c2; 
	m_matrix[11] += m_matrix[9] * c1 + m_matrix[10] * c2;

	m_matrix[1] = ac_1 + as_2; 
	m_matrix[2] = ac_2 - as_1; 

	m_matrix[5] = ac_5 + as_6; 
	m_matrix[6] = ac_6 - as_5; 

	m_matrix[9] = ac_9 + as_A; 
	m_matrix[10]= ac_A - as_9; 
	
}

void CAffinTransformMatrix::rotate_y(const C3DFVector& center, float angle)
{
}

void CAffinTransformMatrix::rotate_z(const C3DFVector& center, float angle)
{
}

void CAffinTransformMatrix::rotate(const C3DFVector& center, const Quaternion& q)
{
}

void CAffinTransformMatrix::scale(const C3DFVector& center, const C3DFVector& scale)
{
	const C3DFVector sh = center * (scale - C3DFVector::_1); 

	m_matrix[3] += m_matrix[2]* sh.z + m_matrix[1]* sh.y + m_matrix[0] *sh.x;
	m_matrix[7] += m_matrix[6]* sh.z + m_matrix[5]* sh.y + m_matrix[4] *sh.x;
	m_matrix[11] += m_matrix[10]* sh.z + m_matrix[9]* sh.y + m_matrix[8] *sh.x;

	for (int i = 0; i < 3; ++i) {
		m_matrix[4 * i] *= scale.x; 
		m_matrix[4 * i + 1] *= scale.y; 
		m_matrix[4 * i + 2] *= scale.z; 
	}
}

const std::vector<float>& CAffinTransformMatrix::data() const
{
	return m_matrix; 
}

void CAffinTransformMatrix::translate(const C3DFVector& shift)
{
	const C3DFVector c = *this * shift; 
	m_matrix[3] = c.x; 
	m_matrix[7] = c.y; 
	m_matrix[11] = c.z; 
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
