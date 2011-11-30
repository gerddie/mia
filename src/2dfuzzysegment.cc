/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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
  LatexBeginProgramDescription{2D image processing}
  
  \subsection{mia-fuzzysegment2d}
  \label{mia-fuzzysegment2d}

  \begin{description}
  \item [Description:] 	This program runs a combined fuzzy c-means clustering and B-field correction 
  to facilitate a 2D segmentation of 2D image (cf. \cite{pham99fuzzy}). 
  \
  \begin{lstlisting}
mia-fuzzysegment2d -i <input image> -o <output image> ... 
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \cmdopt{b0-file}{o}{string}{file to store the b0-field corrected image}
  \cmdopt{cls-file}{c}{string}{file to store the class probability images. 
                The file type must support the storage of multiple images}
  \cmdopt{no-of-classes}{n}{int}{Number of classes to segment}
  \cmdopt{residuum}{r}{float}{relative residuum for the B-field correction optimization}
  }

  \item [Example:]Run a 5-class segmentation over inpt image input.v and store the class probability images in cls.v 
     and the B0-field corrected image in b0.v 
   \
  \begin{lstlisting}
mia-fuzzysegment2d -i input.v -c 5 -o b0.v -c cls.v   
  \end{lstlisting}
  \end{description}
  
  LatexEnd
*/

/*
\remark The programm was origionall written by D.L. Pham and J.L.Prince, Pat. Rec. Let., 20:57-68,1999.
\remark The CG optimization was included by Stefan Burckhardt und Carsten Walters
\author Heike J"anicke and M. Tittgemeyer (tittge@cbs.mpg.de), 2004
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>
#include <cstdlib>
#include <string>
//#include <dlfcn.h>
#include <algorithm>

#include <mia/2d/fuzzyseg.hh>
#include <mia/core/cmdlineparser.hh>

NS_MIA_USE
using namespace std;

const SProgramDescrption g_description = {
	"2D image processing", 
	
	"This program runs a combined fuzzy c-means clustering and B-field correction "
	"to facilitate a fuzzy segmentation of 2D image", 
	
	"Run a 5-class segmentation over input image input.v and store the class "
	"probability images in cls.v and the B0-field corrected image in b0.v.", 
	
	"-i input.v -c 5 -o b0.v -c cls.v"
}; 



int do_main( int argc, char *argv[] )
{


	string in_filename;
	string out_filename;
	string gain_filename;
	string cls_filename;
	int    noOfClasses = 3;
	SFuzzySegParams params; 



	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i',
			      "input image(s) to be segmenetd", CCmdOption::required));
	options.add(make_opt( cls_filename, "cls-file", 'c',
			      "output class probability images", CCmdOption::required));
	options.add(make_opt( out_filename, "b0-file", 'o',
			      "image corrected for intensity non-uniformity" ));
	options.add(make_opt( gain_filename, "gain-file", 'g',
			      "gain field (floating point valued)" ));

	options.add(make_opt( noOfClasses, "no-of-classes", 'n',
			      "number of classes"));
	options.add(make_opt( params.residuum, "residuum", 'r', "relative residuum"));
	options.add(make_opt( params.lambda1, "l1", 0, "Penalize magnitude of intensity inhomogeinity correction"));
	options.add(make_opt( params.lambda2, "l2", 0, "Smoothness of intensity inhomogeinity correction"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	// required options (anything that has no default value)
	if ( in_filename.empty() )
		throw runtime_error("'--in-file'  ('i') option required\n");
	if ( in_filename.empty() )
		throw runtime_error("'--cls-file' ('c') option required\n");

	const C2DImageIOPluginHandler::Instance&
		imageio = C2DImageIOPluginHandler::instance();

	C2DImageIOPluginHandler::Instance::PData inImage_list = imageio.load(in_filename);

	if (!inImage_list.get() || !inImage_list->size() ) {
		string not_found = ("No supported data found in ") + in_filename;
		throw runtime_error(not_found);
	}

	// segment image
	if (inImage_list->size() > 1)
		cvwarn() << "Only segmenting first input image\n";

	C2DImageVector classes;
	P2DImage gain; 
	P2DImage b0_corrected = fuzzy_segment_2d(**inImage_list->begin(), noOfClasses, params, classes, gain);

	if (!out_filename.empty()) {

		// save corrected image to out-file
		C2DImageIOPluginHandler::Instance::Data out_list;

		out_list.push_back(b0_corrected);
		if ( !imageio.save(out_filename, out_list) ){

			string not_save = ("unable to save result to ") + out_filename;
			throw runtime_error(not_save);

		};

	};

	//CHistory::instance().append(argv[0], revision, opts);

	if ( !imageio.save(cls_filename, classes) ){
		string not_save = ("unable to save result to ") + cls_filename;
		throw runtime_error(not_save);

	}

	if (!gain_filename.empty()) {
		if (!save_image(gain_filename, gain)) 
			THROW(runtime_error, "unable to save gain field to '" << gain_filename << "'"); 
	}
	
	return EXIT_SUCCESS;

}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 