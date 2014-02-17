/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_2d_io_getset_hh
#define mia_2d_io_getset_hh

template <typename T> 
struct findGetStore {
	static OFCondition applyGet(DcmFileFormat& dcm, const DcmTagKey &tagKey, 
				    T& value) {
		BOOST_STATIC_ASSERT(sizeof(T) == 0); 
		return OFCondition(); 
	}
	static OFCondition applyGet(DcmFileFormat& dcm, const DcmTagKey &tagKey, 
				    T& value, int pos) {
		BOOST_STATIC_ASSERT(sizeof(T) == 0); 
		return OFCondition(); 
	}

	static OFCondition applyMetaGet(DcmFileFormat& dcm, const DcmTagKey &tagKey, 
					const T& value) {
		BOOST_STATIC_ASSERT(sizeof(T) == 0); 
		return OFCondition(); 
	}

	static OFCondition applySet(DcmFileFormat& dcm, const DcmTagKey &tagKey, 
				    const T& value) {
		BOOST_STATIC_ASSERT(sizeof(T) == 0); 
		return OFCondition(); 
	}
	static OFCondition applySet(DcmFileFormat& dcm, const DcmTagKey &tagKey, 
				    const T& value, int pos) {
		BOOST_STATIC_ASSERT(sizeof(T) == 0); 
		return OFCondition(); 
	}

	static OFCondition applyMetaSet(DcmFileFormat& dcm, const DcmTagKey &tagKey, 
					const T& value) {
		BOOST_STATIC_ASSERT(sizeof(T) == 0); 
		return OFCondition(); 
	}
}; 

#ifdef HAVE_STD_STRING
inline void copy_OFString_to_std_string(const OFString& ofstr, string& str){
	str = ofstr; 
}
inline void copy_std_string_to_OFString(const string& str, OFString& ofstr){
	ofstr = str; 
}
#else 
inline void copy_OFString_to_std_string(const OFString& ofstr, string& str){
	str = string(ofstr.data()); 
}

inline void copy_std_string_to_OFString(const string& str, OFString& ofstr){
	ofstr = OFString(str.c_str(), str.size()); 
}
#endif


template <> 
struct findGetStore<string> {
	static OFCondition applyGet(DcmFileFormat& dcm, const DcmTagKey &tagKey, string& value) {
		OFString s; 
		OFCondition status = dcm.getDataset()->findAndGetOFStringArray(tagKey, s);
		copy_OFString_to_std_string(s, value); 
		return status; 
	}

	static OFCondition applyGet(DcmFileFormat& dcm, const DcmTagKey &tagKey, string& value, int pos) {
		OFString s; 
		OFCondition status = dcm.getDataset()->findAndGetOFString(tagKey, s, pos);
		copy_OFString_to_std_string(s, value); 
		return status; 
	}

	static OFCondition applyMetaGet(DcmFileFormat& dcm, const DcmTagKey &tagKey, string& value) {
		OFString s; 
		OFCondition status = dcm.getMetaInfo()->findAndGetOFStringArray(tagKey, s);
		copy_OFString_to_std_string(s, value); 
		return status; 
	}

	static OFCondition applySet(DcmFileFormat& dcm, const DcmTagKey &tagKey, 
				    const string& value) {
		OFString s; 
		copy_std_string_to_OFString(value, s); 
		return dcm.getDataset()->putAndInsertOFStringArray(tagKey, s);
	}
	static OFCondition applySet(DcmFileFormat& dcm, const DcmTagKey &tagKey, 
				    const string& value, int pos) {
		return dcm.getDataset()->putAndInsertString(tagKey, value.c_str(), pos);
	}
	static OFCondition applyMetaSet(DcmFileFormat& dcm, const DcmTagKey &tagKey, 
					const string& value) {
		return dcm.getMetaInfo()->putAndInsertString(tagKey, value.c_str());
	}
}; 

#define FIND_GET_SET_INSTANCE(TYPE) \
	template <> \
	struct findGetStore<TYPE> { \
		static OFCondition applyGet(DcmFileFormat& dcm, const DcmTagKey &tagKey, TYPE& value) { \
			return dcm.getDataset()->findAndGet##TYPE(tagKey, value); \
		}							\
		static OFCondition applyGet(DcmFileFormat& dcm, const DcmTagKey &tagKey, TYPE& value, int pos) { \
			return dcm.getDataset()->findAndGet##TYPE(tagKey, value, pos); \
		}							\
		static OFCondition applyMetaGet(DcmFileFormat& dcm, const DcmTagKey &tagKey, TYPE& value) { \
			return dcm.getMetaInfo()->findAndGet##TYPE(tagKey, value); \
		}							\
		static OFCondition applySet(DcmFileFormat& dcm, const DcmTagKey &tagKey, TYPE& value) { \
			cvdebug() << "insert " << tagKey << ": " << value << "\n"; \
			return dcm.getDataset()->putAndInsert##TYPE(tagKey, value); \
		}							\
		static OFCondition applySet(DcmFileFormat& dcm, const DcmTagKey &tagKey, TYPE& value, int pos) { \
			return dcm.getDataset()->putAndInsert##TYPE(tagKey, value, pos); \
		}							\
		static OFCondition applyMetaSet(DcmFileFormat& dcm, const DcmTagKey &tagKey, TYPE& value) { \
			return dcm.getMetaInfo()->putAndInsert##TYPE(tagKey, value); \
		}							\
	}

#define FIND_GET_SET_VECTOR_INSTANCE(TYPE) \
	template <> \
	struct findGetStore<vector<TYPE> > {				\
		static OFCondition applyGet(DcmFileFormat& dcm, const DcmTagKey &tagKey, vector<TYPE>& value) { \
			const TYPE *values; 					\
			long unsigned int count; 				\
			OFCondition cnd = dcm.getDataset()->findAndGet##TYPE##Array(tagKey, values, &count, false); \
			if (cnd.good()) {				\
				value.resize(count);			\
				copy(values, values+count, value.begin()); \
			}						\
			return cnd;					\
		}							\
		static OFCondition applyMetaGet(DcmFileFormat& dcm, const DcmTagKey &tagKey, vector<TYPE>& value) { \
			const TYPE *values; 					\
			long unsigned int count; 				\
			OFCondition cnd = dcm.getMetaInfo()->findAndGet##TYPE##Array(tagKey, values, &count, false); \
			if (cnd.good()) {				\
				value.resize(count);			\
				copy(values, values+count, value.begin()); \
			}						\
			return cnd;					\
		}							\
		static OFCondition applySet(DcmFileFormat& dcm, const DcmTagKey &tagKey, const vector<TYPE>& value) { \
			return dcm.getDataset()->putAndInsert##TYPE##Array(tagKey, &value[0], value.size(), false); \
		}							\
	}


FIND_GET_SET_INSTANCE(Uint16); 
FIND_GET_SET_INSTANCE(Uint32); 
FIND_GET_SET_INSTANCE(Float32); 

FIND_GET_SET_VECTOR_INSTANCE(Uint16); 

#undef FIND_GET_SET_INSTANCE

#endif
