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

#include <mia/3d/orientation.hh>

NS_MIA_BEGIN
using namespace std; 


EXPORT_3D  ostream& operator << (ostream& os, E3DImageOrientation orient)
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

EXPORT_3D istream& operator >> (istream& is, E3DImageOrientation& orient)
{
	string temp;
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
	{0, ipp_lastindex, 0}
}; 


const TDictMap<E3DPatientPositioning> g_patient_position_map(patient_position_map_table); 

EXPORT_3D  ostream& operator << (ostream& os, E3DPatientPositioning pp)
{
	os << g_patient_position_map.get_name(pp); 
	return os; 
}

EXPORT_3D  istream& operator >> (istream& is, E3DPatientPositioning& pp)
{
	string s; 
	is >> s; 
	try {
		pp = g_patient_position_map.get_value(s.c_str()); 
	}catch (const invalid_argument& e) {
		cvwarn() << "Reading unknown patient position '" << s 
			 << "', defaulting to '(undefined)'\n"; 
		pp = ipp_undefined; 
	}
	return is; 
}

EXPORT_3D const char * IDPatientPosition = "PatientPosition"; 

NS_MIA_END
