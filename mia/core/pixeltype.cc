/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <mia/core/pixeltype.hh>

NS_MIA_BEGIN
const TDictMap<EPixelType>::Table PixelTypeTable[] = {
	{"bit",   it_bit, "binary data"},
	{"ubyte", it_ubyte, "unsigned 8 bit"},
	{"sbyte", it_sbyte, "signed 8 bit"},
	{"sshort",it_sshort, "signed 16 bit"},
	{"ushort",it_ushort, "unsigned 16 bit"},
	{"sint",  it_sint, "signed 32 bit"},
	{"uint",  it_uint, "unsigned 32 bit"},
	{"slong", it_slong, "signed 64 bit"},
	{"ulong", it_ulong, "unsigned 64 bit"},
	{"float", it_float, "floating point 32 bit"},
	{"double",it_double, "floating point 64 bit"},
	{"none", it_none, "no pixel type defined"}, 
	{NULL, it_unknown, ""}
};
const TDictMap<EPixelType> CPixelTypeDict(PixelTypeTable);


const TDictMap<EPixelConversion>::Table ConversionTypeTable[] = {
	{"copy", pc_copy, "copy data when converting"},
	{"linear", pc_linear, "apply linear transformation x -> a*x+b"},
	{"range", pc_range, "apply linear transformation that maps the input data type range to the output data type range"},
	{"opt", pc_opt, "apply a linear transformation that maps the real input range to the full output range"},
	{"optstat", pc_opt_stat, "apply a linear transform that maps based on input mean and variation to the full output range"},
	{NULL, pc_unknown, ""}
};
const TDictMap<EPixelConversion> CPixelConversionDict(ConversionTypeTable);


NS_MIA_END
