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
  LatexBeginProgramDescription{Image conversion}

  \subsection{mia-2dto3dimage}
  \label{mia-2dto3dimage}
  
  \begin{description}
  \item [Description:] This program is used to convert a series of consecutively numbered 
         2D images into a 3D image. 

  The program is called like 
  \
  \begin{lstlisting}
mia-2dto3dimage -i <2D input file pattern> -o <3D output image>
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optoutfile
  \opttypethreed
  }

  \item [Example:]Convert a series of images imageXXXX.png to a 3D image 3d.v 
   \
  \begin{lstlisting}
mia-2dto3dimage -i imageXXXX.png -o 3d.v
  \end{lstlisting}
  \item [Remark:] All input images must be of the same pixel type and size. 
  See also \ref{mia-2dto3dimageb}. 
  \end{description}
  
  LatexEnd
*/


#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>

#include <mia/core.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/3d/3dimageio.hh>

static const char *program_info = 
	"This program is used to combine a series of 2D images of equal \n"
	"size and type images following a certain numbering scheme to a 3D image.\n"
	"Basic usage:\n"
	"  mia-2dto3dimage  -i <input> -o <output image> \n"; 


NS_MIA_USE
using namespace std;
using namespace boost;

struct C3DImageCollector : public TFilter<bool> {

	C3DImageCollector(size_t slices):
		m_slices(slices),
		m_cur_slice(0)
	{
	}

	template <typename T>
	bool operator ()(const T2DImage<T>& image) {

	        if (!m_image)
			m_image = std::shared_ptr<C3DImage > (new T3DImage<T>(C3DBounds(image.get_size().x,
										  image.get_size().y,
										  m_slices)));

		T3DImage<T> *out_image = dynamic_cast<T3DImage<T> *>(m_image.get());
		if (!out_image)
			throw invalid_argument("input images are not all of the same type");

		if (m_cur_slice < m_slices) {
			if (out_image->get_size().x != image.get_size().x ||
			    out_image->get_size().y != image.get_size().y)
				throw invalid_argument("input images are not all of the same size");

			typename T3DImage<T>::iterator out = out_image->begin() +
				image.get_size().x * image.get_size().y * m_cur_slice;

			copy(image.begin(), image.end(), out);
		}
		return true;
	}

	std::shared_ptr<C3DImage > result() const {
		return m_image;
	}

private:
	size_t m_slices;
	size_t m_cur_slice;

	std::shared_ptr<C3DImage > m_image;
};

/* Revision string */
const char revision[] = "not specified";

int main( int argc, const char *argv[] )
{

	string in_filename;
	string out_filename;
	string out_type;

	const C2DImageIOPluginHandler::Instance& image2dio = C2DImageIOPluginHandler::instance();
	const C3DImageIOPluginHandler::Instance& image3dio = C3DImageIOPluginHandler::instance();

	CCmdOptionList options(program_info);
	options.push_back(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output file name", CCmdOption::required));
	options.push_back(make_opt( out_type, image3dio.get_set(), "type", 't',"output file type"));

	try {

		if (options.parse(argc, argv) != CCmdOptionList::hr_no)
			return EXIT_SUCCESS; 


		if (!options.get_remaining().empty())
			throw runtime_error("unknown option given ...");

		if ( in_filename.empty() )
			throw runtime_error("'--in-file' ('i') option required");

		if ( out_filename.empty() )
			throw runtime_error("'--out-base' ('o') option required");


		CHistory::instance().append(argv[0], revision, options);

		size_t start_filenum = 0;
		size_t end_filenum  = 0;
		size_t format_width = 0;

		std::string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);
		if (start_filenum >= end_filenum)
			throw invalid_argument(string("no files match pattern ") + src_basename);

		char new_line = cverb.show_debug() ? '\n' : '\r';

		C3DImageCollector ic(end_filenum - start_filenum);

		for (size_t i = start_filenum; i < end_filenum; ++i) {

			string src_name = create_filename(src_basename.c_str(), i);
			cvmsg() << new_line << "Read: " << i <<" out of "<< "[" << start_filenum<< "," << end_filenum << "]" ;
			C2DImageIOPluginHandler::Instance::PData  in_image_list = image2dio.load(in_filename);

			if (in_image_list.get() && in_image_list->size()) {
				accumulate(ic, **in_image_list->begin());
			}
		}
		cvmsg() << "\n";

		C3DImageVector result;
		result.push_back(ic.result());

		if (image3dio.save(out_type, out_filename, result))
			return EXIT_SUCCESS;
		else
			cerr << argv[0] << " fatal: unable to output image to " <<  out_filename << endl;
	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (const std::exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}
