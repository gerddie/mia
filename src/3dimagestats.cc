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

  \subsection{mia-3dimagestats}
  \label{mia-3dimagestats}
  
  \begin{description}
  \item [Description:] This program prints out some statistics about the image data. 
  Currently, this is the average intensity and the intensity variation. 
  The program is called like 
  \
  \begin{lstlisting}
mia-3dimagestats -i <file name>
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \cmdopt{thresh}{t}{float}{Intensity threshold to ignore when evaluating the statistics}
  }

  \item [Example:] Evaluate the statistics of image input.v and ignore all voxels with an intensity 
  below 5: 
   \
  \begin{lstlisting}
mia-3dimagestats -i input.v -t 5 
  \end{lstlisting}
  \end{description}

  LatexEnd
*/



#include <sstream>
#include <mia/core.hh>
#include <mia/3d.hh>

NS_MIA_USE;
using namespace std;

const char *program_info = 
	"This program is used to evaluate certain image statistics\n"
	"and print them out on the command line\n"
	"Basic usage:\n"
	"  mia-3dimagestats [options] [<filter1>] [<filter1>] ...\n";

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



int main( int argc, const char *argv[] )
{

	string in_filename;
	float thresh = 10.0;
	try {

		const C3DImageIOPluginHandler::Instance& imageio = C3DImageIOPluginHandler::instance();


		CCmdOptionList options(program_info);
		options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", 
					    CCmdOption::required));
		options.add(make_opt( thresh, "thresh", 't', "intensity thresh to ignore"));

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
