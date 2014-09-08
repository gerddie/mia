/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#define VSTREAM_DOMAIN "2dmyoica-full"

#include <fstream>
#include <itpp/signal/fastica.h>

#include <mia/core/msgstream.hh>
#include <mia/core/threadedmsg.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/minimizer.hh>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/core/attribute_names.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/segsetwithimages.hh>
#include <mia/2d/transformfactory.hh>

#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
using namespace tbb;

using namespace std;
using namespace mia;

namespace bfs=boost::filesystem; 

const SProgramDescription g_description = {

	{pdi_group, "Registration of series of 2D images"}, 
	{pdi_short, "Run a registration of a series of 2D images."}, 
	{pdi_description, "This program implements the 2D version of the motion compensation algorithm "
	 "described in Wollny G, Kellman P, Santos A, Ledesma-Carbayo M-J, \"Automatic Motion "
	 "Compensation of Free Breathing acquired Myocardial Perfusion Data by using Independent "
	 "Component Analysis\", Medical Image Analysis, 2012, DOI:10.1016/j.media.2012.02.004. "
         "The software may first run a linear registration and then a non-linear registration or "
         "just one of the both." 
	 "This version of the program may run all registrations in parallel."}, 

	{pdi_example_descr, "Register the perfusion series given in 'segment.set' by using automatic "
	 "ICA estimation. Skip two images at the beginning and otherwiese use the default parameters. "
	 "Store the result in 'registered.set'."}, 

	{pdi_example_code, "  -i segment.set -o registered.set -k 2"}
}; 


C2DFullCostList create_costs(const string& imagecostbase, int idx)
{
	stringstream cost_descr; 
	cost_descr << imagecostbase << ",src=src" << idx << ".@,ref=ref" << idx << ".@"; 
	auto imagecost = C2DFullCostPluginHandler::instance().produce(cost_descr.str()); 

	C2DFullCostList result; 
	result.push(imagecost); 
	return result; 
}

P2DTransformationFactory create_spline_transform_creator(size_t c_rate, double divcurlweight)
{
	stringstream transf; 
	transf << "spline:rate=" << c_rate << ",imgboundary=mirror,imgkernel=[bspline:d=3]"
	       << ",penalty=[divcurl:weight=" << divcurlweight << "]"; 
	return C2DTransformCreatorHandler::instance().produce(transf.str()); 
}

C2DBounds segment_and_crop_input(CSegSetWithImages&  input_set, 
			    const C2DPerfusionAnalysis& ica, 
			    float box_scale, 
			    C2DPerfusionAnalysis::EBoxSegmentation segmethod, 
			    C2DImageSeries& references, 
			    const string& save_crop_feature)
{
	C2DBounds crop_start = C2DBounds::_0; 
	auto cropper = ica.get_crop_filter(box_scale, crop_start, 
					   segmethod, save_crop_feature); 
	if (!cropper) {
		cvwarn() << "Cropping was requested, but segmentation failed - continuing at full image size\n";
		return crop_start; 
	}
	
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
        return crop_start; 
}


struct SeriesRegistration {
	C2DImageSeries& input_images; 
	CSegSetWithImages::Frames& frames;
	const C2DImageSeries& references; 
	string minimizer; 
	size_t mg_levels; 
	P2DTransformationFactory transform_creator; 
	string imagecostbase; 
	int skip_images; 
        int global_reference; 
	
	SeriesRegistration(C2DImageSeries& _input_images, 
			   CSegSetWithImages::Frames& _frames,
			   const C2DImageSeries& _references, 
			   const string& _minimizer, 
			   size_t _mg_levels, 
			   P2DTransformationFactory _transform_creator, 
			   string _imagecostbase, 
			   int _skip_images, 
                           int _global_reference):
		input_images(_input_images), 
		frames(_frames), 
		references(_references), 
		minimizer(_minimizer), 
		mg_levels(_mg_levels), 
                transform_creator(_transform_creator), 
		imagecostbase(_imagecostbase), 
		skip_images(_skip_images), 
                global_reference(_global_reference)
		{
		}
	P2DTransformation operator()( const blocked_range<int>& range, P2DTransformation init) const {
		CThreadMsgStream thread_stream;
		TRACE_FUNCTION; 
                P2DTransformation result = init; 
                
		auto m =  CMinimizerPluginHandler::instance().produce(minimizer);
		for( int i=range.begin(); i!=range.end(); ++i ) {
			auto costs  = create_costs(imagecostbase, i); 
			C2DNonrigidRegister nrr(costs, m,  transform_creator, mg_levels, i);
                        if (i + skip_images != global_reference) {
				cvmsg() << "image size ["<< i << "]= " << input_images[i + skip_images]->get_size() << ":" << references[i]->get_size() << "\n"; 
                                P2DTransformation transform = nrr.run(input_images[i + skip_images], references[i]);
                                input_images[i + skip_images] = (*transform)(*input_images[i + skip_images]);
                                frames[i + skip_images].inv_transform(*transform);
                        }else {
                                result = nrr.run(references[i], input_images[i + skip_images]);
                        }
		}
                return result; 
	}
};  


void run_registration_pass(CSegSetWithImages& input_set, 
			   const C2DImageSeries& references,  
			   int skip_images,  const string& minimizer, 
			   size_t mg_levels, P2DTransformationFactory transform_creator, 
			   const string&   imagecost, int global_reference) 
{
	C2DImageSeries input_images = input_set.get_images(); 
	CSegSetWithImages::Frames& frames = input_set.get_frames();
	
	
	SeriesRegistration sreg(input_images, frames, references, minimizer, 
				mg_levels, transform_creator, 
				imagecost, skip_images, global_reference); 
        
        P2DTransformation init; 
	P2DTransformation inv_transf = parallel_reduce(blocked_range<int>( 0, references.size()), init, sreg, 
                                                       [](P2DTransformation a, P2DTransformation b) {
                                                               if (a) 
                                                                       return a; 
                                                               return b; 
                                                       });

        // apply inverse to all images 
        if (inv_transf) {
		cvmsg() << "Apply inverse for reference correction\n"; 
                const C2DTransformation& inv_transf_ref = * inv_transf; 
                parallel_for(blocked_range<int>( 0, references.size()), 
                             [&inv_transf_ref, &frames, skip_images, global_reference, &input_images](const blocked_range<int>& range){
				     CThreadMsgStream thread_stream;
                                     for( int i=range.begin(); i!=range.end(); ++i ) {
                                             if (i != global_reference - skip_images) {
                                                     input_images[i + skip_images] = inv_transf_ref(*input_images[i + skip_images]);
                                                     frames[i + skip_images].inv_transform(inv_transf_ref);
                                             }
                                     }
                             });
        }
        input_set.set_images(input_images);
}

void run_nonlinear_registration_passes (CSegSetWithImages& input_set, 
                                        C2DImageSeries& references,  
					int components, bool normalize, bool no_meanstrip, int max_ica_iterations, 
					int skip_images,
                                        const string& minimizer, 
                                        size_t mg_levels, double c_rate, double c_rate_divider, 
                                        double divcurlweight, double divcurlweight_divider, 
                                        int max_pass, const string& imagecost, int global_reference, float min_rel_frequency)
{
        int current_pass = 0; 
	bool do_continue=true; 
	bool lastpass = false; 
	vector<C2DFImage> references_float; 
	do {
		++current_pass; 
		cvmsg() << "Registration pass " << current_pass << "\n"; 
                
                auto transform_creator = create_spline_transform_creator(c_rate, divcurlweight); 


                run_registration_pass(input_set, references,  
                                      skip_images,  minimizer, mg_levels, transform_creator, 
                                      imagecost, global_reference); 

		if (lastpass) 
			break; 
                
		C2DPerfusionAnalysis ica2(components, normalize, !no_meanstrip); 
		if (max_ica_iterations) 
			ica2.set_max_ica_iterations(max_ica_iterations); 
		if (min_rel_frequency >= 0)
			ica2.set_min_movement_frequency(min_rel_frequency); 
	
                vector<C2DFImage> series(input_set.get_images().size() - skip_images); 
		transform(input_set.get_images().begin() + skip_images, 
			  input_set.get_images().end(), series.begin(), FCopy2DImageToFloatRepn()); 

		if (!ica2.run(series)) {
			ica2.set_approach(FICA_APPROACH_SYMM); 
			ica2.run(series); 
		}
		
		divcurlweight /= divcurlweight_divider; 
		if (c_rate > 1) 
			c_rate /= c_rate_divider; 
		references_float = ica2.get_references(); 
		cvmsg() << "references_float size:" << references_float[0].get_size() << "\n"; 
		transform(references_float.begin(), references_float.end(), 
			  references.begin(), FWrapStaticDataInSharedPointer<C2DImage>()); 
		do_continue =  (!max_pass || current_pass < max_pass) && ica2.has_movement(); 
		
		// run one more pass if the limit is not reached and no movement identified
		lastpass = (!do_continue && (!max_pass || current_pass < max_pass)); 
		
	} while (do_continue || lastpass); 
        
}

void run_linear_registration_passes (CSegSetWithImages& input_set, 
                                     C2DImageSeries& references,  
                                     int components, bool normalize, bool no_meanstrip, int max_ica_iterations, 
                                     int skip_images,  const string& minimizer, const string& linear_transform, 
                                     size_t mg_levels, int max_pass, const string& imagecost, int global_reference, 
				     float min_rel_frequency)
{
        int current_pass = 0; 
	bool do_continue=true; 
	bool lastpass = false; 
	vector<C2DFImage> references_float; 
	do {
		++current_pass; 
		cvmsg() << "Registration pass " << current_pass << "\n"; 
                
                auto transform_creator = C2DTransformCreatorHandler::instance().produce(linear_transform); 

		cvmsg() << "references_float size:" << references[0]->get_size() << "\n"; 
                run_registration_pass(input_set, references,  
                                      skip_images,  minimizer, mg_levels, transform_creator, 
                                      imagecost, global_reference); 

		if (lastpass) 
			break; 
			
                
		C2DPerfusionAnalysis ica2(components, normalize, !no_meanstrip); 
		if (max_ica_iterations) 
			ica2.set_max_ica_iterations(max_ica_iterations); 
		if (min_rel_frequency >= 0)
			ica2.set_min_movement_frequency(min_rel_frequency); 
	
                vector<C2DFImage> series(input_set.get_images().size() - skip_images); 
		transform(input_set.get_images().begin() + skip_images, 
			  input_set.get_images().end(), series.begin(), FCopy2DImageToFloatRepn()); 

		if (!ica2.run(series)) {
			ica2.set_approach(FICA_APPROACH_SYMM); 
			ica2.run(series); 
		}
		
		references_float = ica2.get_references(); 
		transform(references_float.begin(), references_float.end(), 
			  references.begin(), FWrapStaticDataInSharedPointer<C2DImage>()); 

		
		cvmsg() << "references_float size:" << references[0]->get_size() << "\n"; 
		do_continue =  (!max_pass || current_pass < max_pass) && ica2.has_movement(); 
		
		// run one more pass if the limit is not reached and no movement identified
		lastpass = (!do_continue && (!max_pass || current_pass < max_pass)); 
		
	} while (do_continue || lastpass); 
        
}

class FInsertData : public TFilter< P2DImage >  {
public: 
	FInsertData(const C2DBounds& start, const C2DBounds& end): 
		m_start(start), m_end(end){}
	
	template <typename T>
	void operator () ( const T2DImage<T>& a, T2DImage<T>& b) const {
		auto ia = a.begin(); 
		auto ea = a.end(); 
		auto ib = b.begin_range(m_start,m_end); 
		while (ia != ea) {
			*ib = *ia; 
			++ia; 
			++ib; 
		}

	}
private: 
		
	C2DBounds m_start; 
	C2DBounds m_end; 
}; 

float get_relative_min_breathing_frequency(const C2DImageSeries& images, int skip, float min_breathing_frequency)
{
	if (min_breathing_frequency < 0) 
		return -1; 
	if (min_breathing_frequency == 0) 
		return 0; 
	int n_heartbeats = images.size() - skip; 
	auto image_begin =  images[skip]; 
	auto image_end = images[images.size() - 1]; 

	if (image_begin->has_attribute("AcquisitionTime") && image_end->has_attribute(IDAcquisitionTime)) {
		double aq_time = image_end->get_attribute_as<double>(IDAcquisitionTime) - 
			image_begin->get_attribute_as<double>(IDAcquisitionTime);
		if (aq_time < 0) 
			throw create_exception<runtime_error>("Got non-postive aquisition time range ", aq_time, 
							      ", can't handle this");  
							      
		double heart_rate = 60 * n_heartbeats / aq_time; 
		cvmsg() << "Read a heartbeat rate of " << heart_rate << " beats/min\n";
		return heart_rate / min_breathing_frequency; 
	}else 
		return -1; 
}

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename;
	string registered_filebase;

	// debug options: save some intermediate steps 
	string cropped_filename;
	string save_crop_feature; 
	string save_ref_filename;
	string save_reg_filename;

	// non-linear registration parameters
	string linear_minimizer("gsl:opt=simplex,step=1.0");
	string nonlinear_minimizer("gsl:opt=gd,step=0.1");
	string imagecost("image:weight=1,cost=ssd");
	double c_rate = 16; 
	double c_rate_divider = 2; 
	double divcurlweight = 10000.0; 
	double divcurlweight_divider = 2.0;

        string linear_transform("affine");

	size_t mg_levels = 3; 

	// ICA parameters 
	size_t components = 0;
	bool normalize = false; 
	bool no_meanstrip = false; 
	float box_scale = 0.0;
	size_t skip_images = 0; 
	size_t max_ica_iterations = 400; 
	C2DPerfusionAnalysis::EBoxSegmentation 
		segmethod=C2DPerfusionAnalysis::bs_features; 

	float min_breathing_frequency = -1.0f; 

	size_t max_linear_passes = 3; 
	size_t max_nonlinear_passes = 3; 
        int global_reference = -1; 

	CCmdOptionList options(g_description);
	
	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', 
			      "input perfusion data set", CCmdOptionFlags::required_input));
	options.add(make_opt( out_filename, "out-file", 'o', 
			      "output perfusion data set", CCmdOptionFlags::required_output));
	options.add(make_opt( registered_filebase, "registered", 'r', 
			      "File name base for the registered images. Image type and numbering "
			      "scheme are taken from the input images as given in the input data set.")); 
	
	options.add(make_opt( cropped_filename, "save-cropped", 0, 
			      "save cropped set to this file, the image files will use the stem of the "
			      "name as file name base", CCmdOptionFlags::output)); 
	options.add(make_opt( save_crop_feature, "save-feature", 0, 
			      "save segmentation feature images and initial ICA mixing matrix", CCmdOptionFlags::output)); 
	
	options.add(make_opt( save_ref_filename, "save-refs", 0, 
			      "for each registration pass save the reference images to files with the given name base", 
			      CCmdOptionFlags::output
			    )); 
	options.add(make_opt( save_reg_filename, "save-regs", 0, 
			      "for each registration pass save intermediate registered images", CCmdOptionFlags::output)); 


	
	options.set_group("Registration"); 
	options.add(make_opt(linear_minimizer, "linear-optimizer", 'L', 
                             "Optimizer used for minimization of the linear registration", 
                             CCmdOptionFlags::none, &CMinimizerPluginHandler::instance()));
	options.add(make_opt(linear_transform, "linear-transform", 0, 
                             "linear transform to be used", 
                             CCmdOptionFlags::none, &C2DTransformCreatorHandler::instance()));
        
        options.add(make_opt(nonlinear_minimizer, "non-linear-optimizer", 'O', 
                             "Optimizer used for minimization in the non-linear registration.", 
                             CCmdOptionFlags::none, &CMinimizerPluginHandler::instance()));
	options.add(make_opt( c_rate, "start-c-rate", 'a', 
                              "start coefficinet rate in spines,"
                              " gets divided by --c-rate-divider with every pass."));
	options.add(make_opt( c_rate_divider, "c-rate-divider", 0, 
                              "Cofficient rate divider for each pass."));
	options.add(make_opt( divcurlweight, "start-divcurl", 'd',
                              "Start divcurl weight, gets divided by"
                              " --divcurl-divider with every pass.")); 
	options.add(make_opt( divcurlweight_divider, "divcurl-divider", 0,
                              "Divcurl weight scaling with each new pass.")); 
	options.add(make_opt( global_reference, "reference", 'R', "Global reference all image should be aligned to. If set "
			      "to a non-negative value, the images will be aligned to this references, and the cropped "
                              "output image date will be injected into the original images. Leave at -1 if "
			      "you don't care. In this case all images with be registered to a mean position of the movement")); 
        
	// why do I allow to set this parameter, it should always be image:cost=ssd  
	options.add(make_opt( imagecost, "imagecost", 'w',
			      "image cost, do not specify the src and ref parameters, these will be set by the program.",
			      CCmdOptionFlags::none, &C2DFullCostPluginHandler::instance())); 
	options.add(make_opt( mg_levels, "mg-levels", 'l', "multi-resolution levels"));
	options.add(make_opt( max_linear_passes, "linear-passes", 'p', "linear registration passes (0 to disable)")); 
	options.add(make_opt( max_nonlinear_passes, "nonlinear-passes", 'P', "non-linear registration passes (0 to disable)")); 

	options.set_group("ICA"); 
	options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation"));
	options.add(make_opt( normalize, "normalize", 0, "normalized ICs"));
	options.add(make_opt( no_meanstrip, "no-meanstrip", 0, 
				    "don't strip the mean from the mixing curves"));
	options.add(make_opt( box_scale, "segscale", 's', 
				    "segment and scale the crop box around the LV (0=no segmentation)"));
	options.add(make_opt( skip_images, "skip", 'k', "skip images at the beginning of the series "
				    "e.g. because as they are of other modalities")); 
	options.add(make_opt( max_ica_iterations, "max-ica-iter", 'm', "maximum number of iterations in ICA")); 

	options.add(make_opt(segmethod , C2DPerfusionAnalysis::segmethod_dict, "segmethod", 'E', 
			     "Segmentation method")); 
	options.add(make_opt(min_breathing_frequency, "min-breathing-frequency", 'b', 
			     "minimal mean frequency a mixing curve can have to be considered to stem from brething. "
			     "A healthy rest breating rate is 12 per minute. A negative value disables the test. "
			     "A value 0.0 forces the series to be indentified as acquired with initial breath hold.")); 
	

	if (options.parse(argc, argv) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

        
	// load input data set
	CSegSetWithImages  input_set(in_filename, true);
	C2DImageSeries input_images = input_set.get_images(); 

        // copy the original image if the global reference it set, because in this case we
        // want the original sized data as result
        
        C2DImageSeries original_images; 
        if (global_reference >= 0) 
                original_images = input_set.get_images(); 

        
        float rel_min_bf = get_relative_min_breathing_frequency(input_images,  skip_images, min_breathing_frequency); 
	
        // now start the first ICA to run the segmentation etc.  
	cvmsg() << "skipping " << skip_images << " images\n"; 
	vector<C2DFImage> series(input_images.size() - skip_images); 
	transform(input_images.begin() + skip_images, input_images.end(), 
		  series.begin(), FCopy2DImageToFloatRepn()); 
	

	// run ICA
	unique_ptr<C2DPerfusionAnalysis> ica(new C2DPerfusionAnalysis(components, normalize, !no_meanstrip)); 
	if (max_ica_iterations) 
		ica->set_max_ica_iterations(max_ica_iterations); 

	if (rel_min_bf >= 0) 
		ica->set_min_movement_frequency(rel_min_bf); 


	ica->set_approach(FICA_APPROACH_DEFL); 
	if (!ica->run(series)) {
		ica.reset(new C2DPerfusionAnalysis(components, normalize, !no_meanstrip)); 
		ica->set_approach(FICA_APPROACH_SYMM); 
		if (!ica->run(series)) 
			box_scale = false; 
	}		

	if( input_set.get_RV_peak() < 0)  {
		if (ica->get_RV_peak_time() > 0)
			input_set.set_RV_peak(ica->get_RV_peak_time() + skip_images); 
	}
	if( input_set.get_LV_peak() < 0) {
		if (ica->get_LV_peak_time() > 0) 
			input_set.set_LV_peak(ica->get_LV_peak_time() + skip_images);
	}

	bool segentation_possible = ica->get_RV_idx() >= 0 && ica->get_LV_idx() >= 0; 
	if (!save_crop_feature.empty() && segentation_possible)
		ica->save_feature_images(save_crop_feature);
	
	
	vector<C2DFImage> references_float = ica->get_references(); 
	
	C2DImageSeries references(references_float.size()); 
	transform(references_float.begin(), references_float.end(), references.begin(), 
		  FWrapStaticDataInSharedPointer<C2DImage>()); 

	// crop if requested && possible
        C2DBounds crop_start; 
	if (box_scale  && segentation_possible) {
		crop_start = segment_and_crop_input(input_set, *ica, box_scale, segmethod, references, save_crop_feature); 
		input_images = input_set.get_images(); 
	}else if (!save_crop_feature.empty()) {
		stringstream cfile; 
		cfile << save_crop_feature << ".txt"; 
		ica->save_coefs(cfile.str()); 
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
			throw create_exception<runtime_error>("unable to save to '", cropped_filename, "'"); 

	}

        if (max_linear_passes > 0) 

                run_linear_registration_passes (input_set, references,  
                                                components, normalize, no_meanstrip,  max_ica_iterations, 
                                                skip_images,  linear_minimizer, linear_transform, 
                                                mg_levels, max_linear_passes, imagecost, global_reference, rel_min_bf); 

        if (max_nonlinear_passes > 0) {
		// if we come from the linear registration, then the references must be re-generated
		vector<C2DFImage> references_float; 
		if (max_linear_passes > 0) {
			C2DPerfusionAnalysis ica2(components, normalize, !no_meanstrip); 
			if (max_ica_iterations) 
				ica2.set_max_ica_iterations(max_ica_iterations); 
			if (rel_min_bf >= 0)
				ica2.set_min_movement_frequency(rel_min_bf); 
			
			vector<C2DFImage> series(input_set.get_images().size() - skip_images); 
			transform(input_set.get_images().begin() + skip_images, 
				  input_set.get_images().end(), series.begin(), FCopy2DImageToFloatRepn()); 
			
			if (!ica2.run(series)) {
				ica2.set_approach(FICA_APPROACH_SYMM); 
				ica2.run(series); 
			}
			
			references_float = ica2.get_references(); 
			transform(references_float.begin(), references_float.end(), 
				  references.begin(), FWrapStaticDataInSharedPointer<C2DImage>()); 
			
		}
                run_nonlinear_registration_passes (input_set, references,  
						   components,  normalize, no_meanstrip, max_ica_iterations, 
                                                   skip_images,  nonlinear_minimizer, 
                                                   mg_levels, c_rate, c_rate_divider, 
                                                   divcurlweight, divcurlweight_divider, 
                                                   max_nonlinear_passes, imagecost, global_reference, rel_min_bf); 
	}
	cvmsg() << "Registration finished\n"; 

	// copy the data back to the original images if requested 
        	// re-insert the registered sub-images if we have a global reference
	if (global_reference >= 0 && box_scale &&  input_set.get_images()[0]->get_size() != original_images[0]->get_size()) {
		cvmsg() << "Put cropped and aligned data back into the original images\n"; 
		auto registered_images = input_set.get_images(); 
		const FInsertData id(crop_start, crop_start + registered_images[0]->get_size()); 
		transform(original_images.begin(), original_images.end(), registered_images.begin(), 
			  original_images.begin(), 
			  [&id](P2DImage orig, P2DImage part) {
				  filter_equal_inplace(id, *part, *orig); 
				  return orig; 
			  }); 
		
                auto tr_creator = C2DTransformCreatorHandler::instance().produce("translate"); 
		P2DTransformation shift = tr_creator->create(C2DBounds(1,1)); 
		auto p = shift->get_parameters(); 
		p[0] = -(float)crop_start.x; 
		p[1] = -(float)crop_start.y; 
		shift->set_parameters(p); 
		
		input_set.transform(*shift);
		input_set.set_images(original_images);  

	}
	cvmsg() << "Save registered images\n"; 
	input_set.save_images(out_filename); 
	
	unique_ptr<xmlpp::Document> outset(input_set.write());
	ofstream outfile(out_filename.c_str(), ios_base::out );
	if (outfile.good())
		outfile << outset->write_to_string_formatted();
	
	return outfile.good() ? EXIT_SUCCESS : EXIT_FAILURE;

}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
