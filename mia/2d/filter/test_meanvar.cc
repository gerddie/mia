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

#include <mia/internal/plugintester.hh>
#include <mia/2d/filter/meanvar.hh>
#include <mia/2d/imageio.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace meanvar_2dimage_filter;

BOOST_AUTO_TEST_CASE( test_meanvar_without_thresh )
{
#ifndef WIN32
	// deterministic test, aleays use the same seed 
	srand48(0);
#endif

	for (int width = 2; width < 12; ++width) {
		int w = width / 2;
		cvdebug() << "test filter of width " << w << "\n";
		ostringstream filter_descr; 
		filter_descr << "meanvar:w=" << w << ",varfile=varfile.@";
		cvdebug() << "Testing " << filter_descr.str() << "\n"; 
		auto filter = BOOST_TEST_create_from_plugin<C2DMeanVarImageFilterFactory>(filter_descr.str().c_str()); 

		int isize = 4 * w + 1;
		int fwidth = 2 * w;
		C2DBounds size(isize, isize);

		C2DFImage *src = new C2DFImage(size);
		for (C2DFImage::iterator i = src->begin(), e = src->end(); i != e; ++i)
#ifndef WIN32
			*i = drand48();
#else
			*i = 1.0f; // windows doesnt have drand48
#endif

		// create the reference image
		C2DFImage ref(src->get_size());

		C2DFImage sigma(size);
		C2DFImage mu(size);


		for (int y = 0; y < isize; ++y) {
			for (int x = 0; x < isize; ++x) {
				float sum  = 0.0f;
				float sum2 = 0.0f;
				int n = 0;

				for (int iy = max(0, y - fwidth); iy < min(isize, y + 1); ++iy)
					for (int ix = max(0, x - fwidth); ix < min(isize, x + 1); ++ix) {
						float const& val = (*src)(ix,iy);
						sum += val;
						sum2 += val * val;
						++n;
					}
				float m = n>0 ? mu(x,y) = sum / n : 0;
				sigma(x,y) = (n > 1) ? (sum2 - n * m * m) / (n - 1) : 0.0f;
			}

		}
		
		P2DImage src_wrap(src);
		P2DImage res_mu_wrap = filter->filter(*src_wrap);
		C2DFImage * res_mu = dynamic_cast<C2DFImage *>(res_mu_wrap.get());
		BOOST_REQUIRE(res_mu);

		BOOST_CHECK_EQUAL(res_mu->get_size(), src_wrap->get_size());
		BOOST_REQUIRE(res_mu->get_size() == src_wrap->get_size());

		for (auto ires = res_mu->begin(), iref = mu.begin();
		     ires != res_mu->end(); ++ires, ++iref) {
			BOOST_CHECK_CLOSE(*ires, *iref, 0.1);
		}		

		P2DImage res_sigma_wrap = load_image2d("varfile.@");
		BOOST_REQUIRE(res_sigma_wrap); 
		C2DFImage *res_sigma = dynamic_cast<C2DFImage *>(res_sigma_wrap.get());
		BOOST_REQUIRE(res_sigma);
		
		BOOST_CHECK_EQUAL(res_sigma->get_size(), src_wrap->get_size());
		BOOST_REQUIRE(res_sigma->get_size() == src_wrap->get_size());

		for (auto ires = res_sigma->begin(), iref = sigma.begin();
		     ires != res_sigma->end(); ++ires, ++iref) {
			BOOST_CHECK_CLOSE(*ires, *iref, 0.1);
		}		
	}
}
