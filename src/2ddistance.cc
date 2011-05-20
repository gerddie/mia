/* -*- mia-c++ -*- 
 *
 * Copyright (c) 2005-2011 Leipzig, Madrid 
 *  
 *  Max-Planck-Institute of Evolutionary Anthropology
 *  BIT, ETSI Telecomunicacion, UPM
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

// scaling of the distance field 
#define SCALE 256

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
	{"avg", dist_avg},
	{"max", dist_max},
	{NULL, dist_unknown},
};


template <typename T> 
struct __convert_to_double {
	static C2DDImage apply(const T2DImage<T>& image, double scale) {
		if (scale <= 0.0) 
			THROW(invalid_argument, "Scaling factor must be >= 0 but it is " << scale); 
		C2DDImage result(image.get_size()); 
		transform(image.begin(), image.end(),  result.begin(), 
			  [scale](T x){return x/scale;}); 
		return result; 
	}
};


template <> 
struct __convert_to_double<float> {
	static C2DDImage apply(const C2DFImage& image, double scale) {
		cvdebug() << "Input is floating point and will not be scaled\n"; 
		C2DDImage result(image.get_size()); 
		copy(image.begin(), image.end(),  result.begin()); 
		return result; 
	}
};

template <> 
struct __convert_to_double<double> {
	static C2DDImage apply(const C2DDImage& image, double scale) {
		return image; 
	}
};

class Convert2DoubleAndScale: public TFilter<C2DDImage>{
 public: 
	Convert2DoubleAndScale(double scale):_M_scale(scale){
	}
	
	template <typename T> 
	C2DDImage operator ()(const T2DImage<T>& image) const {
		return __convert_to_double<T>::apply(image, _M_scale); 
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
	float scale = 256.0; 
	EOps method; 
	
	
	TDictMap<EOps> combine_option(combine_option_table); 


	CCmdOptionList options(g_general_help);
	options.add(make_opt( in_filename, "in-file", 'i', "input image", CCmdOption::required)); 
	options.add(make_opt( dist_filename, "distance-file", 'd', "distance field image", CCmdOption::required)); 
	options.add(make_opt( scale, "scale", 's', "distance scaling factor")); 
	options.add(make_opt( method, combine_option, "method", 'm', "distance measuring method")); 
	

	try {
		if (options.parse(argc, argv, false) != CCmdOptionList::hr_no)
			return EXIT_SUCCESS; 
		
		
                auto in_image = load_image2d(in_filename);
                auto dist_image = load_image2d(in_filename);

		
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
