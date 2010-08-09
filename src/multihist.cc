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

#include <sstream>
#include <ostream>
#include <fstream>
#include <mia/core.hh>
#include <mia/2d.hh>

/*! \brief Evalaute the histogram across multiple 2D images and write it to an output file
This Program evaluates the intensity histogram across a series of images.
The intensities are clamped to thegiven input range and the values are distributed across
a given number of bins.


\autor Gert Wollny <gw.fossdev at gmail.com>
*/

const char *g_description = 
	"This program evaluates the histogram over a series of\n"
	"2D images\n"
	"Basic usage:\n"
	"  mia-multihist [options] <image-files>\n";

NS_MIA_USE;
using namespace std;

class CHistAccumulator : public TFilter<bool> {
public:
	CHistAccumulator(float min, float max, size_t bins):
		_M_histo(CHistogramFeeder<float>(min, max, bins))
	{
	}

	template <typename T>
	bool operator () (const T2DImage<T>& image) {
		for( typename T2DImage<T>::const_iterator i = image.begin();
		     i != image.end(); ++i)
			_M_histo.push(*i);
		return true;
	}
	bool save(const string& fname)const
	{
		size_t last_k = 0;
		for (size_t i = 0; i < 	_M_histo.size(); ++i) {
			if (_M_histo[i] > 0 )
				last_k = i;
		}

		ofstream file(fname.c_str());
		for (size_t i = 0; i < 	last_k; ++i) {
			const CHistogram<CHistogramFeeder<float > >::value_type v = _M_histo.at(i);
			file << v.first << " " << v.second << "\n";
		}
		return file.good();
	}
private:
	CHistogram<CHistogramFeeder<float > > _M_histo;
};



int main( int argc, const char *argv[] )
{
	try {

		float hmin = 0;
		float hmax = 65536;
		size_t bins = 65536;

		string out_filename;

		CCmdOptionList options(g_description);
		options.push_back(make_opt( out_filename, "out", 'o', "output file name", "out", true));
		options.push_back(make_opt( hmin, "min", 0, "minimum of histogram range", "min", false));
		options.push_back(make_opt( hmax, "max", 0, "maximum of histogram range", "max", false));
		options.push_back(make_opt( bins, "bins", 0, "number of histogram bins", "bins", false));
		options.parse(argc, argv);


		vector<const char *> fn = options.get_remaining();
		if ( fn.empty() )
			throw invalid_argument("no files given\n");

		const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

		CHistAccumulator histo_accu(hmin, hmax, bins);
		for (vector<const char *>::const_iterator i = fn.begin(); i != fn.end(); ++i) {
			C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(*i);
			cvmsg() << "Read:" << *i << "\r";
			if (in_image_list.get() && in_image_list->size()) {
				for (C2DImageIOPluginHandler::Instance::Data::iterator i = in_image_list->begin();
				     i != in_image_list->end(); ++i)
					accumulate(histo_accu, **i);
			}
		}
		cvmsg() << "\n";

		if (!histo_accu.save(out_filename))
			throw runtime_error(string("Error writing output file:") + out_filename);

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
