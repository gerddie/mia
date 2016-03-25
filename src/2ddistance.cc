/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
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
#include <mia/internal/main.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/imageio.hh>

using namespace std;
NS_MIA_USE


const SProgramDescription g_general_help = {
	{pdi_group, "Analysis, filtering, combining, and segmentation of 2D images"}, 
	{pdi_short, "Distance between binary masks."}, 
	{pdi_description, "This program evaluate the average or maximum distance of a mask "
	 "given by a binary image to an image representing a distance map "
	 "and prints the result to stdout. The distance map can be obtained by "
	 "running the filter 'diatance' on a binary image."}, 
	{pdi_example_descr, "Evaluate the maximum distance of mask m.v by using "
	 "the distance field distance.v and scale by factor 2.0. The result is "
	 "written to stdout"}, 
	{pdi_example_code, "-i m.v -d distance.v -s 2.0 -m max"}
}; 

enum EOps {dist_avg, 
	   dist_max, 
	   dist_raw, 
	   dist_unknown
}; 


const TDictMap<EOps>::Table combine_option_table[] = {
	{"avg", dist_avg, "use average"},
	{"max", dist_max, "use maximum" },
	{"raw", dist_raw, "collect raw values as vector" },
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
			  [this](T x){return x * _M_scale;}); 
		return result; 
	}
private:
	double _M_scale; 
};

class CGetDistance: public TFilter<vector<double>> {
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
		vector<double> vresult; 

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
			vresult.push_back(n == 0 ? 0 : result / n); 
			return vresult; 
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
			vresult.push_back(result); 
			return vresult; 
		}
		case dist_raw: {
			while (i != e) {
				if (*i) 
					vresult.push_back(*d); 
				++d; 
				++i; 
			}
			return vresult; 
		}
		default: 
			throw runtime_error("unknown distance measure requested\n"); 
		}
	}
private: 
	const C2DDImage& _M_distance; 
	EOps _M_method; 
};

int do_main( int argc, char *argv[] )
{

	string in_filename;
	string dist_filename;
	string out_filename("-");
	float scale = 1.0; 
	EOps method = dist_avg; 
	
	
	TDictMap<EOps> combine_option(combine_option_table); 
	const auto& imageio = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_general_help);
	options.add(make_opt( in_filename, "in-file", 'i', "input image", 
			      CCmdOptionFlags::required_input, &imageio)); 
	options.add(make_opt( dist_filename, "distance-file", 'd', "distance field image (floating point)", 
			      CCmdOptionFlags::required_input, &imageio)); 
	options.add(make_opt(out_filename, "out-file", 'o', "output file, '-': write to stdout", CCmdOptionFlags::required_output)); 

	options.add(make_opt( scale, "scale", 's', "distance scaling factor")); 
	options.add(make_opt( method, combine_option, "method", 'm', "distance measuring method")); 
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	
	auto in_image = load_image2d(in_filename);
	auto dist_image = load_image2d(dist_filename);
	
	
	Convert2DoubleAndScale create_dist(scale); 
	C2DDImage dist = mia::filter(create_dist, *dist_image); 
	
	CGetDistance get_distance(dist, method); 
	auto result = filter(get_distance, *in_image); 
	if (out_filename == "-") 
		cout << result <<"\n";
	else {
		ofstream out(out_filename.c_str()); 
		out << result; 
		if (!out.good())
			create_exception<runtime_error>("Error writing result to '", out_filename, "'"); 
	}
	return EXIT_SUCCESS; 
}

MIA_MAIN(do_main); 
