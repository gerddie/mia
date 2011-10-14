/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

/*
  LatexBeginProgramDescription{2D image processing}

  \subsection{mia-2dlerp}
  \label{mia-2dlerp}
  
  \begin{description}
  \item [Description:] This program linearly combines two images $I_1$ and $I_2$ by using time index positions 
  to estimate the weight. 

  The program is called like 
  \
  \begin{lstlisting}
mia-2dlerp -1 <input1> -2 <input2> -o <output> -p n1,p,n2 ... 
  \end{lstlisting}
  with $n_1 \le p \le n_2$ and $n_1 < n_2$. 
  The voxels of the interpolated image R will have values according to 
  \begin{equation}
  R(\vx) := \frac{n_2 - p}{n_2 - n_1} I_1(\vx) + \frac{p - n_1}{n_2 - n_1} I_2(\vx)
  \end{equation}

  \item [Options:] $\:$

  \optiontable{
  \cmdopt{first}{1}{string}{First input image}
  \cmdopt{second}{2}{string}{Second input image}
  \optoutfile
  \cmdopt{positions}{p}{vector<int>}{Image positions in the series: first,interpolant,second} 
  }

  \item [Example:]Interpolate by using the images a.v and b.v for position 7 
  assuming that they are in the series position 5 and 10 and write the result to ab.v 
   \
  \begin{lstlisting}
mia-2dlerp -1 a.v -2 b.v -o ab.v -p 5,7,10
  \end{lstlisting}
  \end{description}
  
  LatexEnd
*/


#include <climits>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/core.hh>
#include <mia/internal/main.hh>
#include <mia/2d.hh>
#include <mia/2d/2dfilter.hh>
#include <sstream>
#include <iomanip>

NS_MIA_USE
using namespace boost;
using namespace std;

const SProgramDescrption g_description = {
	"3D image processing", 
	"merge two images by linear combination.", 
	"Combine image inputA.v and inputB.v by using position coordinates 4, 7, and 9 and write the result to output.v", 
	"-1 inputA.v -2 inputB.v -p 4,7,9 -o output.v"
}; 

struct FAddWeighted: public TFilter<P2DImage> {
	FAddWeighted(float w):
		m_w(w)
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
			
		float w2 = 1.0 - m_w;
		
		while ( r != e ) {
			*r = (T)(w2 * *ia + m_w * (float)*ib);
			cvdebug() << w2 << " * " <<*ia << " + " << m_w <<" * " <<  (float)*ib  << "->" << *r << "\n";
			++r;
			++ia;
			++ib;
		}
			
		return P2DImage(result);
	}
	
private:
	float m_w;
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
		m_f(f)
		{
		}
	
	P2DImage operator () (const P2DImage& a, const P2DImage& b) const {
		return ::mia::filter(m_f, *a, *b);
	}
private:
	const F& m_f;
};

int do_main(int argc, char **argv)
{

	CCmdOptionList options(g_description);
	string src1_filename;
	string src2_filename;
	string out_filename;
	bool self_test = false;

	vector<float> positions;

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	options.add(make_opt( src1_filename, "first", '1', "first input image ", CCmdOption::required));
	options.add(make_opt( src2_filename, "second", '2', "second input image ", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output vector field", CCmdOption::required));
	options.add(make_opt( positions, "positions", 'p', 
				    "image series positions (first, target, second)", CCmdOption::required));
	options.add(make_opt( self_test, "self-test", 0, "run a self test of the tool"));
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	if (self_test) {
		return ::boost::unit_test::unit_test_main( &init_unit_test_suite, argc, (char **)argv );
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
		cvwarn() << "Number of images differ, only combining first " 
			 << (source1->size() < source2->size() ? source1->size() : source2->size()) << "images\n";

	if (source1->size() <= source2->size())
		transform( source1->begin(), source1->end(), source2->begin(), 
			   source1->begin(), FFilter<FAddWeighted>(FAddWeighted(w)));
	else {
		transform( source2->begin(), source2->end(), source1->begin(), 
			   source2->begin(), FFilter<FAddWeighted>(FAddWeighted(w)));
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
