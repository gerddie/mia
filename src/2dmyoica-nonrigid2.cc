/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
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
	"to speed up computation\n"
	"Special note to this implemnentation: the registration is always run from the "
	"original images to avoid the accumulation of interpolation errors.\n\n"
	"Basic usage: \n"
	" mia-2dnonrigidreg [options] "; 


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

C2DFullCostList create_costs(double divcurlweight, double imageweight)
{
	C2DFullCostList result; 
	stringstream divcurl_descr;  
	divcurl_descr << "divcurl:weight=" << divcurlweight; 
	result.push(C2DFullCostPluginHandler::instance().produce(divcurl_descr.str())); 

	stringstream image_descr; 
	image_descr << "image:weight=" << imageweight; 
	result.push(C2DFullCostPluginHandler::instance().produce(image_descr.str())); 
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

vector<P2DTransformation> 
run_registration_pass(CSegSetWithImages&  input_set, 
		      C2DImageSeries& registered,
		      const C2DImageSeries& references, 
		      int skip_images, PMinimizer minimizer, 
		      C2DInterpolatorFactory& ipfactory, size_t mg_levels, 
			   double c_rate, double divcurlweight, double imageweight) 
{
	vector<P2DTransformation> result; 
	C2DImageSeries input_images = input_set.get_images(); 
	registered.resize(input_images.size()); 
	auto costs  = create_costs(divcurlweight, imageweight); 
	auto transform_creator = create_transform_creator(c_rate); 
	C2DNonrigidRegister nrr(costs, minimizer,  transform_creator, 
				ipfactory, mg_levels);

	// this loop could be parallized 
	for (size_t i = skip_images; i < input_images.size(); ++i) {
		cvmsg() << "Register frame " << i << "\n"; 
		P2DTransformation transform = nrr.run(input_images[i], 
						      references[i - skip_images]);
		registered[i] = (*transform)(*input_images[i], ipfactory);
		result.push_back(transform); 
	}
	return result; 
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

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;

	// registration parameters
	auto minimizer = CMinimizerPluginHandler::instance().produce("gsl:opt=gd,step=0.1");
	double c_rate = 32; 
	double c_rate_divider = 4; 
	double divcurlweight = 20.0; 
	double divcurlweight_divider = 4.0; 
	double imageweight = 1.0; 

	EInterpolation interpolator = ip_bspline3;
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
	
	options.set_group("\nFile-IO"); 
	options.push_back(make_opt( in_filename, "in-file", 'i', 
				    "input perfusion data set", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o', 
				    "output perfusion data set", CCmdOption::required));
	options.push_back(make_opt( registered_filebase, "registered", 'r', 
				    "file name base for registered fiels")); 
	
	options.push_back(make_opt( cropped_filename, "save-cropped", 0, 
				    "save cropped set to this file", NULL)); 
	options.push_back(make_opt( save_crop_feature, "save-feature", 0, 
				    "save segmentation feature images"
				    " and initial ICA mixing matrix", NULL)); 

	
	options.set_group("\nRegistration"); 
	options.push_back(make_opt( minimizer, "optimizer", 'O', "Optimizer used for minimization"));
	options.push_back(make_opt( c_rate, "start-c-rate", 'a', 
				    "start coefficinet rate in spines,"
				    " gets divided by --c-rate-divider with every pass"));
	options.push_back(make_opt( c_rate_divider, "c-rate-divider", 0, 
				    "cofficient rate divider for each pass"));
	options.push_back(make_opt( divcurlweight, "start-divcurl", 'd',
				    "start divcurl weight, gets divided by"
				    " --divcurl-divider with every pass")); 
	options.push_back(make_opt( divcurlweight_divider, "divcurl-divider", 0,
				    "divcurl weight scaling with each new pass")); 
	options.push_back(make_opt( imageweight, "imageweight", 'w', 
				    "image cost weight")); 
	options.push_back(make_opt( interpolator, GInterpolatorTable ,"interpolator", 'p',
				    "image interpolator", NULL));
	options.push_back(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.push_back(make_opt( pass, "passes", 'P', "registration passes")); 

	options.set_group("\nICA"); 
	options.push_back(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation", NULL));
	options.push_back(make_opt( no_normalize, "no-normalize", 0, "don't normalized ICs", NULL));
	options.push_back(make_opt( no_meanstrip, "no-meanstrip", 0, 
				    "don't strip the mean from the mixing curves", NULL));
	options.push_back(make_opt( box_scale, "segscale", 's', 
				    "segment and scale the crop box around the LV (0=no segmentation)", "segscale"));
	options.push_back(make_opt( skip_images, "skip", 'k', "skip images at the beginning of the series "
				    "e.g. because as they are of other modalities")); 
	options.push_back(make_opt( max_ica_iterations, "max-ica-iter", 'm', "maximum number of iterations in ICA")); 

	options.push_back(make_opt(segmethod , C2DPerfusionAnalysis::segmethod_dict, "segmethod", 'E', 
				   "Segmentation method")); 
				    

	options.parse(argc, argv, false);



	// this cost will always be used 

	unique_ptr<C2DInterpolatorFactory>   ipfactory(create_2dinterpolation_factory(interpolator));

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
	vector<P2DTransformation> transformations; 
	C2DImageSeries registered;

	bool do_continue = ica.has_periodic(); 
	while (do_continue){
		++current_pass; 
		cvmsg() << "Registration pass " << current_pass << "\n"; 
		transformations = 
			run_registration_pass(input_set, registered, references,  skip_images,  minimizer, 
					      *ipfactory, mg_levels, c_rate, divcurlweight, imageweight); 
		
		C2DPerfusionAnalysis ica2(components, !no_normalize, !no_meanstrip); 
		if (max_ica_iterations) 
			ica2.set_max_ica_iterations(max_ica_iterations); 
	
		transform(registered.begin() + skip_images, 
			  registered.end(), series.begin(), Convert2Float()); 

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
	}

	CSegSetWithImages::Frames& frames = input_set.get_frames();
	for (size_t i = skip_images; i < input_images.size(); ++i) 
		frames[i].inv_transform(*transformations[i-skip_images]);

	C2DImageSeries iimages = input_set.get_images(); 
	copy(iimages.begin(), iimages.begin() + skip_images, registered.begin());  

	input_set.set_images(registered); 
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
