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

#ifndef mia_core_2dimageiotest_hh
#define mia_core_2dimageiotest_hh

#include <boost/test/unit_test_suite.hpp>

#include <mia/core/defines.hh>
#include <mia/2d/defines2d.hh>

NS_MIA_BEGIN

/// \cond INTERNAL_TEST
void EXPORT_2DTEST imagefiltertest2d_prepare_plugin_path();
void EXPORT_2DTEST add_2dfilter_plugin_test(boost::unit_test::test_suite* suite);
/// \endcond

NS_MIA_END

#endif
