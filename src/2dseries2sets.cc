/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
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

  LatexBeginProgramDescription{Myocardial Perfusion Analysis}
  
  \subsection{mia-2dseries2sets}
  \label{mia-2dseries2sets}

  \begin{description} 
  \item [Description:] 
     This program takes all image files that are given as free parameters on the command line 
     and creates segmentation sets based on information found in the images. 
     Used information is the z-location of the slice and the acquisition number. 
     The code is taylored to used the according descriptors defined in the DICOM standard.
     All images with the same slice location will be grouped together in one segmentation 
     set and ordered according to their aquisition number.
     Slice locations are rounded to three digits accuracy to make proper comparison 
       of floating point values feasable. 

  The program is called like 
  \begin{lstlisting}
mia-2dseries2sets -o <output directory> [options] <image> [<image>] ...
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdopt{out}{o}{string}{Output directory, must exist and be writable.}
  \cmdopt{no-copy}{}{}{Don't copy the images to the output directory. 
                       In this case the created segmentation sets will reference the original files}
  }
  The input images are given as free parameters on the command line. 
  The segmentation sets are named segmentX.set with X starting a zero and larger values correspondent 
    to larger values in the slice location.
  If no acquisition numbers can be found in the images then the order in which the files are given at the 
    command line is used. 
  If no slice location can be found, a value of 0.0 will be used. 

  \item [Example:]Create the segmentation sets from a series of DICOM images and copy the files to 
                  the output directory. 
  \begin{lstlisting}
mia-2dseries2sets -i /home/user/series /net/dicoms/patient1/series1/*.dcm 
  \end{lstlisting}
  \end{description}
  
  LatexEnd
*/

#define VSTREAM_DOMAIN "series2set" 
#include <fstream>
#include <libxml++/libxml++.h>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <boost/filesystem.hpp>


namespace bfs=boost::filesystem; 
using namespace std; 
NS_MIA_USE; 

const char *g_description = 
	"This program is used to create per slice time series sets from " 
	"2D perfusion imaging data. The input files are given as additional "
	"parameters on the command line."
	; 



typedef pair<P2DImage, string> SImage; 
typedef vector<SImage> C2DImageVectorWithName; 

vector<C2DImageVectorWithName> separate_slices(const C2DImageVectorWithName &images)
{
	// collect series 
	// \todo maybe one should also look for SeriesNumber
	typedef map<int, SImage> AquisitionSeries; 
	map<float, AquisitionSeries> series; 
	int aq_number = 0; 
	for (auto i = images.begin(); i != images.end(); ++i) {
		float slice_location = 0.0;
		
		auto pslice_location = dynamic_cast<const CFloatAttribute *>(i->first->get_attribute(IDSliceLocation).get());
		if (pslice_location) {
			// round the location ,because we want to compare it 
			slice_location = floor(1000.0 * *pslice_location) / 1000.0; 
		}
		if (series.find(slice_location) == series.end()) {
			cvmsg() << "Add location " << slice_location << "\n"; 
			series[slice_location] = AquisitionSeries(); 
		}
		
		AquisitionSeries& aqs = series[slice_location]; 
		
		auto pAcquisitionNumber = dynamic_cast<const CIntAttribute *>(i->first->get_attribute(IDAcquisitionNumber).get());
		if (pAcquisitionNumber) {
			aq_number = *pAcquisitionNumber; 
		}else {
			++aq_number; 
		}
		cvmsg() << "Add aquisition " << aq_number << "\n"; 
		if (aqs.find(aq_number) != aqs.end()) {
			cverr() << "Will ignore  '" << i->second 
				<< "' because '" << aqs.find(aq_number)->second.second  
				<< "' has same aquisition number and slice location\n";
		}else
			aqs[aq_number] = *i; 
	}
	// copy series to output vectors
	vector<C2DImageVectorWithName> result; 
	for (auto loc = series.begin(); loc != series.end(); ++loc) {
		C2DImageVectorWithName aqseries; 
		for (auto slice = loc->second.begin(); slice != loc->second.end(); ++slice)
			aqseries.push_back(slice->second);
		result.push_back(aqseries); 
	}
	return result; 
}

void mia_copy_file(const bfs::path& infile, const bfs::path& outfile) 
{
	ifstream ifs(infile.string().c_str(), ios::in  | ios::binary);
	ofstream ofs(outfile.string().c_str(),ios::out | ios::binary);
	ofs << ifs.rdbuf();
}

bool save_series(int index, const C2DImageVectorWithName& series, const string& out_directory, bool no_copy_files) 
{

	// create segmentation set 
	bfs::path outpath(out_directory); 
	
	CSegSetWithImages set; 
	for (auto i = series.begin(); i != series.end(); ++i) {
		CSegFrame frame; 
		if (no_copy_files) 
			frame.set_imagename(i->second);
		else {
			bfs::path infile(i->second); 
			string filename = __bfs_get_filename(infile);
			frame.set_imagename(filename);
			mia_copy_file(infile, outpath / bfs::path(filename)); 
		}
		set.add_frame(frame);
	}
	stringstream fname; 
	fname << "segment" << index << ".set"; 
	
	bfs::path outfilename = bfs::path(out_directory) / bfs::path(fname.str()); 
	
	unique_ptr<xmlpp::Document> outset(set.write());
	ofstream outfile(outfilename.string().c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();
	else 
		cverr() << "Unable to open file '" << outfilename.string() << "'\n"; 
	return outfile.good();
}

int do_main( int argc, const char *argv[] )
{
	string out_directory; 
	bool no_copy_images = false; 
	
	CCmdOptionList options(g_description);
	options.add(make_opt( out_directory, "out", 'o', "output directory (needs to exist and be writable)", 
				    CCmdOption::required));
	options.add(make_opt( no_copy_images, "no-copy", 0, "don't copy image files to output directory"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	auto input_files =  options.get_remaining();
	if (input_files.empty()) {
		THROW(invalid_argument, "no input files given"); 
	}

	C2DImageVectorWithName images;
	for (auto i = input_files.begin(); i != input_files.end(); ++i) {
		SImage simage; 
		simage.second = string(*i); 
		auto file_images = C2DImageIOPluginHandler::instance().load(*i);
		if (file_images && !file_images->empty()) {
			for (auto k = file_images->begin(); k != file_images->end(); ++k) {
				simage.first = *k; 
				images.push_back(simage); 
			}
		}
	}
	
	// now read the image attributes to sort the series 
	vector<C2DImageVectorWithName> sliced_series = separate_slices(images); 
	bool success = true; 
	for (size_t  i = 0; i < sliced_series.size(); ++i) {
		success &= save_series(i, sliced_series[i], out_directory, no_copy_images); 
		if (success) 
			cvmsg() << "Wrote set " << i << "\n"; 
		else 
			cverr() << "unable to save set " << i << "\n"; 
	}
	
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
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

