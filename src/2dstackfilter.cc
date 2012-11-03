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
#define VSTREAM_DOMAIN "2dstackfilter" 


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <stack>
#include <stdexcept>
#include <ctime>

#include <mia/core.hh>
#include <mia/2d/imageio.hh>
#include <mia/3d/2dimagefifofilter.hh>

#include <tbb/task_scheduler_init.h>

using tbb::task_scheduler_init; 
using namespace std;
using namespace mia;

const SProgramDescription g_description = {
        {pdi_group, "Processing of series of 2D images in a 3D fashion (out-of-core)"}, 
	{pdi_short, "Filter a series of 2D images in a 3D fashion."}, 
	{pdi_description, "This program is used to filter and convert a series of 2D "
	 "gray scale images in a 3D fashion by running filters (filter/2dimage) "
	 "as given on the command line."}, 
	{pdi_example_descr, "Run a mean-least-varaiance filter on a series of images that follow the "
	 "numbering pattern imageXXXX.exr and store the output in images filteredXXXX.exr"}, 
	{pdi_example_code, "-i image0000.exr -o filtered -t exr mlv:w=2"}
}; 

class C2DStackSaver: public  TFifoFilter<P2DImage> {

public:
	C2DStackSaver(string const & fnamebase, size_t start_num, size_t end_num, size_t fwidth,
		      string const& filetype, C2DImageIOPluginHandler::Instance const& ifh, time_t start_time);
private:
	virtual void do_push(::boost::call_traits<P2DImage>::param_type image);

	string m_fnamebase;
	size_t m_start_num;
	size_t m_nslices;
	size_t m_slice;
	C2DImageIOPluginHandler::Instance const& m_ifh;
	time_t m_start_time;
};

C2DStackSaver::C2DStackSaver(string const & fnamebase, size_t start_num, size_t end_num, size_t fwidth,
			     string const& filetype, C2DImageIOPluginHandler::Instance const& ifh, time_t start_time):
	TFifoFilter<P2DImage>(0,0,0),
	m_start_num(start_num),
	m_nslices(end_num - start_num),
	m_slice(start_num),
	m_ifh(ifh),
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

	cvdebug() << "C2DStackSaver: save image " << out_filename << '\n';
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

	bool save_okay = m_ifh.save(out_filename, img_list );
	if (!save_okay)
		cverr() << "saving file " << out_filename << "failed\n";

}

int do_main(int argc, char *argv[])
{
	string in_filename;
	string out_filename;
	string out_type;
	vector<int> new_size;

	int max_threads = task_scheduler_init::automatic;
	
	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
	const C2DFifoFilterPluginHandler::Instance& sfh = C2DFifoFilterPluginHandler::instance();


	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", CCmdOption::required, &imageio));
	options.add(make_opt( out_filename, "out-file", 'o', "output file name base, the actual names is created "
			      "by adding the file number based on output order and the extension bysed on the 'type' parameter"
			      , CCmdOption::required, &imageio));
	options.add(make_opt( out_type, imageio.get_supported_suffix_set(), "type", 't',
			      "output file type (if not given deduct from output file name)", CCmdOption::required));
	options.add(make_help_opt( "help-plugins", 0, 
				   "give some help about the filter plugins", 
				   new TPluginHandlerHelpCallback<C2DFifoFilterPluginHandler>)); 

	options.set_group("Processing"); 
	options.add(make_opt(max_threads, "threads", 'T', "Maxiumum number of threads to use for running the registration," 
			     "This number should be lower or equal to the number of processing cores in the machine"
			     " (default: automatic estimation)."));  


	if (options.parse(argc, argv, "filter", &sfh) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	auto filter_chain = options.get_remaining();

	if (filter_chain.empty()) 
		throw invalid_argument("No filters given, bailing out.");

	task_scheduler_init init(max_threads);

	auto i = filter_chain.begin();
	auto filter = sfh.produce(*i);
	++i;
	while ( i != filter_chain.end()) {
		auto f = sfh.produce(*i);
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

	std::shared_ptr<C2DStackSaver>
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
#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
