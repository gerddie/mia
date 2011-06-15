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
  LatexBeginProgramDescription{Myocardial Perfusion Analysis}
  
  \subsection{mia-2dmultiimagevar}
  \label{mia-2dmultiimagevar}

  \begin{description} 
  \item [Description:] 
        This program evauates the pixel-wise variation of a series of images of the same size 
        and stores the result in an image. 
  
  The program is called like 
  \begin{lstlisting}
mia-2dmyoica-nonrigid -o <output image> <image> <image> [<image>] ... 
  \end{lstlisting}
  The output image will have floating point valued pixels. 

  \item [Options:] $\:$

  \optiontable{
  \cmdopt{out-file}{o}{string}{output image}
  }

  \item [Example:]Evaluate the pixel-wise variation over the image series stored in imagesXXXX.png and
                 store the result in var.exr. 
  \begin{lstlisting}
mia-2dmultiimagevar  -o var.exr images*.png
  \end{lstlisting}
  \item [See also:] \sa{mia-2dseriesgradMAD}, \sa{mia-2dseriessmoothgradMAD}, 
                    \sa{mia-2dseriesgradvariation}
  \end{description}
  
  LatexEnd
*/


#include <sstream>
#include <functional>
#include <cmath>
#include <mia/core.hh>
#include <mia/core/errormacro.hh>
#include <mia/2d.hh>

NS_MIA_USE;
using namespace std;

class CVarAccumulator : public TFilter<bool> {
public:
	CVarAccumulator():
		m_n(0)
	{
	}

	template <typename T>
	bool operator () (const T2DImage<T>& image) {
		if (!m_n) {
			m_sum = C2DDImage(image.get_size(), image);
			m_sum2 = C2DDImage(image.get_size());
		}else if (image.get_size() != m_sum.get_size()) {
			THROW(invalid_argument, "input images differ in size");
		}

		transform(image.begin(), image.end(), m_sum.begin(), m_sum.begin(), 
			  [](double x, double y){return x + y;});
		
		transform(image.begin(), image.end(), m_sum2.begin(), m_sum2.begin(), 
			  [](double x, double y){return x * x + y;});
		++m_n;
		return true;
	}

	P2DImage result()const
	{
		C2DFImage *image = new C2DFImage( m_sum.get_size());
		P2DImage result(image);
		transform(m_sum.begin(), m_sum.end(), m_sum2.begin(), image->begin(), 
			  [m_n](double sum, double sum2){return (sum2 - sum * sum/m_n)/(m_n - 1);}); 
		transform(image->begin(), image->end(), image->begin(),  
			  [](double x) {return sqrt(x);}); 
		return result;

	}
private:
	C2DDImage m_sum;
	C2DDImage m_sum2;
	size_t m_n;
};


const char *g_description = 
	"This program evaluates the intensity variation of a set of image sgiven on the command line"; 


int main( int argc, const char *argv[] )
{

	string out_filename;
	string out_type("vista");

	try {

		const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();


		CCmdOptionList options(g_description);
		options.add(make_opt( out_filename, "out-file", 'o', "output image ", CCmdOption::required));

		if (options.parse(argc, argv) != CCmdOptionList::hr_no)
			return EXIT_SUCCESS; 


		if (options.get_remaining().empty())
			throw runtime_error("no input images given ...");

		if ( out_filename.empty() )
			throw runtime_error("'--out-base' ('o') option required");


		vector<const char *> input_images = options.get_remaining();

		CVarAccumulator ic;

		for (vector<const char *>::const_iterator  i = input_images.begin(); i != input_images.end(); ++i) {

			cvmsg() << "Load " << *i << "\r";
			C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(*i);

			if (in_image_list.get() && in_image_list->size()) {
				accumulate(ic, **in_image_list->begin());
			}
		}
		cvmsg() << "\n";

		C2DImageVector result;
		result.push_back(ic.result());

		if (imageio.save(out_filename, result))
			return EXIT_SUCCESS;
		else
			cerr << argv[0] << " fatal: unable to output image to " <<  out_filename << endl;

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
