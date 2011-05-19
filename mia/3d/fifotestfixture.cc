/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif

#include <climits>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <mia/core/msgstream.hh>
#include <mia/2d/imagetest.hh>
#include <mia/3d/fifotestfixture.hh>

NS_MIA_USE;

void fifof_Fixture::call_test(C2DImageFifoFilter& filter)const
{
	typedef TFifoFilterSink<P2DImage> C2DImageFifoFilterSink;

	C2DImageFifoFilterSink::Pointer sink(new C2DImageFifoFilterSink());
	filter.append_filter(sink);

	for (C2DImageStack::const_iterator i = m_in_data.begin();
	     i != m_in_data.end(); ++i)
		filter.push(*i);

	filter.finalize();
	C2DImageFifoFilterSink::result_type r = sink->result();

	BOOST_CHECK_EQUAL(r.size(), m_test_data.size());
	BOOST_REQUIRE(r.size() == m_test_data.size());

	for (size_t i = 0; i < r.size(); ++i) {
		cvdebug() << "fifof_Fixture: test slice " << i << "\n"; 
		test_image_equal(*r[i], *m_test_data[i]);
	}
}


