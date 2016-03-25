/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#define GSL_VECTOR_DISPATCH(TYPE)		 \
	template <>				 \
	struct gsl_vector_dispatch<TYPE> {	 \
		typedef TYPE value_type;	 \
		typedef TYPE *iterator;			\
		typedef const TYPE *const_iterator;	\
		typedef size_t size_type;		\
		typedef TYPE& reference;		\
		typedef const TYPE& const_reference;	\
							\
		typedef gsl_value_type gsl_vector_##TYPE		\
		protected:						\
		static gsl_value_type *alloc(size_t n) {		\
			return gsl_vector_##TYPE##_alloc(n);		\
		}							\
		static gsl_value_type *calloc(size_t n) {		\
			return gsl_vector_##TYPE##_calloc(n);		\
		}							\
		static gsl_vector_type *alloc_from(gsl_vector_type *other) { \
			return gsl_vector_#TYPE#_alloc_from_vector(other, 0, \
								   other->size, 1); \
		}							\
		static void free(gsl_value_type *v) {			\
			gsl_vector_##TYPE##_free(v);			\
		}							\
		static value_type get(gsl_vector_type *v, size_t i) {	\
			gsl_vector_##TYPE##_get(v,i);			\
		}							\
	}; 
