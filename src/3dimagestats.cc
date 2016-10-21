/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <sstream>
#include <mia/core.hh>
#include <mia/3d.hh>

NS_MIA_USE;
using namespace std;

const SProgramDescription g_description = {
	{pdi_group, "Analysis, filtering, combining, and segmentation of 3D images"}, 

	{pdi_short, "Evaluate some image statistics."},
	
	{pdi_description, "This program is used to evaluate certain image statistics "
	 "and print them out on the console."}, 

	{pdi_example_descr, "Evaluate the statistics of image input.v and ignore all voxels with an intensity "
	 "below 5."}, 

	{pdi_example_code, "-i input.v -t 5"}
};



class CHistAccumulator : public TFilter<bool> {
public:
	CHistAccumulator(float min, float max, size_t bins, float thresh):
		m_histo(THistogramFeeder<float>(min, max, bins)),
		m_thresh(thresh)
	{
	}

	template <typename T>
	bool operator () (const T3DImage<T>& image) {
		for (typename T3DImage<T>::const_iterator i = image.begin();
		     i != image.end(); ++i)
			if (*i > m_thresh)
				m_histo.push(*i);
		return true;
	}

	void print_stats()const
		{
			cout   <<  m_histo.average() << " " << m_histo.deviation()  << '\n';
		}

	void print_stats(double thresh_high)const
		{
			THistogram<THistogramFeeder<float > > tmp(m_histo, thresh_high);
			cout   <<  tmp.average() << " " << tmp.deviation()  << '\n';
		}
private:
	THistogram<THistogramFeeder<float > > m_histo;
	float m_thresh;
};



int do_main( int argc, char *argv[] )
{

	string in_filename;
	float thresh = 10.0;
	const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();


	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( thresh, "thresh", 't', "intensity thresh to ignore"));
	options.set_stdout_is_result();
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


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

#include <mia/internal/main.hh>
MIA_MAIN(do_main)
