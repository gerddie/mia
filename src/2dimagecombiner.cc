/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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
  LatexBeginProgramDescription{2D image processing}

  \subsection{mia-2dimagecombiner}
  \label{mia-2dimagecombiner}
  
  \begin{description}
  \item [Description:] This program combines two images $I_1$ and $I_2$ by using one of the 
                       operations defined in the image combiner plug-ins (section \ref{sec:combiner2d}) and 
		       writes the resulting image to a file. 

  The program is called like 
  \
  \begin{lstlisting}
mia-2dimagecombiner -1 <input1> -2 <input2> -o <output> -p <operation> 
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \cmdopt{first}{1}{string}{First input image}
  \cmdopt{second}{2}{string}{Second input image}
  \optoutfile
  \cmdopt{combiner}{c}{string}{Image combiner operation (section \ref{sec:combiner2d})} 
  }

  \item [Example:] Take two images l1.png and l2.png and evaluate the sum of the intensity values. 
   \
  \begin{lstlisting}
mia-2dimagecombiner -1 l1.png -2 l2.png -c result.png -c add
  \end{lstlisting}
  \end{description}
  
  LatexEnd
*/

#include <sstream>
#include <mia/core.hh>
#include <mia/2d.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/core/factorycmdlineoption.hh>

NS_MIA_USE;
using namespace std;
using boost::any_cast; 


static const char *program_info = 
	"This program is used to combine two images.\n"
	"Basic usage:\n"
	"  mia-2dimagecombiner -1 <image 1> -2 <image 2>  -o <output image> [<plugin>]\n";

int do_main( int argc, const char *argv[] )
{

	string in1_filename;
	string in2_filename;
	string out_filename;

	auto combiner = C2DImageCombinerPluginHandler::instance().produce("absdiff");

	stringstream combiner_names;

	CCmdOptionList options(program_info);
	options.add(make_opt( in1_filename, "in-file-1", '1', 
				    "first input image to be combined", CCmdOption::required));
	options.add(make_opt( in2_filename, "in-file-2", '2', 
				    "second input image to be combined", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o',
				    "output image(s) that have been filtered", CCmdOption::required));
	options.add(make_opt( combiner, "operation", 'p', "operation to be applied"));
	
	if (options.parse(argc, argv, false) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

		

	//CHistory::instance().append(argv[0], "unknown", options);

	auto image1 = load_image2d(in1_filename); 
	auto image2 = load_image2d(in2_filename); 

	auto output = combiner->combine(*image1, *image2); 
	if (!save_image(out_filename, output)) 
		THROW(runtime_error, "unable to save result in '" << out_filename << "'");
	
	return EXIT_SUCCESS;
}

int main( int argc, const char *argv[] )
{


	try {
		return do_main(argc, argv);
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

