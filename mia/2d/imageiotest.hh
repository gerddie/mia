/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_core_2dimageiotest_hh
#define mia_core_2dimageiotest_hh

#include <mia/core/defines.hh>
#include <boost/test/unit_test_suite.hpp>

NS_MIA_BEGIN

/// \cond INTERNAL_TEST 
void EXPORT_2DTEST test_2dimageio_plugins();
void EXPORT_2DTEST add_2dvfio_tests(boost::unit_test::test_suite* suite);
/// \endcond

NS_MIA_END

#endif
