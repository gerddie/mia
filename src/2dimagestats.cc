/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

/*
  LatexBeginProgramDescription{Miscellaneous programs}

  \subsection{mia-2dimagestats}
  \label{mia-2dimagestats}
  
  \begin{description}
  \item [Description:] 	This progranm is used to evaluate some statistics of a image.
	Output is Mean, Variation, Median, and Median Average Distance of the intensity values.

  The program is called like 
  \
  \begin{lstlisting}
mia-2dimagestats -i <file name>
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \cmdopt{thresh}{t}{float}{Intensity threshold to ignore when evaluating the statistics}
  \cmdopt{high-thresh}{g}{float}{Percentage of values to ignore at the upper and of the histogram}
  }

  \item [Example:] Evaluate the statistics of image input.v and ignore all voxels with an intensity 
  below 5 and 0.1\% of the high end of the histogram
   \
  \begin{lstlisting}
mia-2dimagestats -i input.v -t 5 -g 0.1
  \end{lstlisting}
  \end{description}

  LatexEnd
*/

#include <sstream>
#include <mia/core.hh>
#include <mia/2d.hh>

NS_MIA_USE;
using namespace std;

const char *g_description = 
	"This progranm is used to evaluate some statistics of an image. " 
	"Output is Mean, Variation, Median, and Median Average Distance of the intensity values."
	; 
	
	

class CHistAccumulator : public TFilter<bool> {
public:
	CHistAccumulator(float min, float max, size_t bins, float thresh):
		m_histo(THistogramFeeder<float>(min, max, bins)),
		m_thresh(thresh)
	{
	}

	template <typename T>
	bool operator () (const T2DImage<T>& image) {
		for (typename T2DImage<T>::const_iterator i = image.begin();
		     i != image.end(); ++i)
			if (*i > m_thresh)
				m_histo.push(*i);
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
		       << tmp.median() << " " << tmp.MAD() << '\n';
	}

	THistogram<THistogramFeeder<float > > m_histo;
	float m_thresh;
};



int main( int argc, const char *argv[] )
{

	string in_filename;
	float thresh = 10.0;
	float high_thresh = 0.05;
	try {

		const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();


		CCmdOptionList options(g_description);
		options.add(make_opt( in_filename, "in-file", 'i', "input image to be analyzed", 
					    CCmdOption::required));
		options.add(make_opt( thresh, "thresh", 't', "intensity thresh to ignore"));
		options.add(make_opt( high_thresh, "high-thresh", 'g', "upper histogram percentage to ignore"));

		if (options.parse(argc, argv) != CCmdOptionList::hr_no)
			return EXIT_SUCCESS; 


		//CHistory::instance().append(argv[0], "unknown", options);

		// read image
		C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(in_filename);

		if (in_image_list.get() && in_image_list->size()) {
			CHistAccumulator histo(0, 4096, 1024, thresh);
			for (C2DImageIOPluginHandler::Instance::Data::iterator i = in_image_list->begin();
			     i != in_image_list->end(); ++i)
				accumulate(histo, **i);
			histo.print_stats(high_thresh);
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
