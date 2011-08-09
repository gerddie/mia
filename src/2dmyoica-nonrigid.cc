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

  LatexBeginProgramDescription{Myocardial Perfusion Analysis}
  
  \subsection{mia-2dmyoica-nonrigid}
  \label{mia-2dmyoica-nonrigid}

  \begin{description} 
  \item [Description:] 
        This program is used to run a ICA based non-linear registration approach 
        for motion compensation in myocardial perfusion imaging described in Wollny at al. 
        \cite{wollny11bvm}. 
	Specifically, the non-linear transformation is defined in terms of B-splines that 
        is regularized by a DivCurl operator that penalizes the gradients of divergence and 
        curl of the transformation. 
	In addition, the code supports the automatic extraction of a region of interest 
        around the left heart ventricle to speed up computation.
  
  The program is called like 
  \begin{lstlisting}
mia-2dmyoica-nonrigid -i <input set> -o <output set> [options]
  \end{lstlisting}

  \item [Options:] $\:$

  \optiontable{
  \cmdgroup{File in- and output} 
  \cmdopt{in-file}{i}{string}{input segmentation set}
  \cmdopt{out-file}{o}{string}{output segmentation set}
  \cmdopt{registered}{r}{string}{File name base for the registered images. Image type and numbering 
                                 scheme are taken from the input images.}
  \cmdopt{save-cropped}{}{string}{save cropped set to this file, the image files will use the stem of the 
                                 name as file name base}
  \cmdopt{save-feature}{}{string}{save segmentation feature images and initial ICA mixing matrix}
  \cmdopt{save-refs}{}{string}{for each registration pass save the reference images to files with the given name base}
  \cmdopt{save-regs}{}{string}{for each registration pass save intermediate registered images}
				 
  \cmdgroup{Independent component analysis} 
  \cmdopt{components}{C}{int}{Number of  ICA components to be used, 0 = automatic estimation}
  \cmdopt{skip}{k}{int}{Skip a number of frames at the beginning of the series}
  \cmdopt{no-normalize}{}{}{don't normalized ICs}
  \cmdopt{no-meanstrip}{}{}{don't strip the mean from the mixing curves}
  \cmdopt{segscale}{s}{float}{segment and scale the crop box around the LV (0=no segmentation)}
  \cmdopt{max-ica-iter}{m}{int}{maximum number of iterations within ICA}
  \cmdopt{segmethod}{E}{string}{Segmentation method - (delta-feature|delta-peak|features)}

  \cmdgroup{Image registration} 
  \cmdopt{imagecost}{w}{string}{Image similarity measure used (see section \ref{sec:2dfullcost})}
  \cmdopt{optimizer}{O}{string}{Optimizer as provided by the \hyperref[sec:minimizers]{minimizer plug-ins}}
  \cmdopt{mg-levels}{l}{int}{Number of multi-resolution levels to be used for image registration}
  \cmdopt{passes}{P}{int}{Number of ICA+Registration passes to be run}
  \cmdopt{start-c-rate}{a}{float}{start coefficinet rate in spines, gets divided by \texttt{-{}-c-rate-divider} 
                                for each new registration pass}
  \cmdopt{c-rate-divider}{}{float}{cofficient rate divider for each pass}
  \cmdopt{start-divcurl}{d}{float}{start divcurl weight, gets divided by \texttt{-{}-divcurl-divider}
                                for each new registration pass}
  \cmdopt{ivcurl-divider}{}{float}{divcurl weight scaling with each new pass}
  }

  \item [Example:]Register the perfusion series given in segment.set by using automatic ICA estimation. 
        Skip two images at the beginning and otherwiese use the default parameters. 
	Store the result in registered.set. 
  \begin{lstlisting}
mia-2dmyoica-nonrigid  -i segment.set -o registered.set -k 2
  \end{lstlisting}
  \item [See also:] \sa{mia-2dmyomilles}, \sa{mia-2dmyoperiodic-nonrigid}, 
                    \sa{mia-2dmyoserial-nonrigid}, \sa{mia-2dmyopgt-nonrigid},
		    \sa{mia-2dsegseriesstats}
  \end{description}
  
  LatexEnd
*/


#define VSTREAM_DOMAIN "2dmyoica"

#include <fstream>
#include <libxml++/libxml++.h>
#include <itpp/signal/fastica.h>
#include <boost/filesystem.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/factorycmdlineoption.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/minimizer.hh>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/transformfactory.hh>


using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 

const char *g_general_help = 
	"This program runs the non-rigid registration of an perfusion image series.\n"
	"In each pass, first an ICA analysis is run to estimate and eliminate \n" 
	"the periodic movement and create reference images with intensities similar\n"
	"to the corresponding original image. Then non-rigid registration is run \n"
	"using the an \"ssd + divcurl\" cost model. The B-spline c-rate and the \n"
	"divcurl cost weight are changed in each pass according to given parameters.\n"
	"In the first pass a bounding box around the LV myocardium may be extracted\n" 
	"to speed up computation\n\n"
	"Basic usage: \n"
	" mia-2dmyoica-nonrigid [options] "; 


class C2DFImage2PImage {
public: 
	P2DImage operator () (const C2DFImage& image) const {
		return P2DImage(new C2DFImage(image)); 
	}
}; 

class Convert2Float {
public: 
	C2DFImage operator () (P2DImage image) const; 
private: 
	FConvert2DImage2float m_converter; 
}; 

C2DFullCostList create_costs(double divcurlweight, P2DFullCost imagecost)
{
	C2DFullCostList result; 
	result.push(imagecost); 

	stringstream divcurl_descr; 
	divcurl_descr << "divcurl:weight=" << divcurlweight; 
	result.push(C2DFullCostPluginHandler::instance().produce(divcurl_descr.str())); 
	return result; 
}

P2DTransformationFactory create_transform_creator(size_t c_rate)
{
	stringstream transf; 
	transf << "spline:rate=" << c_rate; 
	return C2DTransformCreatorHandler::instance().produce(transf.str()); 
}
	

void segment_and_crop_input(CSegSetWithImages&  input_set, 
			    const C2DPerfusionAnalysis& ica, 
			    float box_scale, 
			    C2DPerfusionAnalysis::EBoxSegmentation segmethod, 
			    C2DImageSeries& references, 
			    const string& save_crop_feature)
{
	C2DBounds crop_start; 
	auto cropper = ica.get_crop_filter(box_scale, crop_start, 
					   segmethod, save_crop_feature); 
	if (!cropper)
		THROW(runtime_error, "Cropping was requested, but segmentation failed"); 
	C2DImageSeries input_images = input_set.get_images(); 
	for(auto i = input_images.begin(); i != input_images.end(); ++i)
		*i = cropper->filter(**i); 
	
	for (auto i = references.begin(); i != references.end(); ++i) 
		*i = cropper->filter(**i); 
	
	auto tr_creator = C2DTransformCreatorHandler::instance().produce("translate");
	P2DTransformation shift = tr_creator->create(C2DBounds(1,1)); 
	auto p = shift->get_parameters(); 
	p[0] = crop_start.x; 
	p[1] = crop_start.y; 
	shift->set_parameters(p); 
	
	input_set.transform(*shift);
	input_set.set_images(input_images);  
}

void run_registration_pass(CSegSetWithImages&  input_set, 
			   const C2DImageSeries& references, 
			   int skip_images, PMinimizer minimizer, size_t mg_levels, 
			   double c_rate, double divcurlweight, P2DFullCost imagecost) 
{
	CSegSetWithImages::Frames& frames = input_set.get_frames();
	C2DImageSeries input_images = input_set.get_images(); 
	auto costs  = create_costs(divcurlweight, imagecost); 
	auto transform_creator = create_transform_creator(c_rate); 
	C2DNonrigidRegister nrr(costs, minimizer,  transform_creator, mg_levels);

	// this loop could be parallized 
	for (size_t i = 0; i < input_images.size() - skip_images; ++i) {
		cvmsg() << "Register frame " << i << "\n"; 
		P2DTransformation transform = nrr.run(input_images[i + skip_images], 
						      references[i]);
		input_images[i + skip_images] = 
			(*transform)(*input_images[i + skip_images]);
		frames[i + skip_images].inv_transform(*transform);
	}
	input_set.set_images(input_images); 
}

void save_references(const string& save_ref, int current_pass, int skip_images, const C2DImageSeries& references)
{
	for(size_t i = 0 ; i < references.size(); ++i) {
		stringstream filename; 
		filename << save_ref << current_pass << "-" 
			 << setw(4) << setfill('0') << skip_images + i << ".v"; 
		save_image(filename.str(), references[i]);
	}
}

int do_main( int argc, const char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase("reg");

	// debug options: save some intermediate steps 
	string cropped_filename;
	string save_crop_feature; 
	string save_ref_filename;
	string save_reg_filename;

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;

	// registration parameters
	auto minimizer = CMinimizerPluginHandler::instance().produce("gsl:opt=gd,step=0.1");
	auto imagecost = C2DFullCostPluginHandler::instance().produce("image:weight=1,cost=ssd");
	double c_rate = 32; 
	double c_rate_divider = 4; 
	double divcurlweight = 20.0; 
	double divcurlweight_divider = 4.0; 

	size_t mg_levels = 3; 

	// ICA parameters 
	size_t components = 0;
	bool no_normalize = false; 
	bool no_meanstrip = false; 
	float box_scale = 0.0;
	size_t skip_images = 0; 
	size_t max_ica_iterations = 400; 
	C2DPerfusionAnalysis::EBoxSegmentation 
		segmethod=C2DPerfusionAnalysis::bs_features; 

	size_t current_pass = 0; 
	size_t pass = 3; 

	CCmdOptionList options(g_general_help);
	
	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
				    "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', 
				    "output perfusion data set", CCmdOption::required));
	options.add(make_opt( registered_filebase, "registered", 'r', 
				    "file name base for registered fiels")); 
	
	options.add(make_opt( cropped_filename, "save-cropped", 0, 
				    "save cropped set to this file", NULL)); 
	options.add(make_opt( save_crop_feature, "save-feature", 0, 
				    "save segmentation feature images"
				    " and initial ICA mixing matrix", NULL)); 

	options.add(make_opt( save_ref_filename, "save-refs", 0, 
				    "save reference images", NULL)); 
	options.add(make_opt( save_reg_filename, "save-regs", 0, 
				    "save intermediate registered images", NULL)); 


	
	options.set_group("Registration"); 
	options.add(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.add(make_opt( c_rate, "start-c-rate", 'a', 
				    "start coefficinet rate in spines,"
				    " gets divided by --c-rate-divider with every pass"));
	options.add(make_opt( c_rate_divider, "c-rate-divider", 0, 
				    "cofficient rate divider for each pass"));
	options.add(make_opt( divcurlweight, "start-divcurl", 'd',
				    "start divcurl weight, gets divided by"
				    " --divcurl-divider with every pass")); 
	options.add(make_opt( divcurlweight_divider, "divcurl-divider", 0,
				    "divcurl weight scaling with each new pass")); 
	options.add(make_opt( imagecost, "imagecost", 'w', "image cost")); 
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( pass, "passes", 'P', "registration passes")); 

	options.set_group("ICA"); 
	options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation", NULL));
	options.add(make_opt( no_normalize, "no-normalize", 0, "don't normalized ICs", NULL));
	options.add(make_opt( no_meanstrip, "no-meanstrip", 0, 
				    "don't strip the mean from the mixing curves", NULL));
	options.add(make_opt( box_scale, "segscale", 's', 
				    "segment and scale the crop box around the LV (0=no segmentation)", "segscale"));
	options.add(make_opt( skip_images, "skip", 'k', "skip images at the beginning of the series "
				    "e.g. because as they are of other modalities")); 
	options.add(make_opt( max_ica_iterations, "max-ica-iter", 'm', "maximum number of iterations in ICA")); 

	options.add(make_opt(segmethod , C2DPerfusionAnalysis::segmethod_dict, "segmethod", 'E', 
				   "Segmentation method")); 

	if (options.parse(argc, argv, false) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries input_images = input_set.get_images(); 
	
	cvmsg() << "skipping " << skip_images << " images\n"; 
	vector<C2DFImage> series(input_images.size() - skip_images); 
	transform(input_images.begin() + skip_images, input_images.end(), 
		  series.begin(), Convert2Float()); 
	

	// run ICA
	C2DPerfusionAnalysis ica(components, !no_normalize, !no_meanstrip); 
	if (max_ica_iterations) 
		ica.set_max_ica_iterations(max_ica_iterations); 
	if (!ica.run(series)) {
		ica.set_approach(FICA_APPROACH_SYMM); 
		if (!ica.run(series))
			cvwarn() << "ICA not converged, but the SYMM approach has given something to work with ...\n";
	}
	vector<C2DFImage> references_float = ica.get_references(); 
	
	C2DImageSeries references(references_float.size()); 
	transform(references_float.begin(), references_float.end(), references.begin(), C2DFImage2PImage()); 

	// crop if requested
	if (box_scale) {
		segment_and_crop_input(input_set, ica, box_scale, segmethod, references, save_crop_feature); 
		input_images = input_set.get_images(); 
	}else if (!save_crop_feature.empty()) {
		stringstream cfile; 
		cfile << save_crop_feature << ".txt"; 
		ica.save_coefs(cfile.str()); 
	}

	// save cropped images if requested
	if (!cropped_filename.empty()) {
		bfs::path cf(cropped_filename);
		cf.replace_extension(); 
		input_set.rename_base(__bfs_get_filename(cf)); 
		input_set.save_images(cropped_filename);

		unique_ptr<xmlpp::Document> test_cropset(input_set.write());
		ofstream outfile(cropped_filename, ios_base::out );
		if (outfile.good())
			outfile << test_cropset->write_to_string_formatted();
		else 
			THROW(runtime_error, "unable to save to '" << cropped_filename << "'"); 

	}

	bool do_continue=true; 
	do {
		++current_pass; 
		cvmsg() << "Registration pass " << current_pass << "\n"; 

		if (!save_ref_filename.empty())
			save_references(save_ref_filename, current_pass, skip_images, references); 

		run_registration_pass(input_set, references,  skip_images,  minimizer, 
				      mg_levels, c_rate, divcurlweight, imagecost); 
		
		if (!save_reg_filename.empty()) 
			save_references(save_reg_filename, current_pass, 0, input_set.get_images()); 

		C2DPerfusionAnalysis ica2(components, !no_normalize, !no_meanstrip); 
		if (max_ica_iterations) 
			ica2.set_max_ica_iterations(max_ica_iterations); 
	
		transform(input_set.get_images().begin() + skip_images, 
			  input_set.get_images().end(), series.begin(), Convert2Float()); 

		if (!ica2.run(series))
			ica2.set_approach(FICA_APPROACH_SYMM); 

		ica2.run(series); 
		divcurlweight /= divcurlweight_divider; 
		if (c_rate > 1) 
			c_rate /= c_rate_divider; 
		references_float = ica2.get_references(); 
		transform(references_float.begin(), references_float.end(), 
			  references.begin(), C2DFImage2PImage()); 
		do_continue =  (!pass || current_pass < pass) && ica2.has_periodic(); 
	} while (do_continue); 

	input_set.rename_base(registered_filebase); 
	input_set.save_images(out_filename); 
	
	unique_ptr<xmlpp::Document> outset(input_set.write());
	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();
	
	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;

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

inline C2DFImage Convert2Float::operator () (P2DImage image) const
{
	return ::mia::filter(m_converter, *image); 
}
