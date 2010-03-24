/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
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
	{"bit",   it_bit},
	{"ubyte", it_ubyte},
	{"sbyte", it_sbyte},
	{"sshort",it_sshort},
	{"ushort",it_ushort},
	{"sint",  it_sint},
	{"uint",  it_uint},
#ifdef HAVE_INT64
	{"slong", it_slong},
	{"ulong", it_ulong},
#endif
	{"float", it_float},
	{"double",it_double},
	{NULL, it_unknown}
};
const TDictMap<EPixelType> CPixelTypeDict(PixelTypeTable);


const TDictMap<EPixelConversion>::Table ConversionTypeTable[] = {
	{"copy", pc_copy},
	{"linear", pc_linear},
	{"range", pc_range},
	{"opt", pc_opt},
	{NULL, pc_unknown}
};
const TDictMap<EPixelConversion> CPixelConversionDict(ConversionTypeTable);


NS_MIA_END
