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


#include <sstream>
#include <ostream>
#include <fstream>
#include <mia/core.hh>
#include <mia/2d.hh>


/*
  LatexBeginProgramDescription{2D image stack processing}
  
  \subsection{mia-multihist}
  \label{mia-multihist}

  \begin{description}
  \item [Description:] This program is used to evaluate the histogram of a series of 
   2D gray scale images. 
   If no additional parameters are given, the histogram will have 65536 bins and cover an 
   intensity range of [0,65535]. 
   No interpolation is used when the histogram is filled. 
   When the histogram is saved, the tail of zero bins is eliminated. 
  The program is called like 
  \
  \begin{lstlisting}
mia-multihisto -i <input images> -o <output file> 
  \end{lstlisting}


  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \cmdopt{out}{o}{string}{output file to store ths histogram. The values are stored in pairs (bin-center, count)}
  \cmdopt{bins}{}{}{number of bins to use}
  \cmdopt{min}{}{float}{minimum of the intensity range, lover intensity values go to the first bin}
  \cmdopt{max}{}{float}{maximum of the intensity range, higher intensity values go to the last bin}
  }

  \item [Example:]Evaluate the histogram over all consecutively numbered images that follow the 
  numbering pattern inputXXXX.exr and store the output histogram in histo.txt.  
  Use the range [0,256] and 64 bins. 
   \
  \begin{lstlisting}
mia-multihisto -i input0000.exr -o histo.txt --min 0 --max 256 --bins 64
  \end{lstlisting}
  \end{description}
  
  LatexEnd
*/


/*
\autor Gert Wollny <gw.fossdev at gmail.com>
*/

NS_MIA_USE;
using namespace std;

const SProgramDescription g_description = {
	"2D image stack processing", 
	
	"This program evaluates the histogram over a series of 2D images", 
	
	"Evaluate the histogram over all consecutively numbered images that follow the "
	"numbering pattern inputXXXX.exr and store the output histogram in histo.txt. "
	"Use the range [0,256] and 64 bins.", 

	"-i input0000.exr -o histo.txt --min 0 --max 256 --bins 64"
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
	float hmax = 65536;
	size_t bins = 65536;

	string out_filename;
	string in_filename;
		
	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", CCmdOption::required));
	options.add(make_opt( out_filename, "out", 'o', "output file name", CCmdOption::required));
	options.add(make_opt( hmin, "min", 0, "minimum of histogram range"));
	options.add(make_opt( hmax, "max", 0, "maximum of histogram range"));
	options.add(make_opt( bins, "bins", 0, "number of histogram bins"));
		
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);
		
	if (start_filenum >= end_filenum)
		throw invalid_argument(string("no files match pattern ") + src_basename);


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
	return EXIT_SUCCESS;

}
#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
