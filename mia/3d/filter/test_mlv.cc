/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/internal/autotest.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/filter/mlv.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace mlv_3dimage_filter;

struct FCompare {
	bool operator () (float a, float b){
		return ::fabs(a-b) < 0.01;
	}
};

BOOST_AUTO_TEST_CASE( test_mlv )
{
#ifndef WIN32
	// the test should always use the same random numbers 
	srand48(0);
#endif
	for (int w = 1; w < 5; ++w) {
		cvdebug() << "test filter of width " << w << "\n";

		C3DMLVImageFilter filter(w);

		int isize = 4 * w + 1;
		int fwidth = 2 * w;
		C3DBounds size(isize, isize, isize);

		C3DFImage *src = new C3DFImage(size);
		for (C3DFImage::iterator i = src->begin(), e = src->end(); i != e; ++i)
#ifndef WIN32
			*i = drand48();
#else
			*i = float(rand())/RAND_MAX;
#endif

               // create the reference image
		C3DFImage ref(src->get_size());

		const C3DBounds exsize(isize+fwidth,isize+fwidth,isize+fwidth);
		C3DFImage sigma(exsize);
		C3DFImage mu(exsize);

		for (int z = 0; z < isize + fwidth; ++z) {
			for (int y = 0; y < isize + fwidth; ++y) {
				for (int x = 0; x < isize + fwidth; ++x) {
				float sum  = 0.0f;
					float sum2 = 0.0f;
					int n = 0;

					for (int iz = max(0, z - fwidth); iz < min(isize, z + 1); ++iz)
						for (int iy = max(0, y - fwidth); iy < min(isize, y + 1); ++iy)
							for (int ix = max(0, x - fwidth); ix < min(isize, x + 1); ++ix) {
								float const& val = (*src)(ix,iy,iz);
								sum += val;
								sum2 += val * val;
								++n;
							}
					float m = mu(x,y,z) = sum / n;
					sigma(x,y,z) = (n > 1) ? (sum2 - n * m * m) / (n - 1) : 0.0f;
				}

			}
		}

		C3DFImage::const_iterator i_src = src->begin();
		for (int z = 0; z < isize; ++z) {
			for (int y = 0; y < isize; ++y) {
				for (int x = 0; x < isize; ++x, ++i_src) {
					float best_mu = 0.0;
					float best_sigma = numeric_limits<float>::max();
					for (int iz = z; iz < z + fwidth +1; ++iz)
						for (int iy = y; iy < y + fwidth +1; ++iy)
							for (int ix = x; ix < x + fwidth +1; ++ix) {
								float mu_val = mu(ix, iy, iz);
								if (best_sigma > sigma(ix, iy, iz) ) {
									best_sigma = sigma(ix, iy, iz);
									best_mu = mu_val;
								}else if (best_sigma == sigma(ix, iy, iz) && best_mu != mu_val ) {
									if (::fabs(mu_val - *i_src) < ::fabs(*i_src - best_mu))
										best_mu = mu_val;
								}
							}
					ref(x,y,z) = best_mu;
				}
			}
		}


		P3DImage src_wrap(src);
		P3DImage result = filter.filter(*src_wrap);

		const C3DFImage& presult = dynamic_cast<const C3DFImage&>(*result);
		BOOST_CHECK(equal(presult.begin(), presult.end(), ref.begin(), FCompare()));
	}
}
