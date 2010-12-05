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

// $Id: 2dimagefilter.cc,v 1.12 2006-07-12 13:44:23 wollny Exp $

/*! \brief mia-2dimagefilter

\sa mia-2dimagefilter.cc

\file mask.cc
\author G. Wollny, wollny eva.mpg.de, 2005
*/

#include <sstream>
#include <functional>
#include <cmath>
#include <mia/core.hh>
#include <mia/core/errormacro.hh>
#include <mia/2d.hh>

#include <boost/lambda/lambda.hpp>


NS_MIA_USE;
using namespace std;
using namespace boost::lambda;

class CVarAccumulator : public TFilter<bool> {
public:
	CVarAccumulator():
		_M_n(0)
	{
	}

	template <typename T>
	bool operator () (const T2DImage<T>& image) {
		if (!_M_n) {
			_M_sum = C2DDImage(image.get_size(), image);
			_M_sum2 = C2DDImage(image.get_size());
		}else if (image.get_size() != _M_sum.get_size()) {
			THROW(invalid_argument, "input images differ in size");
		}

		transform(image.begin(), image.end(), _M_sum.begin(), _M_sum.begin(), _1 + _2);
		transform(image.begin(), image.end(), _M_sum2.begin(), _M_sum2.begin(), _1  * _1 + _2);
		++_M_n;
		return true;
	}

	P2DImage result()const
	{
		C2DFImage *image = new C2DFImage( _M_sum.get_size());
		P2DImage result(image);
		transform(_M_sum.begin(), _M_sum.end(), _M_sum2.begin(),
			  image->begin(), (_2 - _1 * _1 / _M_n) / (_M_n - 1));
		transform(image->begin(), image->end(), image->begin(),  ptr_fun(::sqrt));
		return result;

	}
private:
	C2DDImage _M_sum;
	C2DDImage _M_sum2;
	size_t _M_n;
};


const char *g_description = 
	"This program evaluates the intensity variation of a set of image sgiven on the command line"; 


int main( int argc, const char *argv[] )
{

	string out_filename;
	string out_type("vista");

	try {

		const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();


		CCmdOptionList options(g_description);
		options.push_back(make_opt( out_filename, "out-file", 'o', "output image ", CCmdOption::required));
		options.push_back(make_opt( out_type, imageio.get_set(), "type", 't',"output file type"));

		options.parse(argc, argv);

		if (options.get_remaining().empty())
			throw runtime_error("no input images given ...");

		if ( out_filename.empty() )
			throw runtime_error("'--out-base' ('o') option required");


		vector<const char *> input_images = options.get_remaining();

		CVarAccumulator ic;

		for (vector<const char *>::const_iterator  i = input_images.begin(); i != input_images.end(); ++i) {

			cvmsg() << "Load " << *i << "\r";
			C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(*i);

			if (in_image_list.get() && in_image_list->size()) {
				accumulate(ic, **in_image_list->begin());
			}
		}
		cvmsg() << "\n";

		C2DImageVector result;
		result.push_back(ic.result());

		if (imageio.save(out_type, out_filename, result))
			return EXIT_SUCCESS;
		else
			cerr << argv[0] << " fatal: unable to output image to " <<  out_filename << endl;

		return EXIT_SUCCESS;

	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}
