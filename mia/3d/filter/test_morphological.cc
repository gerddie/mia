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

#include <mia/internal/autotest.hh>
#include <mia/3d/filter/morphological.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
namespace bfs=boost::filesystem;
using namespace morph_3dimage_filter;

void SetPluginpath()
{
	static bool run = false;
	if (!run) {
		CPathNameArray kernelsearchpath;
		kernelsearchpath.push_back(bfs::path("..")/bfs::path("shapes"));
		C3DShapePluginHandler::set_search_path(kernelsearchpath);
		run = true;
	}
}

struct SetPathFixture {
	SetPathFixture() {
		SetPluginpath();
	}
};

struct DilateTestFixture: SetPathFixture  {

	void do_test_bit(const C3DShape::Mask& mask, const C3DShape::Size& size,
			 P3DShape shape, bool hint) const
	{
		C3DBitImage *src = new C3DBitImage(C3DBounds(size));
		fill(src->begin(), src->end(), false);
		(*src)(size/2) = true;


		P3DImage src_wrap(src);

		C3DDilate dilate( shape, hint );
		P3DImage result = dilate.filter(*src_wrap);


		BOOST_CHECK_EQUAL(src_wrap->get_size(), result->get_size());

		C3DBitImage *presult = dynamic_cast<C3DBitImage *>(result.get());
		BOOST_REQUIRE(presult);

		BOOST_CHECK(equal(presult->begin(), presult->end(), mask.begin()));
	}

	template <typename T>
	void do_test_T(const C3DShape::Mask& /*mask*/, const C3DShape::Size& size, P3DShape shape) const
	{
		T3DImage<T> *src = new T3DImage<T>(C3DBounds(size));
		typename T3DImage<T>::iterator i_src = src->begin();

		for (size_t z = 0; z < src->get_size().z; ++z)
			for (size_t y = 0; y < src->get_size().y; ++y)
				for (size_t x = 0; x < src->get_size().x; ++x, ++i_src) {
					*i_src = x + 2 * y + 3 * z;
				}

		P3DImage src_wrap(src);

		C3DDilate dilate( shape, false );

		P3DImage result = dilate.filter(*src_wrap);


		BOOST_CHECK_EQUAL(src_wrap->get_size(), result->get_size());

		T3DImage<T> *presult = dynamic_cast< T3DImage<T> *> (result.get());
		BOOST_REQUIRE(presult);

		T3DImage<T> ref = T3DImage<T>(C3DBounds(size));

		typename T3DImage<T>::iterator ref_i = ref.begin();
		i_src = src->begin();

		for (size_t z = 0; z < src->get_size().z; ++z)
			for (size_t y = 0; y < src->get_size().y; ++y)
				for (size_t x = 0; x < src->get_size().x; ++x, ++ref_i, ++i_src) {
					*ref_i = *i_src;
					for (auto si = shape->begin(),  se = shape->end(); si != se; ++si) {

						C3DBounds h(x + si->x, y + si->y, z + si->z);
						if (h < src->get_size()) {
							T val = (*src)(h);
							if (*ref_i < val)
								*ref_i = val;
						}
					}
				}

		BOOST_CHECK(equal(ref.begin(), ref.end(), presult->begin()));
	}
};


BOOST_FIXTURE_TEST_CASE( test_dilate, DilateTestFixture)
{
	cvdebug() << "C3DDilateStackFilterFactory::do_test() \n";

	CParsedOptions options;

	const C3DShapePluginHandler::Instance& sh = C3DShapePluginHandler::instance();
	BOOST_REQUIRE(sh.size());

	for (C3DShapePluginHandler::Instance::const_iterator shb = sh.begin();
	     shb != sh.end(); ++shb) {
		P3DShape shape(shb->second->create(options, ""));

		cvdebug() << "created shape " << shb->first << "\n";

		C3DShape::Mask mask = shape->get_mask();
		C3DShape::Size size = shape->get_size();


#define TEST_TYPE(TYPE, TEXT) cvdebug() << TEXT"\n"; \
		do_test_T<TYPE>(mask, size, shape)

		TEST_TYPE(unsigned short, "unsigned short");
		TEST_TYPE(signed short, "signed short");
		TEST_TYPE(unsigned int, "unsigned int");
		TEST_TYPE(signed int, "signed int");
		TEST_TYPE(float, "float");
		TEST_TYPE(double, "double");
		TEST_TYPE(unsigned char, "unsigned char");
		TEST_TYPE(signed char, "signed char");

#undef TEST_TYPE
		// test a bit image

		do_test_bit(mask, size, shape, false);
		do_test_bit(mask, size, shape, true);
	}

}


struct ErodeTestFixture: SetPathFixture {


	void  do_test_bit(const C3DShape::Mask& mask, const C3DShape::Size& size,
						 P3DShape shape, bool hint) const
	{
		C3DBitImage *src = new C3DBitImage(C3DBounds(size));
		fill(src->begin(), src->end(), true);
		(*src)(size/2) = false;


		P3DImage src_wrap(src);

		C3DErode erode( shape, hint );
		P3DImage result = erode.filter(*src_wrap);

		BOOST_CHECK_EQUAL(src->get_size(), result->get_size());

		C3DBitImage *presult = dynamic_cast<C3DBitImage *>(result.get());
		BOOST_REQUIRE(presult);


		transform(mask.begin(), mask.end(), src->begin(), logical_not<bool>());
		BOOST_CHECK(equal(presult->begin(), presult->end(), src->begin()));
	}

	template <typename T>
	void do_test_T(const C3DShape::Mask& /*mask*/, const C3DShape::Size& size, P3DShape shape) const
	{
		T3DImage<T> *src = new T3DImage<T>(C3DBounds(size));
		typename T3DImage<T>::iterator i_src = src->begin();

		for (size_t z = 0; z < src->get_size().z; ++z)
			for (size_t y = 0; y < src->get_size().y; ++y)
				for (size_t x = 0; x < src->get_size().x; ++x, ++i_src) {
					*i_src = x + 2 * y + 3 * z;
				}

		P3DImage src_wrap(src);

		C3DErode erode( shape, false );

		P3DImage result = erode.filter(*src_wrap);

		BOOST_CHECK_EQUAL(src->get_size(), result->get_size());

		T3DImage<T> *presult = dynamic_cast< T3DImage<T> *> (result.get());
		BOOST_REQUIRE(presult);

		T3DImage<T> ref = T3DImage<T>(C3DBounds(size));

		typename T3DImage<T>::iterator ref_i = ref.begin();
		i_src = src->begin();

		for (size_t z = 0; z < src->get_size().z; ++z)
			for (size_t y = 0; y < src->get_size().y; ++y)
				for (size_t x = 0; x < src->get_size().x; ++x, ++ref_i, ++i_src) {
					*ref_i = *i_src;
					for (auto si = shape->begin(),
						     se = shape->end(); si != se; ++si) {

						C3DBounds h(x + si->x, y + si->y, z + si->z);
						if (h < src->get_size()) {
							T val = (*src)(h);
							if (*ref_i > val)
								*ref_i = val;
						}
					}
				}

		BOOST_CHECK(equal(ref.begin(), ref.end(), presult->begin()));
	}
};


BOOST_FIXTURE_TEST_CASE( test_erode, ErodeTestFixture)
{
	cvdebug() << "C3DErodeStackFilterFactory::do_test() \n";

	CParsedOptions options;

	const C3DShapePluginHandler::Instance& sh = C3DShapePluginHandler::instance();
	BOOST_REQUIRE(sh.size());

	for (C3DShapePluginHandler::Instance::const_iterator shb = sh.begin();
	     shb != sh.end(); ++shb) {
		P3DShape shape(shb->second->create(options, ""));

		cvdebug() << "created shape " << shb->first << "\n";

		C3DShape::Mask mask = shape->get_mask();
		C3DShape::Size size = shape->get_size();


#define TEST_TYPE(TYPE, TEXT) cvdebug() << TEXT"\n";		\
		do_test_T<TYPE>(mask, size, shape)

		TEST_TYPE(unsigned short, "unsigned short");
		TEST_TYPE(signed short, "signed short");
		TEST_TYPE(unsigned int, "unsigned int");
		TEST_TYPE(signed int, "signed int");
		TEST_TYPE(float, "float");
		TEST_TYPE(double, "double");
		TEST_TYPE(unsigned char, "unsigned char");
		TEST_TYPE(signed char, "signed char");

#undef	TEST_TYPE
		// test a bit image
		do_test_bit(mask, size, shape, false);
		do_test_bit(mask, size, shape, true);
	}

}
