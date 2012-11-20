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


#include <sstream>
#include <iterator>
#include <mia/core.hh>
#include <mia/3d/vector.hh>
#include <vistaio/vistaio.h>

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
	enum {value = VistaIOUnknownRepn};
	typedef void type;
};

template <>
struct vista_repnkind<unsigned char> {
	enum {value = VistaIOUByteRepn};
	typedef VistaIOUByte type;
};

template <>
struct vista_repnkind<signed char> {
	enum {value = VistaIOSByteRepn};
	typedef VistaIOSByte type;
};

template <>
struct vista_repnkind<short> {
	enum {value = VistaIOShortRepn};
	typedef VistaIOShort type;
};

template <>
struct vista_repnkind<int> {
	enum {value = VistaIOLongRepn};
	typedef VistaIOLong type;
};

template <>
struct vista_repnkind<float> {
	enum {value = VistaIOFloatRepn};
	typedef VistaIOFloat type;
};

template <>
struct vista_repnkind<double> {
	enum {value = VistaIODoubleRepn};
	typedef VistaIODouble type;
};

template <>
struct vista_repnkind<bool> {
	enum {value = VistaIOBitRepn};
	typedef VistaIOBit type;
};

template <>
struct vista_repnkind<std::string> {
	enum {value = VistaIOStringRepn};
	typedef VistaIOString type;
};


template <typename I, typename O>
struct dispatch_creat_vimage {
	static VistaIOImage apply(I begin, I end, size_t x, size_t y, size_t z) {
		VistaIOImage result = VistaIOCreateImage(z, y, x, (VistaIORepnKind)vista_repnkind<typename std::iterator_traits<I>::value_type>::value);
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
VISTA4MIA_EXPORT void copy_attr_list(mia::CAttributedData& attributes, const VistaIOAttrList target);

/**
   Helper class to indice atomatic destruction of vista attribute lists. 
 */
class CVAttrList {
public: 
	CVAttrList(VistaIOAttrList list); 
	~CVAttrList(); 
	operator VistaIOAttrList(); 
	bool operator !() const; 
	
	VistaIOAttrList operator ->(); 
private: 
	VistaIOAttrList m_list; 
		
}; 

NS_MIA_END



