/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#ifndef mia_core_pixeltype_hh
#define mia_core_pixeltype_hh

#include <mia/core/dictmap.hh>

NS_MIA_BEGIN

/** an enumerate for the different pixel types
    \remark replacing the values to become flags might be a better idea then a sequancial numbering
 */
enum EPixelType {it_bit,  /**< bit/bool pixels */
		 it_sbyte,  /**< signed byte pixels  (8-bit)*/
		 it_ubyte,  /**< unsigned byte pixels (8-bit) */
		 it_sshort, /**< signed short pixels (16-bit) */
		 it_ushort, /**< unsigned short pixels (16-bit) */
		 it_sint,   /**< signed int pixels (32-bit)*/
		 it_uint,   /**< unsigned int pixels (32-bit)*/
#ifdef HAVE_INT64
		 it_slong,  /**< signed long pixels (64 bit)*/
		 it_ulong,  /**< unsigned long pixels (64 bit) */
#endif
		 it_float,  /**< float pixels (32 bit)*/
		 it_double, /**< double pixels (63 bit)*/
		 it_none,   /**< type not set */
		 it_unknown /**< unexpected type */
};

enum EPixelConversion {pc_copy, /**< copy pixel values, ranges and values are truncated at conversion */
		       pc_linear, /**< apply the linear function ax+b to the voxel values, then convert the values */
		       pc_range, /**< apply a linear mapping the maps the complete input range of the input type to the 
				    complete output range of the output type, output to float and double 
				    is mapped to [-1,1] */
		       pc_opt, /**< apply a linear mapping the maps the actual input value range to the 
				    complete output value range, output to float and double is mapped to [-1,1] */
		       pc_opt_stat, /**< apply a linear mapping the maps the mean of the the actual input values 
				       to the mean of the output range and lets the output variation be 0.25 of 
				       the output range */
		       pc_unknown};

EXPORT_CORE extern const TDictMap<EPixelType>::Table PixelTypeTable[];
EXPORT_CORE extern const TDictMap<EPixelType> CPixelTypeDict;

EXPORT_CORE extern const TDictMap<EPixelConversion>::Table ConversionTypeTable[];
EXPORT_CORE extern const TDictMap<EPixelConversion> CPixelConversionDict;

/**
   A type-traits class to map pixel types to the according pixel type value.
*/
template <typename T>
struct pixel_type {
	enum {value = it_unknown};
};

template <>
struct pixel_type<bool> {
	enum {value = it_bit};
};

template <>
struct pixel_type<signed char> {
	enum {value = it_sbyte};
};

template <>
struct pixel_type<unsigned char> {
	enum {value = it_ubyte};
};

template <>
struct pixel_type<signed short> {
	enum {value = it_sshort};
};

template <>
struct pixel_type<unsigned short> {
	enum {value = it_ushort};
};

template <>
struct pixel_type<signed int> {
	enum {value = it_sint};
};

template <>
struct pixel_type<unsigned int> {
	enum {value = it_uint};
};

#ifdef HAVE_INT64
template <>
struct pixel_type<mia_int64> {
	enum {value = it_slong};
};

template <>
struct pixel_type<mia_uint64> {
	enum {value = it_ulong};
};
#endif

template <>
struct pixel_type<float> {
	enum {value = it_float};
};

template <>
struct pixel_type<double> {
	enum {value = it_double};
};


NS_MIA_END

#endif
