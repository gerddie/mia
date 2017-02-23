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

#include <sstream>
#include <iterator>
#include <mia/core.hh>
#include <mia/3d/vector.hh>
#include <vistaio/vistaio.h>
#include <vistaio/vconfig.h>

#ifdef WIN32
#ifdef vista4mia_EXPORTS
#define VISTA4MIA_EXPORT __declspec(dllexport)
#else
#define VISTA4MIA_EXPORT __declspec(dllimport)
#endif
#else
#ifdef __GNUC__
#    define VISTA4MIA_EXPORT __attribute__((visibility("default")))
#else 
#    define VISTA4MIA_EXPORT
#endif
#endif

template <typename T>
struct vista_repnkind {
	static const VistaIORepnKind value = VistaIOUnknownRepn;
	static const bool is_unsigned = false; 
	typedef void type;
};

template <>
struct vista_repnkind<unsigned char> {
	static const VistaIORepnKind value = VistaIOUByteRepn;
	static const bool is_unsigned = true; 
	typedef VistaIOUByte type;
};

template <>
struct vista_repnkind<signed char> {
	static const VistaIORepnKind value = VistaIOSByteRepn;
	static const bool is_unsigned = false; 
	typedef VistaIOSByte type;
};

template <>
struct vista_repnkind<short> {
	static const VistaIORepnKind value = VistaIOShortRepn;
	static const bool is_unsigned = false; 
	typedef VistaIOShort type;
};

template <>
struct vista_repnkind<int> {
	static const VistaIORepnKind value = VistaIOLongRepn;
	static const bool is_unsigned = false; 
	typedef VistaIOLong type;
};

template <>
struct vista_repnkind<unsigned short> {
	static const VistaIORepnKind value = VistaIOShortRepn;
	static const bool is_unsigned = true; 
	typedef VistaIOShort type;
};

template <>
struct vista_repnkind<unsigned int> {
	static const VistaIORepnKind value = VistaIOLongRepn;
	static const bool is_unsigned = true; 
	typedef VistaIOLong type;
};

#if VISTAIO_MAJOR_VERSION > 1 || \
	VISTAIO_MAJOR_VERSION == 1 && VISTAIO_MINOR_VERSION > 2 || \
	VISTAIO_MAJOR_VERSION == 1 && VISTAIO_MINOR_VERSION ==  2 && VISTAIO_MICRO_VERSION > 19

template <>
struct vista_repnkind<int64_t> {
	static const VistaIORepnKind value = VistaIOLong64Repn;
	static const bool is_unsigned = false; 
	typedef VistaIOLong64 type;
};
#endif 


template <>
struct vista_repnkind<float> {
	static const VistaIORepnKind value = VistaIOFloatRepn;
	static const bool is_unsigned = false; 
	typedef VistaIOFloat type;
};

template <>
struct vista_repnkind<double> {
	static const VistaIORepnKind value = VistaIODoubleRepn;
	static const bool is_unsigned = false; 
	typedef VistaIODouble type;
};

template <>
struct vista_repnkind<bool> {
	static const VistaIORepnKind value = VistaIOBitRepn;
	static const bool is_unsigned = false; 
	typedef VistaIOBit type;
};

template <>
struct vista_repnkind<std::string> {
	static const VistaIORepnKind value = VistaIOStringRepn;
	static const bool is_unsigned = false; 
	typedef VistaIOString type;
};


template <typename I, typename O>
struct dispatch_creat_vimage {
	static VistaIOImage apply(I begin, I end, size_t x, size_t y, size_t z) {
		VistaIOImage result = VistaIOCreateImage(z, y, x, vista_repnkind<O>::value);
		assert(result); 
		
		VistaIOSetAttr(VistaIOImageAttrList(result), "repn-unsigned", NULL, VistaIOBitRepn, 
			       vista_repnkind<O>::is_unsigned);

		std::copy(begin, end, (O *)VistaIOPixelPtr(result,0,0,0));
		return result;
	}
};

template <typename I>
struct dispatch_creat_vimage<I, void> {
	static VistaIOImage apply(I /*begin*/, I /*end*/, size_t /*x*/, size_t /*y*/, size_t /*z*/) {
		throw std::invalid_argument("unsupported pixel format in saving to vista");
		return NULL;
	}
};

NS_MIA_BEGIN

VISTA4MIA_EXPORT void copy_attr_list(VistaIOAttrList target, const mia::CAttributedData& attributes);
VISTA4MIA_EXPORT void copy_attr_list(mia::CAttributedData& attributes, const VistaIOAttrList source);

/**
   Helper class to indice atomatic destruction of vista attribute lists. 
 */
class VISTA4MIA_EXPORT CVAttrList {
public: 
	CVAttrList(VistaIOAttrList list); 
	~CVAttrList(); 
	operator VistaIOAttrList(); 
	bool operator !() const; 
	
	VistaIOAttrList operator ->(); 
private: 
	VistaIOAttrList m_list; 
		
}; 

VISTA4MIA_EXPORT void add_standard_vistaio_properties(CPluginBase& plugin); 

NS_MIA_END



