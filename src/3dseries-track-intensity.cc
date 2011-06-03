/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2010-2011
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
  LatexBeginProgramDescription{Analysis of image series}
  
  \subsection{mia-3dseries-track-intensity}
  \label{mia-3dseries-track-intensity}

  \begin{description} 
  \item [Description:] 
     This program tracks the intensity of a pixel at the given coordinates.  
  
  The program is called like 
  \begin{lstlisting}
mia-3dseries-track-intensity -i <input images> -o out file <point> [<point>] ...
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdgroup{File in- and output} 
  \cmdopt{in-file}{i}{string}{input images}
  \cmdopt{out-file}{o}{string}{output file}
  }
  The points are given as free parameters. 

  \item [Example:]Evaluate the intensity-time curves at points <12,13,14> and <222,113,214>
  \begin{lstlisting}
mia-3dseries-track-intensity -i images0000.v -o curves.txt "<12,13,14>" "<222,113,214>"
  \end{lstlisting}
  \end{description}
  
  LatexEnd
*/

#define VSTREAM_DOMAIN "3dseries-track-intensity"

#include <fstream>
#include <ostream>

#include <mia/core/filetools.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/3d/3dimageio.hh>

using namespace std;
using namespace mia;

//namespace bfs=boost::filesystem; 

const char *g_general_help = 
	"This program tracks the intensity of a pixel at the given coordinates."
  	"Basic usage: \n"
	" mia-3dseries-track-intensity [options] <cost1> <cost2> ..."; 


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

int do_main( int argc, const char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	                        

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;

	CCmdOptionList options(g_general_help);
	
	options.set_group("\nFile-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', 
				    "file name for output intensity slopes")); 
	
	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	
        // create cost function chain
	auto points_descr = options.get_remaining(); 
	if (points_descr.empty())
		throw invalid_argument("No points given - nothing to do"); 

	vector<C3DBounds> points(points_descr.size()); 
	transform(points_descr.begin(), points_descr.end(), points.begin(), 
		  [](const char *s) {
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
			THROW(runtime_error, "image " << src_name << " not found");

		cvdebug() << "read '" << src_name << "\n";
		mia::accumulate(intensity_getter,*image); 
	}
	
	return intensity_getter.write(out_filename) ? EXIT_SUCCESS : EXIT_FAILURE;
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


