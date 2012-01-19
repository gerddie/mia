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


#include <algorithm>
#include <climits>

#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/history.hh>
#include <mia/3d/3dvfio.hh>
#include <mia/3d/3dvfiotest.hh>

NS_MIA_BEGIN
using namespace boost;
using namespace std;
using namespace boost::unit_test;


const 	C3DFVector voxel(1.0, 2.0, 3.0);

static void check_save_load(const C3DIOVectorfield& field, const C3DVFIOPlugin& io, bool /*test_history_split*/)
{
	const string format = io.get_name();
	const string tmp_name = string("vftest_") + format;

	CHistory::instance().clear();
#if 0
	if (imgio.has_history()){
		// append some history
		popt::COptions options;
		CHistory::instance().append("test_vfio", revision, options);
	}
#endif

	BOOST_REQUIRE(io.save(tmp_name.c_str(), field));

	cvdebug() << format << "saved" << std::endl;

	// clear the history
	CHistory::instance().clear();

	C3DVFIOPlugin::PData reread(io.load(tmp_name.c_str()));

	BOOST_REQUIRE(reread.get());

	BOOST_REQUIRE(field.get_size() == reread->get_size());

	C3DIOVectorfield::const_iterator in_i = field.begin();
	C3DIOVectorfield::const_iterator in_e = field.end();

	C3DIOVectorfield::const_iterator lo_i = reread->begin();

	for (; in_i != in_e; ++in_i, ++lo_i) {
		BOOST_CHECK(equal(in_i, in_e, lo_i));
	}

	//BOOST_CHECK_EQUAL(voxel, reread->get_voxel_size());
#if 0
	if (io.has_history()) {
		// now check the history
		if (CHistory::instance().size() != 1) {
			cvfail() <<  format <<  " history failed, History size = " <<
				CHistory::instance().size() << endl;
			return false;
		}

		if (test_history_split) {
			CHistory::const_iterator i = CHistory::instance().begin();

			if (!check_entry("+PROGRAM", "test_imageio", i->second))
				return false;
			if (!check_entry("+LIBMIA_VERSION", PACKAGE_VERSION, i->second))
				return false;
			if (!check_entry("+USER", getenv("USER"), i->second))
				return false;
			if (!check_entry("+VERSION", revision, i->second))
				return false;
			if (!check_entry("+LIBMIA_REVISION", get_revision(), i->second))
				return false;
		}
	}
#endif
	unlink(tmp_name.c_str());

}

static void test_3dvfio_plugins(const C3DVFIOPluginHandler::value_type& p)
{

	C3DIOVectorfield field(C3DBounds(2,3,5));
	for (C3DIOVectorfield::iterator i = field.begin();  i != field.end(); ++i) {
		*i = C3DFVector(drand48(), drand48(), drand48());
	}


	//field.set_voxel_size(voxel);

	check_save_load(field, *p.second, p.second->has_property(io_plugin_property_history_split));
}



void EXPORT_3DTEST add_3dvfio_plugin_tests( test_suite* suite)
{
	C3DVFIOPluginHandler::const_iterator i = C3DVFIOPluginHandler::instance().begin();
	C3DVFIOPluginHandler::const_iterator e = C3DVFIOPluginHandler::instance().end();

	if (i != e)
		suite->add( BOOST_PARAM_TEST_CASE(&test_3dvfio_plugins, i, e ));
}

NS_MIA_END
