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

/*
  LatexBeginProgramDescription{Image conversion}

  \subsection{mia-2dto3dimageb}
  \label{mia-2dto3dimageb}
  
  \begin{description}
  \item [Description:] This program is used to convert a series 2D images into a 3D image. 
     The 2D images are read as additional command line parameters and the slice 
     ordering corresponds to the ordering of the file names on the commend line. 
  The program is called like 
  \
  \begin{lstlisting}
mia-2dto3dimage  -o <3D output image> <slice1> [<slice2>] ...
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optoutfile
  }

  \item [Example:]Convert a series of images imageXXXX.png to a 3D image 3d.v 
   \
  \begin{lstlisting}
mia-2dto3dimage -i imageXXXX.png -o 3d.v
  \end{lstlisting}
  \item [Remark:] All input images must be of the same pixel type and size. 
  See also \ref{mia-2dto3dimage}. 

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

NS_MIA_USE
using namespace std;
using namespace boost;

const SProgramDescription g_description = {
	"Image conversion", 
	
	"This program is used to convert a series 2D images into a 3D image. "
	"The 2D images are read as additional command line parameters and the slice "
	"ordering corresponds to the ordering of the file names on the commend line.", 
	
	"Convert a series of images imageXXXX.png to a 3D image 3d.v", 
	
	"-i imageXXXX.png -o 3d.v"

}; 

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
		++ m_cur_slice;
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

int do_main( int argc, char *argv[] )
{

	string in_filename;
	string out_filename("3");
	string out_type;

	const C2DImageIOPluginHandler::Instance& image2dio = C2DImageIOPluginHandler::instance();
	const C3DImageIOPluginHandler::Instance& image3dio = C3DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( out_filename, "out-file", 'o', "output file name", CCmdOption::required));

	if (options.parse(argc, argv, "sliceimage") != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
		
	if (options.get_remaining().empty())
		throw runtime_error("no slices given ...");


	CHistory::instance().append(argv[0], revision, options);

	//size_t start_filenum = 0;
	//size_t end_filenum  = 0;
	//size_t format_width = 0;

	char new_line = cverb.show_debug() ? '\n' : '\r';

	vector<const char *> input_images = options.get_remaining();

	C3DImageCollector ic(input_images.size());

	for (vector<const char *>::const_iterator  i = input_images.begin(); i != input_images.end(); ++i) {

		cvmsg() << "Load " << *i << new_line;
		C2DImageIOPluginHandler::Instance::PData  in_image_list = image2dio.load(*i);

		if (in_image_list.get() && in_image_list->size()) {
			accumulate(ic, **in_image_list->begin());
		}
	}
	cvmsg() << "\n";

	C3DImageVector result;
	result.push_back(ic.result());

	if (image3dio.save(out_filename, result))
		return EXIT_SUCCESS;
	else
		cerr << argv[0] << " fatal: unable to output image to " <<  out_filename << endl;
	return EXIT_FAILURE;
}
#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
