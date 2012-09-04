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


#include <mia/3d/orientation.hh>

NS_MIA_BEGIN

C3DOrientationAndPosition::C3DOrientationAndPosition():
	m_axisorder(ior_default), 
	m_origin(0,0,0), 
	m_scale(1,1,1), 
	m_rotation(1,0,0,0)
{
}

C3DOrientationAndPosition::C3DOrientationAndPosition(E3DImageAxisOrientation axis, 
						     const C3DFVector& origin, 
						     const C3DFVector& scale, 
						     const Quaternion& rot):
	m_axisorder(axis), 
	m_origin(origin), 
	m_scale(scale), 
	m_rotation(rotation)
{
}

void C3DOrientationAndPosition::get_transform_parameters(CDoubleVector& params) const
{
	assert(params.size() == 12); 

	const C3DMatrix& axis_switch = get_axis_switch_matrix(); 
	const C3DFMatrix rot = m_rotation.get_rotation_matrix(); 
	const C3DFMatrix scale(C3DFVector(m_scale.x, 0, 0), C3DFVector(0, m_scale.y, 0), C3DFVector(0,0, m_scalele.z)); 

	C3DFMatrix rs = axis_switch * (rot * scale);
	C3DFVector t = axis_switch * m_origin;
	
	params[0] = rs.x.x; 
	params[1] = rs.x.y; 
	params[2] = rs.x.z; 
	params[3] = m_origin.x; 

	params[4] = rs.y.x; 
	params[5] = rs.y.y; 
	params[6] = rs.y.z; 
	params[7] = m_origin.y; 

	params[8]  = rs.z.x; 
	params[9]  = rs.z.y; 
	params[10] = rs.z.z; 
	params[11] = m_origin.z; 

	params[12] = 0; 
	params[13] = 0; 
	params[14] = 0; 
	params[15] = 1; 


}

void get_inverse_transform_parameters(CDoubleVector& params) const
{
	assert(params.size() == 12);
	const C3DMatrix& axis_switch = get_axis_switch_matrix(); 
	const C3DFMatrix rot = m_rotation.get_rotation_matrix().transposed(); 
	const C3DFMatrix scale(C3DFVector(1.0/m_scale.x, 0, 0), 
			       C3DFVector(0, 1.0/m_scale.y, 0), 
			       C3DFVector(0,0, 1.0/m_scalele.z)); 

	const C3DFMatrix sr = scale * rot;
	const C3DFVector mt = C3DFVector::_0 - m_origin; 
	const C3DFVector it = sr * mt; 

	const C3DFMatrix stf = sr * axis_switch;
	const C3DFVector ft = it * axis_switch;
	
	
	params[0] = stf.x.x; 
	params[1] = stf.x.y; 
	params[2] = stf.x.z; 
	params[3] = ft.x; 

	params[4] = stf.y.x; 
	params[5] = stf.y.y; 
	params[6] = stf.y.z; 
	params[7] = ft.y; 

	params[8]  = stf.z.x; 
	params[9]  = stf.z.y; 
	params[10] = stf.z.z; 
	params[11] = ft.z;

	params[12] = 0; 
	params[13] = 0; 
	params[14] = 0; 
	params[15] = 1; 

}

const C3DFMatrix& C3DOriantationAndPosition::get_axis_switch_matrix() const
{
	switch (m_axisorder) {
	case ior_xyz: return ms_order_XYZ; 
	case ior_xzy: return ms_order_XZY; 
	case ior_yxz: return ms_order_YXZ; 
	case ior_yzx: return ms_order_YZX; 
	case ior_zxy: return ms_order_ZXY; 
	case ior_zyx: return ms_order_ZYX; 
	default: 
		cvwarn() << "C3DOrientationAndPosition: axis ordering not specified, assuming default XYZ\n"; 
		return ms_order_XYZ; 
	}
}

C3DOriantationAndPosition& operator +=(const C3DOriantationAndPosition& other); 

bool operator == (const C3DOriantationAndPosition& other) const; 

private:
	
	E3DImageAxisOrientation m_axisorder; 
	C3DFVector m_origin; 
	C3DFVector m_scale; 
	Quaternion m_rotation; 
	bool m_flipped;


EXPORT_3D  std::ostream& operator << (std::ostream& os, E3DImageAxisOrientation orient)
{
	switch (orient) {
	case ior_axial:
		os << "axial";
		break;
	case ior_coronal:
		os << "coronal";
		break;
	case ior_saggital:
		os << "saggital";
		break;
	case ior_unknown:
		os << "unknown";
		break;
	default:
		os << "undefined";
	}
	return os;
}

EXPORT_3D std::istream& operator >> (std::istream& is, E3DImageAxisOrientation& orient)
{
	std::string temp;
	is >> temp;

	if (temp == "axial")
		orient = ior_axial;
	else if (temp == "coronal")
		orient = ior_coronal;
	else if (temp == "saggital")
		orient = ior_saggital;
	else
		orient = ior_unknown;
	return is;
}

const C3DFMatrix C3DOrientationAndPosition::ms_order_XYZ(C3DFVector(1,0,0), C3DFVector(0,1,0), C3DFVector(0,0,1)); 
const C3DFMatrix C3DOrientationAndPosition::ms_order_YXZ(C3DFVector(0,1,0), C3DFVector(1,0,0), C3DFVector(0,0,1));
const C3DFMatrix C3DOrientationAndPosition::ms_order_XZY(C3DFVector(1,0,0), C3DFVector(0,0,1), C3DFVector(0,1,0));
const C3DFMatrix C3DOrientationAndPosition::ms_order_ZYX(C3DFVector(0,0,1), C3DFVector(0,1,0), C3DFVector(1,0,0));
const C3DFMatrix C3DOrientationAndPosition::ms_order_ZXY(C3DFVector(0,0,1), C3DFVector(1,0,0), C3DFVector(0,1,0));
const C3DFMatrix C3DOrientationAndPosition::ms_order_YZX(C3DFVector(0,1,0), C3DFVector(0,0,1), C3DFVector(1,0,0));



NS_MIA_END
