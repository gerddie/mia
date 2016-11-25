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


#include <mia/internal/main.hh>
#include <mia/template/filter_chain.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/filter.hh>
#include <mia/core/convergence_measure.hh>
#include <mia/core/paramarray.hh>
#include <mia/core/watch.hh>
#include <mia/3d/imageio.hh>
#include <mia/3d/filter.hh>
#include <mia/3d/transformio.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/vfregularizer.hh>
#include <mia/3d/cost.hh>
#include <mia/3d/deformer.hh>


NS_MIA_USE;
using namespace std;

const SProgramDescription g_description = {
        {pdi_group, "Registration, Comparison, and Transformation of 3D images"}, 
	{pdi_short, "Non-linear registration of 3D images by using a diffeomorphic SyN registration"}, 
	{pdi_description, "This program implements the registration of two gray scale 3D images. "
         "The transformation applied is a symmeric diffeomorpic fluid dynamic registration. "
         "To work most efficiently, this program makes a few assumptions about the imput data, i.e. "
         "the image must be of the same size, have the same voxel spacing, and any intensity "
         "range normalization or equalization should also be done before calling this program." 
        }, 
	{pdi_example_descr, "Register image test.v to image ref.v saving the transformation to transform.v "
         "and the inverse transform to inverse-transform.v. "
	 "Use three multiresolution levels, ssd as image cost function."}, 
	{pdi_example_code, "-i test.v -r ref.v -t transform.v -T inverse-transform.v  -l 3 ssd"}
};

void check_number_of_levels_consistency(size_t nvalues, size_t levels, const std::string& value_name)
{
        if (nvalues != 1 && nvalues != levels) {
                throw create_exception<invalid_argument>(levels, " ", 
                                                         value_name, "  values given, but ", nvalues, 
                                                         " multiresolution levels requested"); 
        }
}


typedef pair<P3DFVectorfield, P3DFVectorfield> TransformPair;

struct C3DSymScaledRegisterParams {
	uint32_t mg_levels; 
	P3DImageCost cost; 
        float current_step;
	P3DVectorfieldRegularizer regularizer;
	TPerLevelScalarParam<uint32_t> conv_count;
	TPerLevelScalarParam<double> stop_decline_rate;
	TPerLevelScalarParam<double> stop_cost;
        TPerLevelScalarParam<unsigned> iterations; 
	
	C3DSymScaledRegisterParams();

	void add_options(CCmdOptionList& options);
}; 

C3DSymScaledRegisterParams::C3DSymScaledRegisterParams():
	mg_levels(3), 
        current_step(0.25), 
	conv_count(10), 
	stop_decline_rate(0.1), 
	stop_cost(0.1), 
        iterations(100)
{
}

void C3DSymScaledRegisterParams::add_options(CCmdOptionList& options)
{
	options.set_group("Registration");
	options.add(make_opt( mg_levels, "mg-levels", 'l', "Number of multi-resolution levels to run the registration on. "
			      "Not that some registration parameters can be given as a coma seperated list to "
			      "indicate per level values. In these cases  if the number of given values is smaller "
			      "than the number of multi-resolution levels (this parameter), the the last given value "
			      "is used for all subsequest multi-resolution levels."));

	options.add(make_opt(current_step,  EParameterBounds::bf_min_open | EParameterBounds::bf_max_closed,
			     {0.0f, 0.5f},  "step", 'S', "Initial step size for all levels")); 
	
	options.add(make_opt( cost, "ssd", "cost", 'c', "Image similarity function to be minimized"));
	options.add(make_opt( regularizer, "sor:kernel=fluid,maxiter=50", "regularizer", 'R', 
			      "Regularization for the force to transformation update")); 


	options.add(conv_count.
		    create_level_params_option("conv-test-interval",'T', EParameterBounds::bf_closed_interval, {4,40}, 
					       "Convergence test interations intervall: In order to measure "
					       "convergence the cost function value is averaged over this "
					       "amount of iterations, and the decline rate is evaluated based on the "
					       "linare regression of the cost function values in this intervall. "
					       "This parameter can be given as a coma-seperated list with values corresponding "
					       "to the multi-resolution levels, see option --mg-levels for more information."
			    ));
	options.add(stop_decline_rate.
		    create_level_params_option("stop-decline-rate", 'D', EParameterBounds::bf_min_closed, {0},
					       "Stopping criterium for registration based on the cost decline rate. "
					       "If the rate below this value, the iteration is stopped. "
					       "This parameter can be given as a coma-seperated list with values corresponding "
					       "to the multi-resolution levels, see option --mg-levels for more information."
			    ));
	
	options.add(stop_cost.
		    create_level_params_option("stop-cost", 'C',
					       "Stopping criterium for registration based on the cost value. "
					       "If the cost drops below this value, the iteration is stopped. "
					       "This parameter can be given as a coma-seperated list with values corresponding "
					       "to the multi-resolution levels, see option --mg-levels for more information."
			    ));
	
	options.add(iterations.
		    create_level_params_option("iter", 'I', EParameterBounds::bf_min_closed, {4},
					       "Naximum number if iterations done on each multi-resolution level. "
					       "This parameter can be given as a coma-seperated list with values corresponding "
					       "to the multi-resolution levels, see option --mg-levels for more information."));
}


class C3DSymFluidRegistration {
public: 
        C3DSymFluidRegistration(const C3DSymScaledRegisterParams& params);
        
        TransformPair run(const C3DFImage& src, const C3DFImage& ref) const; 
private:
	const C3DSymScaledRegisterParams& m_params;
}; 

P3DTransformation wrap_vectorfield_in_transformation(const C3DFVectorfield& field, 
                                                     const C3DTransformCreator& vftcreator) 
{
        
	// the final transformation is twice the distance 
	auto transform = vftcreator.create(field.get_size()); 
	CDoubleVector buffer(transform->degrees_of_freedom(), false);

	double max_transform = 0.0;
	C3DFVector max_vec; 
	
	auto ib = buffer.begin(); 
	for (auto ivf = field.begin(); ivf != field.end(); ++ivf) {
		cvdebug() << *ivf << "\n"; 
		*ib++ = 2.0f * ivf->x; 
		*ib++ = 2.0f * ivf->y; 
		*ib++ = 2.0f * ivf->z;

		double vm = ivf->norm();
		if (vm > max_transform) {
			max_transform = vm;
			max_vec = *ivf; 
		}
	}

	cvmsg() << "Max-transform = " << max_vec << " norm=" << max_transform << "\n"; 
	transform->set_parameters(buffer); 
        return transform; 
}

C3DFImage get_asfloat_pixel(P3DImage image) 
{
        if (image->get_pixel_type() != it_float)
                image = run_filters(image, "convert:map=copy,repn=float");
        return dynamic_cast<const C3DFImage&>(*image); 
}

int do_main( int argc, char *argv[] )
{
        string src_filename;
	string ref_filename;
        
	string out_transform_filename;
	string out_inv_transform_filename;


        const auto& imageio = C3DImageIOPluginHandler::instance();
        const auto& transio = C3DTransformationIOPluginHandler::instance();

        CCmdOptionList options(g_description);
        options.set_group("IO"); 
	options.add(make_opt( src_filename, "in-image", 'i', "test image", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( ref_filename, "ref-image", 'r', "reference image", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( out_transform_filename, "transform", 'o', "output transformation", 
			      CCmdOptionFlags::required_output, &transio));
	options.add(make_opt( out_inv_transform_filename, "inverse-transform", 'O', 
                              "inverse output transformation", 
			      CCmdOptionFlags::required_output, &transio));

	C3DSymScaledRegisterParams params; 
	params.add_options(options); 
        
                              
        if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

        
        P3DImage src = load_image3d(src_filename); 
        P3DImage ref = load_image3d(ref_filename); 
        
        C3DSymFluidRegistration registration(params); 
        
        auto result = registration.run(get_asfloat_pixel(src), get_asfloat_pixel(ref));
        
        auto vftranscreator  = produce_3dtransform_factory("vf:imgkernel=[bspline:d=1],imgboundary=zero");
        
        P3DTransformation t_src_ref = wrap_vectorfield_in_transformation(*result.first, *vftranscreator); 
        
        if (!C3DTransformationIOPluginHandler::instance().save(out_transform_filename, *t_src_ref)) 
		throw create_exception<runtime_error>( "Unable to save transformation to '", 
                                                       out_transform_filename, "'"); 
        
        P3DTransformation t_ref_src = wrap_vectorfield_in_transformation(*result.second, *vftranscreator);
        
        if (!C3DTransformationIOPluginHandler::instance().save(out_inv_transform_filename, *t_ref_src)) 
		throw create_exception<runtime_error>( "Unable to save transformation to '", 
                                                       out_inv_transform_filename, "'"); 
        return EXIT_SUCCESS; 
}


MIA_MAIN(do_main); 


C3DSymFluidRegistration::C3DSymFluidRegistration(const C3DSymScaledRegisterParams& params):
	m_params(params)
{
}

P3DFVectorfield upscale( const C3DFVectorfield& vf, C3DBounds size)
{
	P3DFVectorfield Result(new C3DFVectorfield(size));
	float x_mult = float(size.x) / (float)vf.get_size().x;
	float y_mult = float(size.y) / (float)vf.get_size().y;
	float z_mult = float(size.z) / (float)vf.get_size().z;
	float ix_mult = 1.0f / x_mult;
	float iy_mult = 1.0f / y_mult;
	float iz_mult = 1.0f / z_mult;

	double max_defo = 0.0; 
	auto callback = [&](const C1DParallelRange& range){
		//	CThreadMsgStream thread_stream;
		
		for (auto z = range.begin(); z != range.end();++z) { 
			auto i = Result->begin_at(0,0,z);
			for (unsigned int y = 0; y < size.y; y++)
				for (unsigned int x = 0; x < size.x; x++,++i){
					C3DFVector help(ix_mult * x, iy_mult * y, iz_mult * z);
					C3DFVector val = vf.get_interpol_val_at(help);
					*i = C3DFVector(val.x * x_mult,val.y * y_mult, val.z * z_mult);

					double defo = i->norm();
					if (max_defo < defo)
						max_defo = defo; 
				}
		}
	}; 
	pfor(C1DParallelRange(0, size.z, 1), callback);
	// race condition !! 
	cvmsg() << "max-defo at size (" << size << ")=" << max_defo << "\n"; 
	
	return Result;
}



class C3DSymScaledRegister {
public: 
        C3DSymScaledRegister(unsigned level, const C3DSymScaledRegisterParams& params); 
        void run (const C3DFImage& src, const C3DFImage& ref, TransformPair& transforms) const; 
private: 
        void deform(const C3DFImage& src, const C3DFVectorfield& t, C3DFImage& result) const;

	unsigned m_level;
	const C3DSymScaledRegisterParams& m_params;
	C3DInterpolatorFactory m_ipfac; 
        
}; 

TransformPair
C3DSymFluidRegistration::run(const C3DFImage& src, const C3DFImage& ref) const 
{
	const CWatch& watch = CWatch::instance();

	double start_time_global = watch.get_seconds(); 

	TransformPair transforms;
        
        for (unsigned l = 0; l < m_params.mg_levels; ++l) {
		double start_time_level = watch.get_seconds(); 
                unsigned scale_block = 1 << (m_params.mg_levels - l - 1);
                
                stringstream downscale_descr;
                downscale_descr << "downscale:bx=" << scale_block
                                << ",by=" << scale_block
                                << ",bz=" << scale_block;
                
                auto downscaler =
                        C3DFilterPluginHandler::instance().produce(downscale_descr.str().c_str());
                
                auto scaled_src_p = downscaler->filter(src); 
                auto scaled_ref_p = downscaler->filter(ref); 

                const C3DFImage& scaled_src = dynamic_cast<const C3DFImage&>(*scaled_src_p); 
                const C3DFImage& scaled_ref = dynamic_cast<const C3DFImage&>(*scaled_ref_p); 

                if (!transforms.first) {
                        transforms.first.reset(new C3DFVectorfield(scaled_src.get_size()));
                        transforms.second.reset(new C3DFVectorfield(scaled_src.get_size()));
                }else{
                        transforms.first = upscale(*transforms.first, scaled_src.get_size());
                        transforms.second = upscale(*transforms.second, scaled_src.get_size());
                }
                
                C3DSymScaledRegister level_worker(l, m_params); 
                
                level_worker.run(scaled_src, scaled_ref, transforms);
		cvmsg() << "Level " << l << " time=" << watch.get_seconds() - start_time_level << "\n"; 
                
        }
	cvmsg() << "Global run time " << watch.get_seconds() - start_time_global << "\n"; 
        return transforms; 
}

C3DSymScaledRegister::C3DSymScaledRegister(unsigned level, const C3DSymScaledRegisterParams& params):
	m_level(level),
	m_params(params),
	m_ipfac("bspline:d=1", "zero")
{

}

void C3DSymScaledRegister::run (const C3DFImage& src, const C3DFImage& ref, TransformPair& transforms) const
{
	TransformPair transforms_best = transforms; 
	cvmsg() << "Enter registration at level "<< m_level << " with size " << src.get_size() << "\n";
	cvmsg() << "  Running at most " << m_params.iterations[m_level] << " iterations\n"; 

	auto current_step = m_params.current_step; 
	CConvergenceMeasure conv_measure(m_params.conv_count[m_level]);

	m_params.regularizer->set_size(src.get_size()); 
        
        // deform the images with the input transformations
        C3DFImage src_tmp(src.get_size()); 
        deform(src, *transforms.first, src_tmp); 
        
        C3DFImage ref_tmp(ref.get_size()); 
        deform(ref, *transforms.second, ref_tmp); 

        // now this runs ping-pong 
        unsigned iter = 0; 

	C3DFVectorfield grad(src.get_size()); 
        C3DFVectorfield v(src.get_size()); 

	double decline_rate = numeric_limits<double>::max();
	double avg_cost = numeric_limits<double>::max();

	double old_cost = numeric_limits<double>::max();
	
        while (!conv_measure.is_full_size() || (
	       decline_rate > m_params.stop_decline_rate[m_level] &&
	       avg_cost > m_params.stop_cost[m_level] &&
	       iter < m_params.iterations[m_level] &&
	       current_step > 0.0001)) {
		
		double start_time_level = CWatch::instance().get_seconds(); 
                ++iter; 

		m_params.cost->set_reference(ref_tmp);
		double cost_fw = m_params.cost->evaluate_force(src_tmp, grad); 
		float max_v_fw = m_params.regularizer->run(v, grad, *transforms.first); 
		transforms.first->update_by_velocity(v, current_step / max_v_fw);
		transforms.second->update_as_inverse_of(*transforms.first, 1e-5, 20);
		
                deform(src, *transforms.first, src_tmp); 
                deform(ref, *transforms.second, ref_tmp);
		
		m_params.cost->set_reference(src_tmp);
		double cost_bw = m_params.cost->evaluate_force(ref_tmp, grad);
		double cost_val = cost_bw + cost_fw; 

                // update step length 
		if ( cost_val <= old_cost ) {
			
			transforms_best = transforms;

			old_cost = cost_val;
			conv_measure.push(cost_val);
			
			decline_rate = - conv_measure.rate();
			avg_cost = conv_measure.value();
			
			cvmsg() << "[" << setw(4) << iter << "]:"
				<< "cost = "<< cost_fw + cost_bw
				<< ", step = " << current_step
				<< ", cost_avg(n="<< conv_measure.fill()<< ")=" << avg_cost
				<< ", rate=" <<  decline_rate
				<< ", ct=" << CWatch::instance().get_seconds() - start_time_level << "s"
				<< "\n";
			
		}
		
		if ( cost_val < 0.9 * old_cost ) {
			if (current_step < 0.25) {
				current_step *= 1.25;
				if (current_step > 0.25)
					current_step = 0.25;
			}
		}else if ( cost_val > old_cost ) {
			current_step *= 0.5;
			transforms = transforms_best;
			cvmsg() << "Increasing cost = " << cost_val
				<< ", Discard step and retry with step size" << current_step; 
		}
				
		float max_v_bw = m_params.regularizer->run(v, grad, *transforms.second); 
		
		transforms.second->update_by_velocity(v, current_step / max_v_bw);  
		transforms.first->update_as_inverse_of(*transforms.second, 1e-5, 20);
                
                deform(src, *transforms.first, src_tmp); 
                deform(ref, *transforms.second, ref_tmp);
        }
	transforms = transforms_best; 
}

void C3DSymScaledRegister::deform(const C3DFImage& src, const C3DFVectorfield& t, C3DFImage& result) const
{
        FDeformer3D src_deformer(t, m_ipfac); 
        src_deformer(src,result); 
}

