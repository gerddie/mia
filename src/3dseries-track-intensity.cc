/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <fstream>
#include <ostream>

#include <mia/core/filetools.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/internal/main.hh>
#include <mia/3d/imageio.hh>

using namespace std;
using namespace mia;

const SProgramDescription g_description = {
	{pdi_group,
	 "Tools for the Analysis of 3D image series"},

	{pdi_short,
	 "Track intensities of pixels in series of 3D images."}, 
	

	{pdi_description,
	 "This program tracks the intensity of a pixel at the given coordinates."}, 

	{pdi_example_descr,
	 "Evaluate the intensity-time curves at points <12,13,14> and <222,113,214>"}, 


	{pdi_example_code,
	 "-i images0000.v -o curves.txt '<12,13,14>' '<222,113,214>'"}
}; 


class FIntensityGetter: public TFilter<void> { 
public: 
	FIntensityGetter(const vector<C3DBounds>& points); 

	template <typename T> 
	void operator () ( const T3DImage<T>& image); 

	bool write(const string& fname) const; 
private: 
	const vector<C3DBounds>& m_points; 
	vector<vector<double> > m_values; 
}; 

FIntensityGetter::FIntensityGetter(const vector<C3DBounds>& points):
	m_points(points)
{
}


template <typename T> 
void FIntensityGetter::operator () ( const T3DImage<T>& image)
{
	vector<double> row(m_points.size()); 
	transform(m_points.begin(), m_points.end(), row.begin(), 
		  [&image](const C3DBounds& x) {return image(x);}); 
	m_values.push_back(row); 
}

bool FIntensityGetter::write(const string& fname) const
{
	ofstream os(fname.c_str()); 
	for (auto row = m_values.begin(); row != m_values.end(); ++row) {
		for (auto v = row->begin(); v != row->end(); ++v) {
			os << *v << " "; 
		}
		os << "\n"; 
	}
	return os.good();
}

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	                        
	CCmdOptionList options(g_description);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
			      "input perfusion data set", CCmdOptionFlags::required_input, &C3DImageIOPluginHandler::instance()));
	options.add(make_opt( out_filename, "out-file", 'o', 
			      "file name for output intensity slopes", CCmdOptionFlags::required_output)); 
	
	
	if (options.parse(argc, argv, "points") != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	
        // create cost function chain
	auto points_descr = options.get_remaining(); 
	if (points_descr.empty())
		throw invalid_argument("No points given - nothing to do"); 

	vector<C3DBounds> points(points_descr.size()); 
	transform(points_descr.begin(), points_descr.end(), points.begin(), 
		  [] (const string& s) -> C3DBounds {
			  istringstream v(s); 
			  C3DBounds result; 
			  v >> result; 
			  return result; 
		  }); 


	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);

	FIntensityGetter intensity_getter(points); 
	for (size_t i = start_filenum; i < end_filenum; ++i) {
		string src_name = create_filename(src_basename.c_str(), i);
		P3DImage image = load_image<P3DImage>(src_name);
		if (!image)
			throw create_exception<runtime_error>( "image ", src_name, " not found");

		cvdebug() << "read '" << src_name << "\n";
		mia::accumulate(intensity_getter,*image); 
	}
	
	return intensity_getter.write(out_filename) ? EXIT_SUCCESS : EXIT_FAILURE;
}

MIA_MAIN(do_main); 

