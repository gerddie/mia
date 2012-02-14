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

#include <type_traits>

#include <mia/core/attributes.hh>
#include <mia/test/testhelpers.hh>

#ifndef mia_template_filtertest_hh
#define mia_template_filtertest_hh

NS_MIA_BEGIN

template <template <class> class Image> 
class TFiltertestFixture {
	typedef typename Image<int>::dimsize_type dimsize_type; 
public: 
	
	TFiltertestFixture():m_attr(new CStringAttribute("teststring")) 
	{
	}

	

	template <typename Filter, typename IN, typename OUT> 
	void run(const dimsize_type& in_size, const IN *init_data, 
		 const dimsize_type& out_size, const OUT *test_data, const Filter& f) {
		Image<IN> image(in_size, init_data); 
		image.set_attribute("test_attr", m_attr); 
		
		auto result = f.filter(image); 
		
		// test attribute transfer 
		auto result_attr = result->get_attribute("test_attr"); 
		BOOST_REQUIRE(result_attr); 
		BOOST_CHECK_EQUAL(*result_attr, *m_attr); 


		// test output data 
		auto r =  dynamic_cast<const Image<OUT>&>(*result);

		BOOST_CHECK_EQUAL(r.get_size(), out_size); 
		BOOST_REQUIRE(r.get_size()== out_size); 
		
		const OUT *t = test_data; 
		auto i = r.begin_range(dimsize_type::_0, out_size); 
		auto e = r.end_range(dimsize_type::_0, out_size); 
		while (i != e) {
			if (*i !=  *t) 
				cvfail() << i.pos() << "\n"; 
			miatest::equal_or_close(*i, *t);
			++i; ++t; 
		}
	}
private: 
	PAttribute m_attr; 
}; 

NS_MIA_END

#endif
