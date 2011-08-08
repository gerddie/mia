/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

EXPORT_3D  std::ostream& operator << (std::ostream& os, E3DImageOrientation orient)
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

EXPORT_3D std::istream& operator >> (std::istream& is, E3DImageOrientation& orient)
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

NS_MIA_END
