/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
#include <mia/3d/3DVector.hh>
#include <vistaio/vistaio.h>

#ifdef WIN32
#ifdef vista4mia_EXPORTS
#define VISTA4MIA_EXPORT __declspec(dllexport)
#else
#define VISTA4MIA_EXPORT __declspec(dllimport)
#endif
#else
#define VISTA4MIA_EXPORT
#endif

template <typename T>
struct vista_repnkind {
	enum {value = VUnknownRepn};
	typedef void type;
};

template <>
struct vista_repnkind<unsigned char> {
	enum {value = VUByteRepn};
	typedef VUByte type;
};

template <>
struct vista_repnkind<signed char> {
	enum {value = VSByteRepn};
	typedef VSByte type;
};

template <>
struct vista_repnkind<short> {
	enum {value = VShortRepn};
	typedef VShort type;
};

template <>
struct vista_repnkind<int> {
	enum {value = VLongRepn};
	typedef VLong type;
};

template <>
struct vista_repnkind<float> {
	enum {value = VFloatRepn};
	typedef VFloat type;
};

template <>
struct vista_repnkind<double> {
	enum {value = VDoubleRepn};
	typedef VDouble type;
};

template <>
struct vista_repnkind<bool> {
	enum {value = VBitRepn};
	typedef VBit type;
};

template <>
struct vista_repnkind<std::string> {
	enum {value = VStringRepn};
	typedef VString type;
};


template <typename I, typename O>
struct dispatch_creat_vimage {
	static VImage apply(I begin, I end, size_t x, size_t y, size_t z) {
		VImage result = VCreateImage(z, y, x, (VRepnKind)vista_repnkind<typename std::iterator_traits<I>::value_type>::value);
		std::copy(begin, end, (O *)VPixelPtr(result,0,0,0));
		return result;
	}
};

template <typename I>
struct dispatch_creat_vimage<I, void> {
	static VImage apply(I /*begin*/, I /*end*/, size_t /*x*/, size_t /*y*/, size_t /*z*/) {
		throw std::invalid_argument("unsupported pixel format in saving to vista");
		return NULL;
	}
};

NS_MIA_BEGIN

VISTA4MIA_EXPORT void copy_attr_list(VAttrList target, const mia::CAttributeMap& attributes);
VISTA4MIA_EXPORT void copy_attr_list(mia::CAttributeMap& attributes, const VAttrList target);

NS_MIA_END



