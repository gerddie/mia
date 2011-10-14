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

  \subsection{mia-2ddistance}
  \label{mia-2ddistance}
  
  \begin{description}
  \item [Description:] This program evaluates the maximum or avarage distance of a binary mask 
                       using a distance map that was evaluated by using the 
		       \hyperref[filter2d:distance]{distance image filter}. 
		       The result is written to stdout. 

  The program is called like 
  \
  \begin{lstlisting}
mia-2ddistance -i <mask> -d <distance> [options]
  \end{lstlisting}
  \item [Options:] $\:$

  \optiontable{
  \optinfile
  \cmdopt{distance-file}{d}{string}{Distance map}
  \cmdopt{scale}{s}{double}{Scale the values of the distance map by dividing by this value}
  \cmdopt{method}{m}{string}{Method to combine the values of the distance measure. 
                            Average(avg) and maximum(max) are supported.} 
  }

  \item [Example:] Evaluate the distances of the pixels in mask.png by using the map distance.v
                   and return the maximum distance
   \
  \begin{lstlisting}
mia-2ddistance -i mask.png -d distance.v -m max
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
#include <algorithm>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/2dimageio.hh>

using namespace std;
NS_MIA_USE

const char *g_general_help = 
	"This program evaluate the average or maximum distance of a mask\n"
	"given by a binary image to an image representing a distance map\n"
	"and prints the result to stdout.\n\n"
	"Basic usage: \n"
	"  mia-2ddistance -i <mask> -d <distance>  [options]"; 


enum EOps {dist_avg, 
	   dist_max, 
	   dist_unknown
}; 


const TDictMap<EOps>::Table combine_option_table[] = {
	{"avg", dist_avg, "use average"},
	{"max", dist_max, "use maximum" },
	{NULL, dist_unknown, ""},
};

class Convert2DoubleAndScale: public TFilter<C2DDImage>{
 public: 
	Convert2DoubleAndScale(double scale):_M_scale(scale){
	}
	
	template <typename T> 
	C2DDImage operator ()(const T2DImage<T>& image) const {
		C2DDImage result(image.get_size()); 
		transform(image.begin(), image.end(),  result.begin(), 
			  [_M_scale](T x){return x/_M_scale;}); 
		return result; 
	}
private:
	double _M_scale; 
};

class CGetDistance: public TFilter<double> {
public: 
	CGetDistance(const C2DDImage& dist_field, EOps method): 
		_M_distance(dist_field),
		_M_method(method)
	{
	}
	
	template <typename T> 
	CGetDistance::result_type operator()(const T2DImage<T>& image) const 
	{
		
		auto d = _M_distance.begin(); 
		auto i = image.begin(); 
		auto e = image.end(); 

		switch (_M_method) {
		case dist_avg: {
			int n = 0; 
			double result = 0.0; 
			while (i != e) {
				if (*i) {
					result += *d; 
					++n; 
				}
				++d; 
				++i; 
			}
			return n == 0 ? 0 : result / n; 
		}
		case dist_max: {
			double result = 0.0; 
			while (i != e) {
				if (*i) 
					if (result < *d)
						result = *d; 
				++d; 
				++i; 
			}
			return result; 
		}
		default: 
			throw runtime_error("unknown distance measure requested\n"); 
		}
	}
private: 
	const C2DDImage& _M_distance; 
	EOps _M_method; 
};

int main( int argc, const char *argv[] )
{

	string in_filename;
	string dist_filename;
	float scale = 1.0; 
	EOps method = dist_avg; 
	
	
	TDictMap<EOps> combine_option(combine_option_table); 


	CCmdOptionList options(g_general_help);
	options.add(make_opt( in_filename, "in-file", 'i', "input image", CCmdOption::required)); 
	options.add(make_opt( dist_filename, "distance-file", 'd', "distance field image", CCmdOption::required)); 
	options.add(make_opt( scale, "scale", 's', "distance scaling factor")); 
	options.add(make_opt( method, combine_option, "method", 'm', "distance measuring method")); 
	

	try {
		if (options.parse(argc, argv) != CCmdOptionList::hr_no)
			return EXIT_SUCCESS; 
		
		
                auto in_image = load_image2d(in_filename);
                auto dist_image = load_image2d(dist_filename);

		
		Convert2DoubleAndScale create_dist(scale); 
		C2DDImage dist = mia::filter(create_dist, *dist_image); 

		CGetDistance get_distance(dist, method); 
		cout << filter(get_distance, *in_image) <<"\n";
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
