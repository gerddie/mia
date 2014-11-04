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

#include <climits>

#define BOOST_TEST_MAIN
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/core.hh>
#include <mia/internal/main.hh>
#include <mia/3d.hh>
#include <mia/3d/filter.hh>
#include <sstream>
#include <iomanip>

NS_MIA_USE
using namespace boost;
using namespace std;

const SProgramDescription g_description = {
        {pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 
	{pdi_short, "Linearly combine two 3D images."}, 
	{pdi_description, "merge two images by linear combination."}, 
	{pdi_example_descr, "Combine image inputA.v and inputB.v by using position coordinates "
	 "4, 7, and 9. This means the output pixel values will be evaluated according to\n\n"
	 "    (9-7)/(9-4) * A + (7-4)/(9-4) * B  \n\n"
	 "The result image will be written to output.v"}, 
	{pdi_example_code, "-1 inputA.v -2 inputB.v -p 4,7,9 -o output.v"}
}; 

struct FAddWeighted: public TFilter<P3DImage> {
	FAddWeighted(float w):
		m_w(w)
	{
	}

	template <typename T, typename S>
	P3DImage operator() (const T3DImage<T>& a, const T3DImage<S>& b) const
	{
		if (a.get_size() != b.get_size()) {
			throw invalid_argument("input images cann not be combined because they differ in size");
		}

		if (a.get_voxel_size() != b.get_voxel_size()) {
			throw invalid_argument("input images cann not be combined because they have differ pixel sizes");
		}


		C3DFImage *result = new C3DFImage(a.get_size(), a);
		C3DFImage::iterator r = result->begin();
		C3DFImage::iterator e = result->end();

		typename T3DImage<T>::const_iterator ia = a.begin();
		typename T3DImage<S>::const_iterator ib = b.begin();



		float w2 = 1.0 - m_w;

		while ( r != e ) {
			*r = w2 * *ia + m_w * (float)*ib;
			cvdebug() << w2 << " * " <<*ia << " + " << m_w <<" * " <<  (float)*ib  << "->" << *r << "\n";
			++r;
			++ia;
			++ib;
		}

		return P3DImage(result);
	}

private:
	float m_w;
};

BOOST_AUTO_TEST_CASE( run_self_test )
{
	const C3DBounds size(1,2,1);

	C3DFImage *A = new C3DFImage(size);
	C3DUBImage *B = new C3DUBImage(size);

	P3DImage pA(A);
	P3DImage pB(B);

	(*A)(0,0,0) = 1.0; (*A)(0,1,0) = 2.0;
	(*B)(0,0,0) = 3.0; (*B)(0,1,0) = 4.0;

	FAddWeighted aw(0.25);

	P3DImage R = mia::filter(aw, *pA, *pB);


	const C3DFImage* result = dynamic_cast<const C3DFImage*>(R.get());
	BOOST_REQUIRE(result);

	BOOST_CHECK_CLOSE( (*result)(0,0,0), 1.5f, 0.1f);
	BOOST_CHECK_CLOSE( (*result)(0,1,0), 2.5f, 0.1f);

}

SELFTEST_CALLBACK(CSelftest);

template <typename F>
struct FFilter {
	FFilter(const F& f):
		m_f(f)
	{
	}

	P3DImage operator () (const P3DImage& a, const P3DImage& b) const {
		return ::mia::filter(m_f, *a, *b);
	}
private:
	const F& m_f;
};

// set op the command line parameters and run the registration
int do_main(int argc, char **argv)
{

	CCmdOptionList options(g_description);
	string src1_filename;
	string src2_filename;
	string out_filename;
	int self_test_result = 0;

	vector<float> positions;

	const auto& imageio = C3DImageIOPluginHandler::instance();

	options.add(make_opt( src1_filename, "first", '1', "first input image ", CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( src2_filename, "second", '2', "second input image ", CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( out_filename, "out-file", 'o', "linearly interpolated image", 
			      CCmdOptionFlags::required_output, &imageio));
	options.add(make_opt( positions, "positions", 'p', 
				    "image series positions (first, target, second)", CCmdOptionFlags::required));
	
	options.add_selftest(self_test_result, new CSelftest(argc, argv));


	switch (options.parse(argc, argv, "boost-test-options")) {
	case CCmdOptionList::hr_no:
		break; 
	case CCmdOptionList::hr_selftest: 
		return self_test_result; 
	default: 
		return EXIT_SUCCESS; 
	}

	if (positions.size() != 3) {
		stringstream msg;
		msg << "positions must be 3 values: first, target, second, got " << positions.size() << "images";
		throw invalid_argument(msg.str());
	}

	C3DImageIOPlugin::PData source1 = imageio.load(src1_filename);
	C3DImageIOPlugin::PData source2 = imageio.load(src2_filename);


	if (!source1 || source1->size() < 1) {
		cerr << "no image found in " << src1_filename << "\n";
		return EXIT_FAILURE;
	}

	if (!source2 || source2->size() < 1) {
		cerr << "no image found in " << src2_filename << "\n";
		return EXIT_FAILURE;
	}

	float w = (positions[1] - positions[0]) / (positions[2] - positions[0]);



	if (source1->size() != source2->size())
		cvwarn() << "Number of images differ, only combining first " << 
			(source1->size() < source2->size() ? source1->size() : source2->size()) << "images\n";

	if (source1->size() <= source2->size())
		transform( source1->begin(), source1->end(), source2->begin(), source1->begin(), FFilter<FAddWeighted>(FAddWeighted(w)));
	else {
		transform( source2->begin(), source2->end(), source1->begin(), source2->begin(), FFilter<FAddWeighted>(FAddWeighted(w)));
		source1 = source2;
	}

	// save the result;
	if ( !imageio.save(out_filename, *source1) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
	};

	return EXIT_SUCCESS;
}

MIA_MAIN(do_main); 
