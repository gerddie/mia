/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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
#include <mia/core/meanvar.hh>
#include <mia/2d.hh>
#include <mia/internal/main.hh>

NS_MIA_USE;
using namespace std;

const SProgramDescription g_general_help = {
	{pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"}, 
	
	{pdi_short, "Evaluate some statistics of a 2D image."}, 
	
	{pdi_description, "This progranm is used to evaluate some statistics of an image. " 
	 "Output is Mean, Variation, Median, and Median Average Distance of the intensity values. "
	 "The program allows one to set a lower threshold and to cut off a percentage of the high intensity pixels"}, 
	
	{pdi_example_descr, "Evaluate the statistics of image input.png. Don't include the upper 1% "
	 "of intensities in the statistics. The result is written to stdout"},

        {pdi_example_code,	"-i input.png -g 1"}
}; 


class CHistAccumulator : public TFilter<bool> {
public:
	CHistAccumulator(float min, float max, size_t bins, float thresh):
		m_histo(THistogramFeeder<float>(min, max, bins)),
		m_thresh(thresh), 
		m_max(0)
	{
	}

	template <typename T>
	bool operator () (const T2DImage<T>& image) {
		for (auto i = image.begin(); i != image.end(); ++i) {
			if (*i > m_thresh)
				m_histo.push(*i);
			if (*i > m_max) 
				m_max = *i; 
		}
		return true;
	}

	void print_stats()const
	{
		print_stats(m_histo);
	}

	void print_stats(double thresh_high)const
	{
		THistogram<THistogramFeeder<float > > tmp(m_histo, thresh_high);
		print_stats(tmp);
	}
private:
	void print_stats(const THistogram<THistogramFeeder<float > >& tmp)const
	{
		cout   <<  tmp.average() << " " << tmp.deviation()  <<  " "
		       << tmp.median() << " " << tmp.MAD() 
		       << " " << m_max 
		       << '\n';
	}

	THistogram<THistogramFeeder<float > > m_histo;
	float m_thresh;
	float m_max; 
};

struct FFullAccumulater: public TFilter<bool> {
	FFullAccumulater(float thresh):m_thresh(thresh){}; 

	template <typename T>
	bool operator () (const T2DImage<T>& image) {
		for (auto i = image.begin(); i != image.end(); ++i) {
			if (*i > m_thresh)
				m_acc.push_back(*i);
		}
		return true;
	}
	void print_stats()
	{
		sort( m_acc.begin(), m_acc.end()); 
		auto mv = mean_var(m_acc.begin(), m_acc.end()); 
		auto minmax = minmax_element(m_acc.begin(), m_acc.end()); 
		cout << mv.first << " " 
		     << mv.second << " " 
		     << m_acc[m_acc.size() / 2] << " " 
		     << *minmax.first << " " 
		     << *minmax.second << "\n"; 
	}
	
private: 
	float m_thresh; 
	vector<float> m_acc; 
}; 


int do_main( int argc, char *argv[] )
{

	string in_filename;
	float thresh = 10.0;
	float high_thresh = 0.05;
	bool use_histogram = false; 
	
	const auto& imageio = C2DImageIOPluginHandler::instance();
	
	
	CCmdOptionList options(g_general_help);
	options.add(make_opt( in_filename, "in-file", 'i', "input image to be analyzed", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( thresh, "thresh", 't', "intensity thresh to ignore"));
	options.add(make_opt( high_thresh, "high-thresh", 'g', "upper histogram percentage to ignore"));
	options.add(make_opt( use_histogram, "use-histogram", 0, "Use a histogram to accumulate statistics"));

	options.set_stdout_is_result();
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	
	C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(in_filename);
	if (in_image_list.get() && in_image_list->size()) {
		if (use_histogram) {
			CHistAccumulator histo(0, 4096, 1024, thresh);
			for (auto i = in_image_list->begin(); i != in_image_list->end(); ++i)
				accumulate(histo, **i);
			histo.print_stats(high_thresh);
		}else{
			vector<float> result;
			FFullAccumulater acc(thresh); 
			for (auto i = in_image_list->begin(); i != in_image_list->end(); ++i)
				accumulate(acc, **i);
			acc.print_stats();
			
		}
	}else
		throw runtime_error(string("No errors found in ") + in_filename);
	
	return EXIT_SUCCESS;
	
}

MIA_MAIN(do_main); 
