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
  LatexBeginProgramSection{2D image stack processing}
  \label{sec:2dstackprograms}
  
  These programs all take a series of consecutively numbered input files of the 
  same dimensions and size and process it as if it would be a 3D image. 
  For that reason only filters are supported that can process the the slices in one pass. 

  LatexEnd
*/

/*
  LatexBeginProgramDescription{2D image stack processing}

  \subsection{mia-2dstackfilter}
  \label{mia-2dstackfilter}
  
  \begin{description}
  \item [Description:] This program is used to filter and convert a series of 2D gray scale images in a 
  3D fashion by running filters as given on the command line. 
  For available filters see section \ref{sec:fifof}. 

  The program is called like 
  \
  \begin{lstlisting}
mia-2dstackfilter -i <input images> -o <output image basename> \
                  -t <output file type> [<filter>] ... 
  \end{lstlisting}
  with the filters given as extra parameters as additional command line parameters. 

  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optoutfile
  \opttypetwod
  \opthelpplugin
  }

  \item [Example:]Run a mean-least-varaiance filter on a series of images that follow the 
  numbering pattern imageXXXX.exr and store the output in images filteredXXXX.exr 
   \
  \begin{lstlisting}
mia-2dstackfilter -i image0000.exr -o filtered -t exr mlv:w=2 
  \end{lstlisting}
  \end{description}
  
  LatexEnd
*/


#define VSTREAM_DOMAIN "2dstackfilter" 


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <stack>
#include <stdexcept>
#include <ctime>

#include <mia/core.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/3d/2dimagefifofilter.hh>


static const char *program_info = 
	"This program is used to filter conscutive numbered 2D images in a 3D manner.\n"
	"This is useful for out-of-core processing of large files.\n"
	"Usage:\n"
	"  mia-2dstackfilter -i <input> -o <output> -t <output-type> <plugin> [<plugin>] ...\n"; 

using namespace std;
using namespace mia;

class C2DStackSaver: public  TFifoFilter<P2DImage> {

public:
	C2DStackSaver(string const & fnamebase, size_t start_num, size_t end_num, size_t fwidth,
		      string const& filetype, C2DImageIOPluginHandler::Instance const& ifh, time_t start_time);
private:
	virtual void do_push(::boost::call_traits<P2DImage>::param_type x);

	string m_fnamebase;
	size_t m_start_num;
	size_t m_nslices;
	size_t m_slice;
	C2DImageIOPluginHandler::Instance const& m_ifh;
	string m_filetype;
	time_t m_start_time;
};

C2DStackSaver::C2DStackSaver(string const & fnamebase, size_t start_num, size_t end_num, size_t fwidth,
			     string const& filetype, C2DImageIOPluginHandler::Instance const& ifh, time_t start_time):
	TFifoFilter<P2DImage>(0,0,0),
	m_start_num(start_num),
	m_nslices(end_num - start_num),
	m_slice(start_num),
	m_ifh(ifh),
	m_filetype(filetype),
	m_start_time(start_time)

{
	stringstream ss;
	ss << fnamebase << "%0" << fwidth << "d." << filetype;
	m_fnamebase  = ss.str();
}

void C2DStackSaver::do_push(::boost::call_traits<P2DImage>::param_type image)
{
	TRACE_FUNCTION; 
	C2DImageVector img_list;
	img_list.push_back(image);
	string out_filename = create_filename(m_fnamebase.c_str(), m_slice++);

	cvdebug() << "C2DStackSaver: save image " << out_filename << " to type "<< m_filetype << '\n';
#ifndef WIN32
	if (cverb.get_level() == vstream::ml_message) {
		char esttime[30];
		time_t now = time(NULL);
		time_t est_end = (m_nslices * (now - m_start_time)) / (m_slice - m_start_num) + m_start_time;
		ctime_r(&est_end, esttime);
		char *est = esttime;
		while (*est != '\n' && *est != 0)
			++est;
		if (*est == '\n')
			*est = ' ';
		cvmsg() << "Filtered " << m_slice << ", estimated finish at: " <<  esttime << "\r";
	}
#endif

	bool save_okay = m_ifh.save(m_filetype, out_filename, img_list );
	if (!save_okay)
		cverr() << "saving file " << out_filename << "failed\n";

}

int main(int argc, const char *argv[])
{
	string in_filename;
	string out_filename;
	string out_type;
	vector<int> new_size;
	bool help_plugins = false;

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
	const C2DFifoFilterPluginHandler::Instance& sfh = C2DFifoFilterPluginHandler::instance();


	CCmdOptionList options(program_info);
	options.push_back(make_opt( in_filename, "in-file", 'i',
				    "input image(s) to be filtered", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o',
				    "output file name base", CCmdOption::required));
	options.push_back(make_opt( out_type, imageio.get_set(), "type", 't',
				    "output file type (if not given deduct from output file name)", CCmdOption::required));
	options.push_back(make_help_opt( "help-plugins", 0,
					 "give some help about the filter plugins", 
					 new TPluginHandlerHelpCallback<C2DFifoFilterPluginHandler>)); 

	try{
		if (options.parse(argc, argv) != CCmdOptionList::hr_no)
			return EXIT_SUCCESS; 

		vector<const char *> filter_chain = options.get_remaining();

		if (help_plugins) {
			sfh.print_help(cout);
			return EXIT_SUCCESS;
		}


		if (filter_chain.empty()) {
			cvwarn() << "No filters given, will only copy files ";
		}

		// now start the fun part
		//first count the number of slices
		vector<const char *>::const_iterator i = filter_chain.begin();

		C2DFifoFilterPlugin::ProductPtr filter = sfh.produce(*i);
		++i;
		while ( i != filter_chain.end()) {
			C2DFifoFilterPlugin::ProductPtr f = sfh.produce(*i);
			if (!filter){
				stringstream error;
				error << "Filter " << *i << " not found";
				throw invalid_argument(error.str());
			}
			filter->append_filter(f);
			++i; 
		}

		size_t start_filenum = 0;
		size_t end_filenum  = 0;
		size_t format_width = 0;



		string src_basename = get_filename_pattern_and_range(in_filename, start_filenum,
								     end_filenum, format_width);
	       
		if (start_filenum >= end_filenum)
			throw invalid_argument(string("no files match pattern ") + src_basename);


		std::shared_ptr<C2DStackSaver >
			  endchain(new C2DStackSaver(out_filename, start_filenum, end_filenum, format_width,
						     out_type, imageio, time(NULL)));

		filter->append_filter(endchain);


		//		char new_line = cverb.show_debug() ? '\n' : '\r';

		cvmsg() << "will filter " << end_filenum - start_filenum << " images\n";

		// read all the files
		for (size_t i = start_filenum; i < end_filenum; ++i) {

			string src_name = create_filename(src_basename.c_str(), i);

			C2DImageIOPluginHandler::Instance::PData in_image_list = imageio.load(src_name);

			if (!in_image_list.get() || !in_image_list->size()) {
				cverr() << "expected " << end_filenum - start_filenum <<
					" images, got only" << i - start_filenum <<"\n";
				break;
			}

			filter->push(*in_image_list->begin());

		}

		cvdebug() << "\nrun finalize\n";  
		filter->finalize();
		cvdebug() << "done";  
		
		cvmsg() << '\n';
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

