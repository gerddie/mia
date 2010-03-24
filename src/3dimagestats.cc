/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

// $Id: 3dimagefilter.cc,v 1.12 2006-07-12 13:44:23 wollny Exp $

/*! \brief mia-3dimagefilter

\sa mia-3dimagefilter.cc

\file mask.cc
\author G. Wollny, wollny eva.mpg.de, 2005
*/

#include <sstream>
#include <mia/core.hh>
#include <mia/3d.hh>

NS_MIA_USE;
using namespace std;

class CHistAccumulator : public TFilter<bool> {
public:
	CHistAccumulator(float min, float max, size_t bins, float thresh):
		_M_histo(CHistogramFeeder<float>(min, max, bins)),
		_M_thresh(thresh)
	{
	}

	template <typename T>
	bool operator () (const T3DImage<T>& image) {
		for (typename T3DImage<T>::const_iterator i = image.begin();
		     i != image.end(); ++i)
			if (*i > _M_thresh)
				_M_histo.push(*i);
		return true;
	}

	void print_stats()const
	{
		cout   <<  _M_histo.average() << " " << _M_histo.deviation()  << '\n';
	}

	void print_stats(double thresh_high)const
	{
		CHistogram<CHistogramFeeder<float > > tmp(_M_histo, thresh_high);
		cout   <<  tmp.average() << " " << tmp.deviation()  << '\n';
	}
private:
	CHistogram<CHistogramFeeder<float > > _M_histo;
	float _M_thresh;
};



int main( int argc, const char *argv[] )
{

	string in_filename;
	float thresh = 10.0;
	try {

		const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();


		CCmdOptionList options;
		options.push_back(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", "input", true));
		options.push_back(make_opt( thresh, "thresh", 't', "intensity thresh to ignore", "thresh", false));

		options.parse(argc, argv);

		//CHistory::instance().append(argv[0], "unknown", options);

		// read image
		C3DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(in_filename);

		if (in_image_list.get() && in_image_list->size()) {
			CHistAccumulator histo(0, 4096, 1024, thresh);
			for (C3DImageIOPluginHandler::Instance::Data::iterator i = in_image_list->begin();
			     i != in_image_list->end(); ++i)
				accumulate(histo, **i);
			histo.print_stats(0.05);
		}else
			throw runtime_error(string("No errors found in ") + in_filename);

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
