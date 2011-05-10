/*
 * Copyright (c) 2004 Max-Planck-Institute of Evolutionary Anthropology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

  \subsection{mia-2dbinarycombiner}
  \label{mia-2dbinarycombiner}
  
  \begin{description}
  \item [Description:] This program combines two images $I_1$ and $I_2$ by using one of the 
                       defined binary operations writes the resulting image to a file. 
		       Both input image must be binary images. 

  The program is called like 
  \
  \begin{lstlisting}
mia-2dbinarycombiner -1 <input1> -2 <input2> -o <output> -p <operation> 
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \cmdopt{first}{1}{string}{First input image}
  \cmdopt{second}{2}{string}{Second input image}
  \optoutfile
  \cmdopt{combiner}{c}{string}{Pixel wise binary image combiner operation (and|nand|nor|nxor|or|xor)} 
  }

  \item [Example:] Take two images l1.v and l2.v and union of the mask. 
   \
  \begin{lstlisting}
mia-2dbinarycombiner -1 l1.v -2 l2.v -c result.png -c or 
  \end{lstlisting}
  \end{description}
  
  LatexEnd
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <string>
#include <stdexcept>
#include <dlfcn.h>

#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/2d/2dimageio.hh>


using namespace std;
NS_MIA_USE

enum EBinops  {
	bin_or, 
	bin_nor, 
	bin_and, 
	bin_nand, 
	bin_xor, 
	bin_nxor, 
	bin_unknown
}; 

struct logical_xor : public binary_function<bool,bool,bool> {
	bool operator() (bool a, bool b) const {
		return (a ^ b);
	}
};

P2DImage binary_op(const C2DBitImage& a, const C2DBitImage& b, EBinops op)
{
	assert(a.get_size() == b.get_size()); 
	C2DBitImage *result = new C2DBitImage(a.get_size()); 
	
	switch (op) {
	case bin_or  : 	transform(a.begin(), a.end(), b.begin(), result->begin(), logical_or<bool>()); 
		break; 
	case bin_nor : 
		transform(a.begin(), a.end(), b.begin(), result->begin(), not2(logical_or<bool>())); 
		break; 
	case bin_and : 
		transform(a.begin(), a.end(), b.begin(), result->begin(), logical_and<bool>()); 
		break; 
	case bin_nand:  
		transform(a.begin(), a.end(), b.begin(), result->begin(), not2(logical_and<bool>())); 
		break; 
	case bin_xor : 
		transform(a.begin(), a.end(), b.begin(), result->begin(), logical_xor());
		break; 
	case bin_nxor:  
		transform(a.begin(), a.end(), b.begin(), result->begin(), not2(logical_xor())); 
		break; 
	default: 
		throw invalid_argument("Unknown binary operation requested"); 
	}
	return P2DImage(result); 
}


const TDictMap<EBinops>::Table g_binops_table[] = {
	{"or", bin_or}, 
	{"nor", bin_nor}, 
	{"and", bin_and}, 
	{"nand", bin_nand}, 
	{"xor", bin_xor}, 
	{"nxor", bin_nxor}, 
	{NULL,bin_unknown}, 
}; 

const TDictMap<EBinops> g_binops_dict(g_binops_table);

/* Revision string */
const char g_description[] = "This program is used to combine two binary images";

int main( int argc, const char *argv[] )
{

	string filename1;
	string filename2;
	string out_filename;
	EBinops op = bin_nor; 

	CCmdOptionList options(g_description);
	
	options.push_back(make_opt( filename1, "file1", '1', 
				    "input mask image 1", NULL, CCmdOption::required)); 
	options.push_back(make_opt( filename2, "file2", '2', 
				    "input input mask image 2", NULL, CCmdOption::required)); 
	options.push_back(make_opt(op, g_binops_dict, "operation", 'p', 
				   "Operation to be applied")); 
	options.push_back(make_opt( out_filename, "out-file", 'o', 
					"output mask image", NULL, CCmdOption::required)); 

	try {

		if (options.parse(argc, argv, false) != CCmdOptionList::hr_no) 
			return EXIT_SUCCESS; 

	
		// read images
		P2DImage image1 = load_image2d(filename1); 
		P2DImage image2 = load_image2d(filename2); 

		try {
			const C2DBitImage& img1 = dynamic_cast<const C2DBitImage&>(*image1); 
			const C2DBitImage& img2 = dynamic_cast<const C2DBitImage&>(*image2); 
			P2DImage result = binary_op(img1, img2, op); 
			
			if ( !save_image(out_filename, result) ){
				THROW(runtime_error, "cannot save result to " << out_filename); 
			}
			
		}catch (bad_cast& x) {
			throw invalid_argument("Input images are not binary masks"); 
		}
		
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
