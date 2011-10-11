/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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


#include <mia/core/pixeltype.hh>

NS_MIA_BEGIN
const TDictMap<EPixelType>::Table PixelTypeTable[] = {
	{"bit",   it_bit, "binary data"},
	{"ubyte", it_ubyte, "unsigned 8 bit"},
	{"sbyte", it_sbyte, "signed 8 bit"},
	{"sshort",it_sshort, "unsigned 16 bit"},
	{"ushort",it_ushort, "signed 16 bit"},
	{"sint",  it_sint, "unsigned 32 bit"},
	{"uint",  it_uint, "signed 32 bit"},
#ifdef HAVE_INT64
	{"slong", it_slong, "unsigned 64 bit"},
	{"ulong", it_ulong}, "signed 64 bit",
#endif
	{"float", it_float, "single floating point values (32 bit)"},
	{"double",it_double, "souble floating point values (64 bit)"},
	{NULL, it_unknown, ""}
};
const TDictMap<EPixelType> CPixelTypeDict(PixelTypeTable);


const TDictMap<EPixelConversion>::Table ConversionTypeTable[] = {
	{"copy", pc_copy, "copy data when converting"},
	{"linear", pc_linear, "apply linear transform x-> ax+b"},
	{"range", pc_range, "apply linear transform that maps input data type range to output data type range"},
	{"opt", pc_opt, "apply a linear transform that applies the real input range to map to the full output range"},
	{"optstat", pc_opt_stat, "apply a linear transform that mappes based on input mean and variation the full output range"},
	{NULL, pc_unknown, ""}
};
const TDictMap<EPixelConversion> CPixelConversionDict(ConversionTypeTable);


NS_MIA_END
