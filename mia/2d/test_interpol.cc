/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/test/test_case_template.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/list.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/filter.hh>

#include <mia/2d/interpolator.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/deformer.hh>

NS_MIA_USE
using namespace std;
using namespace boost;

namespace bmpl=boost::mpl;

CSplineKernelTestPath init_splinekernel_path; 

template <class Data2D, class Interpolator>
void test_interpolator(const Data2D& data, const Interpolator& src)
{

	typename Data2D::const_iterator i = data.begin();

	for (size_t y = 0; y < data.get_size().y; ++y)
		for (size_t x = 0; x < data.get_size().x; ++x, ++i) {
			float fx = float(x), fy = float(y);
			C2DFVector loc(fx,fy);
			typename Data2D::value_type v = src(loc);
			BOOST_CHECK_CLOSE(1.0 + v, 1.0 + *i, 0.1);
		}
}


template <class T, template <class> class Interpolator>
void test_direct_interpolator(const T2DDatafield<T>& data)
{
	Interpolator<T> src(data);
	test_interpolator(data, src);
}


template <class T>
void test_conv_interpolator(const T2DDatafield<T>& data, PSplineKernel kernel)
{
	T2DConvoluteInterpolator<T>  src(data, kernel);
	test_interpolator(data, src);
}

double omoms3(double x)
{
	if (x < 0)
		x = -x;

	if (x >= 2)
		return 0;
	if (x >= 1)
		return ((1 - 1.0 / 6.0 * x) * x - 85.0/42.0 ) * x + 29.0 / 21.0;

	return ((0.5 * x - 1) * x + 1/14.0) * x + 13.0 / 21.0;
}

BOOST_AUTO_TEST_CASE( test_omoms3 )
{
	const double x = 0.2;
	auto  kernel = CSplineKernelPluginHandler::instance().produce("omoms:d=3");
	std::vector<double> weights(kernel->size());
	kernel->get_weights(x, weights);

	for (size_t i = 0; i < weights.size(); ++i) {
		BOOST_CHECK_CLOSE(weights[3 - i], omoms3( x - 2.0 + i), 1e-4);
	}
}

template <typename T, bool is_float>
struct __dispatch_compare {
	static void apply(T a, T b) {
		BOOST_CHECK( a == b);
	}
};

template <typename T>
struct __dispatch_compare<T, true> {
	static void apply(T a, T b) {
		BOOST_CHECK_CLOSE(a, b, 0.01);
	}
};




struct FCompareImages: public TFilter<bool> {
	template <typename T>
	bool operator ()( const T2DImage<T>& a, const T2DImage<T>& b) const {
		typedef typename T2DImage<T>::const_iterator II;
		for (II ai = a.begin(), bi = b.begin(); ai != a.end(); ++ai, ++bi) {
			__dispatch_compare<T, ::boost::is_floating_point<T>::value>::apply(*ai,*bi);
		}
		return true;
	}
};

void save(P2DImage image, const string& name)
{
	C2DImageIOPluginHandler::Instance::Data vlist;
	vlist.push_back(image);
	C2DImageIOPluginHandler::instance().save(name, vlist);
}

void test_deformadd()
{
	C2DBounds size(256, 256);

	C2DUSImage *fimage = new C2DUSImage(size);
	P2DImage image(fimage);

	std::shared_ptr<C2DInterpolatorFactory > ipf(create_2dinterpolation_factory(ip_linear, bc_mirror_on_bounds));

	C2DFVectorfield A(size);
	C2DFVectorfield B(size);

	C2DFVectorfield::iterator a = A.begin();
	C2DFVectorfield::iterator b = B.begin();
	C2DUSImage::iterator i = fimage->begin();

	for (size_t y = 0; y < size.y; ++y) {
		float my = (128.0f - y);
		my *= my;
		for (size_t x = 0; x < size.x; ++x, ++i, ++a, ++b) {
			float mx = (128.0f - x);
			mx *= mx;
			*i = x % 16 &&  y % 16  ? 0: 48000;
			*a = C2DFVector( mx / (16 * y + 1), 0.0);
			*b = C2DFVector( 0.0, my / ( 16 *  x + 1));
		}
	}

	P2DImage im = filter(FDeformer2D(A, *ipf), *image);
	P2DImage result_add = filter(FDeformer2D(B, *ipf), *im);

	A += B;

	P2DImage result_direct = filter(FDeformer2D(A, *ipf), *image);

	if (!filter_equal(FCompareImages(), *result_direct, *result_add)) {
		save(image, "original.png");
		save(im, "inter.png");
		save(result_direct, "result_direct.png");
		save(result_add, "result_add.png");
	}

}



typedef bmpl::list<signed char,
		   unsigned char,
		   signed short,
		   unsigned short,
		   signed int,
		   unsigned int,
		   float,
		   double
#ifdef HAVE_INT64
		   mia_int64,
		   mia_uint64,
#endif
		   > test_types;



template <typename T>
T2DDatafield<T> create_data()
{
	T2DDatafield<T> data(C2DBounds(10, 12));
	typename T2DDatafield<T>::iterator i = data.begin();
	for (size_t y = 0; y < data.get_size().y; ++y)
		for (size_t x = 0; x < data.get_size().x; ++x, ++i)
			*i = T(y + x);
	return data;
}


BOOST_AUTO_TEST_CASE_TEMPLATE( test_interpolateNN_type, T, test_types )
{
	T2DDatafield<T> data = create_data<T>();
	test_conv_interpolator<T>(data, CSplineKernelPluginHandler::instance().produce("bspline:d=0"));
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_interpolateBiLin_type, T, test_types )
{
	T2DDatafield<T> data = create_data<T>();
	test_conv_interpolator<T>(data, CSplineKernelPluginHandler::instance().produce("bspline:d=1"));
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_interpolateBSpline2_type, T, test_types )
{
	T2DDatafield<T> data = create_data<T>();
	test_conv_interpolator<T>(data, CSplineKernelPluginHandler::instance().produce("bspline:d=2"));
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_interpolateBSpline3_type, T, test_types )
{
	T2DDatafield<T> data = create_data<T>();
	test_conv_interpolator<T>(data, CSplineKernelPluginHandler::instance().produce("bspline:d=3"));
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_interpolateBSpline4_type, T, test_types )
{
	T2DDatafield<T> data = create_data<T>();
	test_conv_interpolator<T>(data, CSplineKernelPluginHandler::instance().produce("bspline:d=4"));
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_interpolateBSpline5_type, T, test_types )
{
	T2DDatafield<T> data = create_data<T>();
	test_conv_interpolator<T>(data, CSplineKernelPluginHandler::instance().produce("bspline:d=5"));
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_interpolateOMoms3_type, T, test_types )
{
	T2DDatafield<T> data = create_data<T>();
	test_conv_interpolator<T>(data, CSplineKernelPluginHandler::instance().produce("omoms:d=3"));
}


