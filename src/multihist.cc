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
#include <ostream>
#include <fstream>
#include <mia/core.hh>
#include <mia/2d.hh>

NS_MIA_USE;
using namespace std;

const SProgramDescription g_description = {
	
	{pdi_group, 	"Processing of series of 2D images in a 3D fashion (out-of-core)"}, 
	{pdi_short, "histogram of a series of 2D images"}, 
	
	
	{pdi_description, "This program evaluates the histogram over a series of 2D images"}, 
	
	
	{pdi_example_descr,"Evaluate the histogram over all consecutively numbered images that follow the "
	 "numbering pattern inputXXXX.exr and store the output histogram in histo.txt. "
	 "Use the range [0,256] and 64 bins."}, 
	
	
	{pdi_example_code, "-i input0000.exr -o histo.txt --min 0 --max 256 --bins 64"}
}; 

class CAutoHistogramAccumulator : public TFilter<bool> {
public: 
	template <typename T>
	bool operator () (const T2DImage<T>& image) {
		for(auto i = image.begin(); i != image.end(); ++i) {
			auto slot = m_hist.find(*i);
			if (slot == m_hist.end()) {
				m_hist[*i] = 1; 
			}else{
				++m_hist[*i];
			}
		}
		return true;
	}

	bool save(const string& fname)const
	{
		ofstream file(fname.c_str());
		for (auto ih :m_hist) {
			file << ih.first << " " << ih.second << "\n";
		}
		return file.good();
	}

	int propose_threshold()const {
		assert(!m_hist.empty());
		
		float min_val = m_hist.begin()->first; 
		float max_val = m_hist.rbegin()->first; 

		float down_start_search = (max_val - min_val) / 10 + min_val; 
		float up_end_search = (max_val - min_val) / 6 + min_val; 

		auto i = m_hist.begin(); 
		
		while (i->first < down_start_search) 
			++i; 
		
		auto val = i->second;
		
		auto pos = i; 
		while (i !=  m_hist.begin()) {
			if (val > i->second) {
				val = i->second; 
				pos = i; 
			}
			--i; 
		}
		
		while (i->first < up_end_search) {
			if (val > i->second) {
				val = i->second; 
				pos = i; 
			}
			++i; 
		}
		
		return pos->first; 
	}
	
private: 
	map<float, size_t> m_hist; 
}; 

class CHistAccumulator : public TFilter<bool> {

public:
	CHistAccumulator(float min, float max, size_t bins):
		m_histo(THistogramFeeder<float>(min, max, bins)), 
		m_last(0)
	{
	}

	template <typename T>
	bool operator () (const T2DImage<T>& image) {
		for(auto i = image.begin(); i != image.end(); ++i)
			m_histo.push(*i);
		return true;
	}

	void resize() {
		for (size_t i = 0; i < 	m_histo.size(); ++i) {
			if (m_histo[i] > 0 )
				m_last = i;
		}
	}
			
	bool save(const string& fname)const
	{
		ofstream file(fname.c_str());
		for (size_t i = 0; i < 	m_last; ++i) {
			const THistogram<THistogramFeeder<float > >::value_type v = m_histo.at(i);
			file << v.first << " " << v.second << "\n";
		}
		return file.good();
	}

	int propose_threshold()const {
		int pos = m_last / 10; 
		int max_pos = m_last / 6; 
		double val = m_histo[pos];
		
		for (int i = pos; i > 10; --i)
			if (m_histo[i] < val) {
				pos = i; 
				val = m_histo[i]; 
			}
		
		for (int i = pos; i < max_pos; ++i)
			if (m_histo[i] < val) {
				pos = i; 
				val = m_histo[i]; 
			}
		return pos; 
	}
private:
	THistogram<THistogramFeeder<float > > m_histo;
	size_t m_last; 
};



int do_main( int argc, char *argv[] )
{
	float hmin = 0;
	float hmax = 65535;
	size_t bins = 65536;

	string out_filename;
	string in_filename;
		
	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( out_filename, "out-histogram", 'o', "output file name", 
			      CCmdOptionFlags::required_output));
	options.add(make_opt( hmin, "min", 0, "minimum of histogram range"));
	options.add(make_opt( hmax, "max", 0, "maximum of histogram range"));
	options.add(make_opt( bins, "bins", 0, "number of histogram bins, set to zero to create a bin for "
			      "each intensity value available in the input data. In this case the histogram "
			      "range is also evaluated automatically"));
		
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 




	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);
		
	if (start_filenum >= end_filenum)
		throw invalid_argument(string("no files match pattern ") + src_basename);

	if (bins > 0) {
		CHistAccumulator histo_accu(hmin, hmax, bins);
		for (size_t i = start_filenum; i < end_filenum; ++i) {
			string src_name = create_filename(src_basename.c_str(), i);
			C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(src_name);
			cvmsg() << "Read:" << src_name << "\r";
			if (in_image_list.get() && in_image_list->size()) {
				for (auto k = in_image_list->begin(); k != in_image_list->end(); ++k)
					accumulate(histo_accu, **k);
			}
		}
		cvmsg() << "\n";
		
		histo_accu.resize(); 
		
		if (!histo_accu.save(out_filename))
			throw runtime_error(string("Error writing output file:") + out_filename);
		cout << histo_accu.propose_threshold(); 
		
	}else{
		CAutoHistogramAccumulator histo; 
		for (size_t i = start_filenum; i < end_filenum; ++i) {
			string src_name = create_filename(src_basename.c_str(), i);
			C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(src_name);
			cvmsg() << "Read:" << src_name << "\r";
			if (in_image_list.get() && in_image_list->size()) {
				for (auto k = in_image_list->begin(); k != in_image_list->end(); ++k)
					accumulate(histo, **k);
			}
		}
		cvmsg() << "\n";
		
		
		if (!histo.save(out_filename))
			throw runtime_error(string("Error writing output file:") + out_filename);
		
		cout << histo.propose_threshold(); 
	}
	
	return EXIT_SUCCESS;

}
#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
