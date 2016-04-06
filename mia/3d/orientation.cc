/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/3d/orientation.hh>
#include <mia/core/utils.hh>
#include <mia/core/attributes.cxx>

NS_MIA_BEGIN
using namespace std; 


C3DOrientationAndPosition::C3DOrientationAndPosition():
	m_axisorder(ior_default), 
	m_origin(0,0,0), 
	m_scale(1,1,1), 
	m_rotation(1,0,0,0)
{
}

C3DOrientationAndPosition::C3DOrientationAndPosition(E3DImageOrientation axis):
	m_axisorder(axis), 
	m_origin(0,0,0), 
	m_scale(1,1,1), 
	m_rotation(1,0,0,0)
{
}

C3DOrientationAndPosition::C3DOrientationAndPosition(E3DImageOrientation axis, 
						     const C3DFVector& origin, 
						     const C3DFVector& scale, 
						     const Quaternion& rot):
	m_axisorder(axis), 
	m_origin(origin), 
	m_scale(scale), 
	m_rotation(rot)
{
}

void C3DOrientationAndPosition::get_transform_parameters(CDoubleVector& params) const
{
	assert(params.size() == 16); 

	const C3DFMatrix& axis_switch = get_axis_switch_matrix(); 
	const C3DFMatrix rot = m_rotation.get_rotation_matrix(); 
	const C3DFMatrix scale(C3DFVector(m_scale.x, 0, 0), C3DFVector(0, m_scale.y, 0), C3DFVector(0,0, m_scale.z)); 

	C3DFMatrix rs = axis_switch * (rot * scale);
	
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

void C3DOrientationAndPosition::get_inverse_transform_parameters(CDoubleVector& params) const
{
	assert(params.size() == 16);
	const C3DFMatrix& axis_switch = get_axis_switch_matrix(); 
	const C3DFMatrix rot = m_rotation.get_rotation_matrix().transposed(); 
	const C3DFMatrix scale(C3DFVector(1.0/m_scale.x, 0, 0), 
			       C3DFVector(0, 1.0/m_scale.y, 0), 
			       C3DFVector(0,0, 1.0/m_scale.z)); 

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

const C3DFMatrix& C3DOrientationAndPosition::get_axis_switch_matrix() const
{
	switch (m_axisorder) {
	case ior_xyz:         return ms_order_XYZ; 
	case ior_xyz_flipped: return ms_order_XYZ_F; 
	case ior_xzy:         return ms_order_XZY; 
	case ior_xzy_flipped: return ms_order_XZY_F; 
	case ior_yxz:         return ms_order_YXZ; 
	case ior_yxz_flipped: return ms_order_YXZ_F; 
	case ior_yzx:         return ms_order_YZX; 
	case ior_yzx_flipped: return ms_order_YZX_F; 
	case ior_zxy:         return ms_order_ZXY; 
	case ior_zxy_flipped: return ms_order_ZXY_F; 
	case ior_zyx:         return ms_order_ZYX; 
	case ior_zyx_flipped: return ms_order_ZYX_F; 
	default: 
		cvwarn() << "C3DOrientationAndPosition: axis ordering not specified, assuming default XYZ\n"; 
		return ms_order_XYZ; 
	}
}

C3DOrientationAndPosition& C3DOrientationAndPosition::operator +=(const C3DOrientationAndPosition& MIA_PARAM_UNUSED(other))
{
	assert(0 && "to be implemented"); 
}

bool C3DOrientationAndPosition::operator == (const C3DOrientationAndPosition& other) const
{
	return 	m_axisorder == other.m_axisorder && 
		m_origin == other.m_origin && 
		m_scale == other.m_scale && 
		m_rotation == other.m_rotation; 

}

bool C3DOrientationAndPosition::operator < (const C3DOrientationAndPosition& MIA_PARAM_UNUSED(other)) const
{
	assert(0 && "to be implemented"); 
}

E3DImageOrientation C3DOrientationAndPosition::get_axis_orientation() const
{
	return m_axisorder; 
}



const mia::C3DFVector& C3DOrientationAndPosition::get_scale() const
{
	return m_scale; 
}


const mia::C3DFVector& C3DOrientationAndPosition::get_origin() const
{
	return m_origin; 
}

const mia::Quaternion& C3DOrientationAndPosition::get_rotation() const
{
	return m_rotation; 
}


EXPORT_3D  std::ostream& operator << (std::ostream& os, E3DImageOrientation orient)
{
	os << g_image_orientation_map.get_name(orient); 
	return os;
}

EXPORT_3D std::istream& operator >> (std::istream& is, E3DImageOrientation& orient)
{
	string temp;
	is >> temp;
	orient = g_image_orientation_map.get_value(temp.c_str()); 
	return is;
}

const C3DFMatrix C3DOrientationAndPosition::ms_order_XYZ(C3DFVector(1,0,0), C3DFVector(0,1,0), C3DFVector(0,0,1)); 
const C3DFMatrix C3DOrientationAndPosition::ms_order_YXZ(C3DFVector(0,1,0), C3DFVector(1,0,0), C3DFVector(0,0,1));
const C3DFMatrix C3DOrientationAndPosition::ms_order_XZY(C3DFVector(1,0,0), C3DFVector(0,0,1), C3DFVector(0,1,0));
const C3DFMatrix C3DOrientationAndPosition::ms_order_ZXY(C3DFVector(0,0,1), C3DFVector(1,0,0), C3DFVector(0,1,0));
const C3DFMatrix C3DOrientationAndPosition::ms_order_ZYX(C3DFVector(0,0,1), C3DFVector(0,1,0), C3DFVector(1,0,0));
const C3DFMatrix C3DOrientationAndPosition::ms_order_YZX(C3DFVector(0,1,0), C3DFVector(0,0,1), C3DFVector(1,0,0));

const C3DFMatrix C3DOrientationAndPosition::ms_order_XYZ_F(C3DFVector(1,0,0), C3DFVector(0,1,0), C3DFVector(0,0,-1)); 
const C3DFMatrix C3DOrientationAndPosition::ms_order_YXZ_F(C3DFVector(0,1,0), C3DFVector(1,0,0), C3DFVector(0,0,-1));
const C3DFMatrix C3DOrientationAndPosition::ms_order_XZY_F(C3DFVector(1,0,0), C3DFVector(0,0,1), C3DFVector(0,-1,0));
const C3DFMatrix C3DOrientationAndPosition::ms_order_ZXY_F(C3DFVector(0,0,1), C3DFVector(1,0,0), C3DFVector(0,-1,0));
const C3DFMatrix C3DOrientationAndPosition::ms_order_ZYX_F(C3DFVector(0,0,1), C3DFVector(0,1,0), C3DFVector(-1,0,0));
const C3DFMatrix C3DOrientationAndPosition::ms_order_YZX_F(C3DFVector(0,1,0), C3DFVector(0,0,1), C3DFVector(-1,0,0));


static const TDictMap<E3DImageOrientation>::Table image_orientation_map[] = {
	{ "(undefined)", ior_undefined, "image orientation not defined"}, 
	{ "axial", ior_xyz, "transversal/axial"}, 
	{ "axial-flipped", ior_xyz_flipped, "transversal/axial z-order reverse"}, 
	{ "axial-transposed",  ior_yxz, "transversal/axial, transposed"}, 
	{ "axial-transposed-flipped", ior_yxz_flipped, "transversal/axial, z-order reverse, transposed"}, 

	{ "coronal", ior_xzy, "coronal,  face facing front"}, 
	{ "coronal-flipped", ior_xzy_flipped, "coronal, back facing front"}, 
	{ "coronal-transposed", ior_zxy, "coronal, face facing front, transposed"}, 
	{ "coronal-transposed-flipped", ior_zxy_flipped, "coronal, back facing front,  transposed"},

	{ "saggital", ior_yzx, "saggital,  left facing front"}, 
	{ "saggital-flipped", ior_yzx_flipped, "saggital, right facing front"}, 
	{ "saggital-transposed", ior_zyx, "saggital, left facing front, transposed"}, 
	{ "saggital-transposed-flipped", ior_zyx_flipped, "saggital, right facing front,  transposed"},
	
	{ nullptr, ior_undefined,  nullptr}
}; 

const TDictMap<E3DImageOrientation> g_image_orientation_map(image_orientation_map, true); 

static const TDictMap<E3DPatientPositioning>::Table patient_position_map_table[] = {
	{"(undefined)", ipp_undefined, "undefined patient position"}, 
	{"HFS", ipp_hfs, " head first supine "},  
	{"HFP", ipp_hfp, " head first prone "},  
	{"HFDR", ipp_hfdr, " head first Decubitus Right "},
	{"HFDL", ipp_hfdl, " head first Decubitus Left "},
	{"FFP", ipp_ffp, " feet first prone "},  
	{"FFS", ipp_ffs, " feet first supine "},  
	{"FFDR", ipp_ffdr, " feet first Decubitus Right "},
	{"FFDL", ipp_ffdl, " feet first Decubitus Left "}, 
	{0, ipp_undefined, 0}
}; 


const TDictMap<E3DPatientPositioning> g_patient_position_map(patient_position_map_table, true); 

EXPORT_3D  ostream& operator << (ostream& os, E3DPatientPositioning pp)
{
	os << g_patient_position_map.get_name(pp); 
	return os; 
}

EXPORT_3D  istream& operator >> (istream& is, E3DPatientPositioning& pp)
{
	string s; 
	is >> s; 
	pp = g_patient_position_map.get_value(s.c_str()); 
	return is; 
}

EXPORT_3D const char * IDPatientPosition = "PatientPosition"; 


void C3DOrientationAndPosition::print(std::ostream& os)const
{
	os << "[" << m_axisorder << " [" << m_origin << "] [" 
	   << m_scale << "] [" << m_rotation << "]]"; 
}

EXPORT_3D  std::istream& operator >> (std::istream& is, C3DOrientationAndPosition& orient)
{
	const char *msg = "unable to read C3DOrientationAndPosition from stream"; 
	E3DImageOrientation axisorder; 
	C3DFVector origin; 
	C3DFVector scale; 
	Quaternion rotation; 

	eat_char(is, '[', msg); 
	
	is >> axisorder; 

	eat_char(is, '[', msg); 

	is >> origin; 

	eat_char(is, ']', msg); 
	eat_char(is, '[', msg); 
	
	is >> scale; 
	
	eat_char(is, ']', msg); 
	eat_char(is, '[', msg); 
	
	
	is >> rotation; 
	
	eat_char(is, ']', msg); 
	eat_char(is, ']', msg); 

	orient = C3DOrientationAndPosition(axisorder, origin, scale, rotation);
	
	return is; 
}

template class  EXPORT_3D TTranslator<E3DImageOrientation>;
template class  EXPORT_3D TTranslator<E3DPatientPositioning>;
template class  EXPORT_3D TTranslator<C3DOrientationAndPosition>; 


NS_MIA_END
