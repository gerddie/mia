/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <climits>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/2d/2dfilter.hh>
#include <sstream>
#include <iomanip>

NS_MIA_USE
using namespace boost;
using namespace std;

const char *g_description = 
	"This program is used to evaluate an image that is the linear combination of "
	"two gray scale images"; 


struct FAddWeighted: public TFilter<P2DImage> {
	FAddWeighted(float w):
		_M_w(w)
		{
		}
	
	template <typename T, typename S>
	P2DImage operator() (const T2DImage<T>& a, const T2DImage<S>& b) const	{
		if (a.get_size() != b.get_size()) {
			throw invalid_argument("input images cann not be combined because they differ in size");
		}
		T2DImage<T> *result = new T2DImage<T>(a.get_size());
		typename T2DImage<T>::iterator r = result->begin();
		typename T2DImage<T>::iterator e = result->end();
		
		typename T2DImage<T>::const_iterator ia = a.begin();
		typename T2DImage<S>::const_iterator ib = b.begin();
			
		float w2 = 1.0 - _M_w;
		
		while ( r != e ) {
			*r = (T)(w2 * *ia + _M_w * (float)*ib);
			cvdebug() << w2 << " * " <<*ia << " + " << _M_w <<" * " <<  (float)*ib  << "->" << *r << "\n";
			++r;
			++ia;
			++ib;
		}
			
		return P2DImage(result);
	}
	
private:
	float _M_w;
};

static void run_self_test()
{
	const C2DBounds size(1,2);
	
	C2DFImage *A = new C2DFImage(size);
	C2DUBImage *B = new C2DUBImage(size);
	
	P2DImage pA(A);
	P2DImage pB(B);
	
	(*A)(0,0) = 1.0; (*A)(0,1) = 2.0;
	(*B)(0,0) = 3.0; (*B)(0,1) = 4.0;
	
	FAddWeighted aw(0.25);
	
	P2DImage R = mia::filter(aw, *pA, *pB);
	
	
	const C2DFImage* result = dynamic_cast<const C2DFImage*>(R.get());
	BOOST_REQUIRE(result);
	
	BOOST_CHECK_CLOSE( (*result)(0,0), 1.5f, 0.1f);
	BOOST_CHECK_CLOSE( (*result)(0,1), 2.5f, 0.1f);
	
}

static bool init_unit_test_suite( )
{
	::boost::unit_test::framework::master_test_suite().add( BOOST_TEST_CASE( &run_self_test));
	return true;
}

template <typename F>
struct FFilter {
	FFilter(const F& f):
		_M_f(f)
		{
		}
	
	P2DImage operator () (const P2DImage& a, const P2DImage& b) const {
		return ::mia::filter(_M_f, *a, *b);
	}
private:
	const F& _M_f;
};

int do_main(int argc, char **args)
{

	CCmdOptionList options(g_description);
	string src1_filename;
	string src2_filename;
	string out_filename;
	string out_type;
	bool self_test = false;

	vector<float> positions;

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	options.push_back(make_opt( src1_filename, "first", '1', "first input image ", "first", true));
	options.push_back(make_opt( src2_filename, "second", '2', "second input image ", "second", true));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output vector field", "output", true));
	options.push_back(make_opt( positions, "positions", 'p', "image series positions (first, target, second)", "pos", true));
	options.push_back(make_opt( out_type, imageio.get_set(), "type", 't',"output file type (if not given deduct from output file name)",
				    "image-type"));

	options.push_back(make_opt( self_test, "self-test", 0, "run a self test of the tool", "selftest", false));


	options.parse(argc, args);

	if (!options.get_remaining().empty()) {
		cerr << "Unknown options found\n";
		return EXIT_FAILURE;
	}

	if (self_test) {
		return ::boost::unit_test::unit_test_main( &init_unit_test_suite, argc, args );
	}

	if (positions.size() != 3) {
		stringstream msg;
		msg << "positions must be 3 values: first, target, second, got " << positions.size() << "images";
		throw invalid_argument(msg.str());
	}

	C2DImageIOPlugin::PData source1 = imageio.load(src1_filename);
	C2DImageIOPlugin::PData source2 = imageio.load(src2_filename);


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
		cvwarn() << "Number of images differ, only combining first " << (source1->size() < source2->size() ? source1->size() : source2->size()) << "images\n";

	if (source1->size() <= source2->size())
		transform( source1->begin(), source1->end(), source2->begin(), source1->begin(), FFilter<FAddWeighted>(FAddWeighted(w)));
	else {
		transform( source2->begin(), source2->end(), source1->begin(), source2->begin(), FFilter<FAddWeighted>(FAddWeighted(w)));
		source1 = source2;
	}

	// save the result;
	if ( !imageio.save(out_type, out_filename, *source1) ){
		string not_save = ("unable to save result to ") + out_filename;
		throw runtime_error(not_save);
	};

	return EXIT_SUCCESS;
}

// for readablility the real main function encapsulates the do_main in a try-catch block
int main(int argc, char **args)
{
	try {
		return do_main(argc, args);
	}
	catch (invalid_argument& err) {
		cerr << "invalid argument: " << err.what() << "\n";
	}
	catch (runtime_error& err) {
		cerr << "runtime error: " << err.what() << "\n";
	}
	catch (std::exception& err) {
		cerr << "exception: " << err.what() << "\n";
	}
	catch (...) {
		cerr << "unknown exception\n";
	}
	return EXIT_FAILURE;
}
