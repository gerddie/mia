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
  LatexBeginProgramDescription{3D image processing}
  
  \subsection{mia-3dbrainextractT1}
  \label{mia-3dbrainextract3dT1}

  \begin{description}
  \item [Description:] 	This program is used to extract the brain from T1 MR images. 
  It first runs a combined fuzzy c-means clustering and B-field correction 
  to facilitate a 3D segmentation of 3D image (cf. \cite{pham99fuzzy}). 
  Then various fiters are run to obtain a white matter segmentation as initial 
  mask that is then used to run a region growing to obtain a mask of the whole brain. 
  Finally, this mask is used to extact the brain from the $B^0$ field corrected images.  
  \
  \begin{lstlisting}
mia-3dbrainextract3dT1 -i <input image> -o <output image> [options]
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \optoutfile
  \cmdopt{no-of-classes}{n}{int}{Number of classes to segment}
  \cmdopt{wm-class}{w}{int}{Index of white matter class (normally this should be \texttt{-{}-no-of-classes} - 1)}
  \cmdopt{wm-prob}{p}{float}{White matter probability to create the initial mask from the white matter 
                             class probability image}
  \cmdopt{grow-threshold}{t}{int}{low intensity threshold for region growing}
  \cmdopt{grow-shape}{}{string}{Neighbourhood shape for the region growing (section \ref{sec:3dshapes})}.
  }

  \item [Example:]Create a mask from the input image by running a 5-class segmentation over inpt image input.v 
                and use class 4 as white matter class and store the masked image in masked.v 
     and the B0-field corrected image in b0.v 
   \
  \begin{lstlisting}
mia-3dbrainextract3dT1 -i input.v -n 5 -w 4 -o masked.v
  \end{lstlisting}
  \end{description}
  
  LatexEnd
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>
#include <cstdlib>
#include <string>
//#include <dlfcn.h>
#include <algorithm>

#include <boost/filesystem/convenience.hpp>


#include <mia/3d/fuzzyseg.hh>
#include <mia/3d/3dfilter.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/datapool.hh>

NS_MIA_USE
using namespace std;
namespace bfs = ::boost::filesystem;

const SProgramDescrption g_description = {
	"3D image processing", 
	
	"This program is used to extract the brain from T1 MR images. "
	"It first runs a combined fuzzy c-means clustering and B-field correction "
	"to facilitate a 3D segmentation of 3D image. "
	"Then various fiters are run to obtain a white matter segmentation as initial "
	"mask that is then used to run a region growing to obtain a mask of the whole brain. "
	"Finally, this mask is used to extact the brain from the B0 field corrected images.", 

	"Create a mask from the input image by running a 5-class segmentation over inpt image input.v "
        "and use class 4 as white matter class and store the masked image in masked.v "
	"and the B0-field corrected image in b0.v", 
	
	"-i input.v -n 5 -w 4 -o masked.v"
}; 
	


int do_main( int argc, char *argv[] )
{

	const char *b0poolkey = "b0.@";
	string in_filename;
	string out_filename;
	int    noOfClasses = 3;
	int    wmclass = noOfClasses - 1;
	float  residuum = 0.1;
	int    growthresh = 20;
	string growshape("18n");
	float  wmclassprob = 0.7;

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i',
			      "input image(s) to be segmented", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "brain mask", CCmdOption::required ));
	options.add(make_opt( noOfClasses, "no-of-classes", 'n', "number of classes"));
	options.add(make_opt( wmclass,     "wm-class",      'w', "index of white matter"));
	options.add(make_opt( wmclassprob, "wm-prob", 'p',
			      "white matter class probability for initial mask creation"));
	options.add(make_opt( growthresh, "grow-threshold", 't', "intensity threshold for region growing"));
	options.add(make_opt( growshape, "grow-shape", 0, "neighbourhood mask region growing"));


	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	auto& imageio = C3DImageIOPluginHandler::instance();
	auto inImage_list = imageio.load(in_filename);

	if (!inImage_list.get() || !inImage_list->size() ) {
		string not_found = ("No supported data found in ") + in_filename;
		throw runtime_error(not_found);
	}

	if (wmclass >= noOfClasses)
		throw invalid_argument("number of classes should be smaller then number of classes");

	// segment image
	if (inImage_list->size() > 1)
		cvwarn() << "Only segmenting first input image\n";

	C3DImageVector classes;

	P3DImage b0_corrected = fuzzy_segment_3d(**inImage_list->begin(), noOfClasses, residuum, classes);
	CDatapool::instance().add(b0poolkey, create_image3d_vector(b0_corrected));


	P3DImage result = classes[wmclass];

	// create filter chain
	vector<string> filter_chain;
	stringstream binarize;
	binarize << "binarize:min=" << wmclassprob;
	filter_chain.push_back(binarize.str());

	filter_chain.push_back("erode:shape=6n");
	filter_chain.push_back("label");
	filter_chain.push_back("selectbig");
	stringstream grow;
	grow << "growmask:ref="<< b0poolkey <<",min=" << growthresh << ",shape=" << growshape;
	filter_chain.push_back(grow.str());
	filter_chain.push_back("close:shape=[sphere:r=3]");
	filter_chain.push_back("open:shape=[sphere:r=3]");
	filter_chain.push_back(string("mask:input=") + b0poolkey);

	vector<P3DFilter> filters = create_filter_chain(filter_chain);

	for (auto f = filters.begin(); f != filters.end(); ++f) {
		result = (*f)->filter(*result);
	}


	if (!out_filename.empty()) {

		// save corrected image to out-file
		C3DImageIOPluginHandler::Instance::Data out_list;

		out_list.push_back(result);
		if ( !imageio.save(out_filename, out_list) ){

			string not_save = ("unable to save result to ") + out_filename;
			throw runtime_error(not_save);

		};

	};

	return EXIT_SUCCESS;

}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
