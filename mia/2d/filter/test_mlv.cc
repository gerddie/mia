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

#include <mia/internal/plugintester.hh>
#include <mia/2d/filter/mlv.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace mlv_2dimage_filter;

BOOST_AUTO_TEST_CASE( test_mlv )
{
#ifndef WIN32
	srand48(time(NULL));
#endif

	for (int width = 2; width < 12; ++width) {
		int w = width / 2;
		cvdebug() << "test filter of width " << w << "\n";
		ostringstream filter_descr; 
		filter_descr << "mlv:w=" << w; 
		auto filter = BOOST_TEST_create_from_plugin<C2DMLVImageFilterFactory>(filter_descr.str().c_str()); 

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

		const C2DBounds exsize(isize+fwidth,isize+fwidth);
		C2DFImage sigma(exsize);
		C2DFImage mu(exsize);


		for (int y = 0; y < isize + fwidth; ++y) {
			for (int x = 0; x < isize + fwidth; ++x) {
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


		for (int y = 0; y < isize; ++y) {
			for (int x = 0; x < isize; ++x) {
				float best_mu = 0.0;
				float best_sigma = numeric_limits<float>::max();

				for (int iy = y; iy < y + fwidth +1; ++iy)
					for (int ix = x; ix < x + fwidth +1; ++ix) {
						float mu_val = mu(ix, iy);
						if (best_sigma > sigma(ix, iy) ) {
							best_sigma = sigma(ix, iy);
							best_mu = mu_val;
						}else if (best_sigma == sigma(ix, iy) && best_mu != mu_val ) {
							const auto ref = (*src)(x,y); 
							if (::fabs(mu_val - ref) < ::fabs(best_mu - ref))
								best_mu = mu_val;
						}
					}
				ref(x,y) = best_mu;
			}

		}


		P2DImage src_wrap(src);
		P2DImage res_wrap = filter->filter(*src_wrap);
		C2DFImage * res = dynamic_cast<C2DFImage *>(res_wrap.get());
		BOOST_REQUIRE(res);

		BOOST_CHECK_EQUAL(res->get_size(), src->get_size());
		BOOST_REQUIRE(res->get_size() == src->get_size());

		for (auto ires = res->begin(), iref = ref.begin();
		     ires != res->end(); ++ires, ++iref) {
			BOOST_CHECK_CLOSE(*ires, *iref, 0.1);
		}		
		
		// redo to test 2nd code path 
		res_wrap = filter->filter(*src_wrap);
		auto res2 = dynamic_cast<const C2DFImage&>(*res_wrap);
		for (auto ires = res2.begin(), iref = ref.begin();
		     ires != res2.end(); ++ires, ++iref) {
			BOOST_CHECK_CLOSE(*ires, *iref, 0.1);
		}
		
		

	}
}
