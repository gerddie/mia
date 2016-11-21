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


#include <mia/internal/main.hh>
#include <mia/template/filter_chain.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/filter.hh>
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

class C3DSymFluidReistration {
public: 
        C3DSymFluidReistration(P3DImageCost cost, unsigned mg_levels, 
                               const vector<unsigned>& iterations_per_level, 
                               const vector<double>& epsilon_per_level,
			       P3DVectorfieldRegularizer regularizer
		);
        
        TransformPair run(const C3DFImage& src, const C3DFImage& ref) const; 
private: 
        P3DImageCost m_cost; 
        unsigned m_mg_levels; 
        const vector<unsigned>& m_iterations_per_level; 
        const vector<double>& m_epsilon_per_level;
	P3DVectorfieldRegularizer m_regularizer; 
}; 

P3DTransformation wrap_vectorfield_in_transformation(const C3DFVectorfield& field, 
                                                     const C3DTransformCreator& vftcreator) 
{
        
	
	auto transform = vftcreator.create(field.get_size()); 
	CDoubleVector buffer(transform->degrees_of_freedom(), false);
	
	auto ib = buffer.begin(); 
	for (auto ivf = field.begin(); ivf != field.end(); ++ivf) {
		cvdebug() << *ivf << "\n"; 
		*ib++ = ivf->x; 
		*ib++ = ivf->y; 
		*ib++ = ivf->z; 
	}
	
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

        unsigned mg_levels = 3; 

        P3DImageCost cost;

        vector<unsigned> iterations_per_level; 
        vector<double>   epsilon_per_level;

	P3DVectorfieldRegularizer regularizer; 

        const auto& imageio = C3DImageIOPluginHandler::instance();
        const auto& transio = C3DTransformationIOPluginHandler::instance();

        CCmdOptionList options(g_description);
        options.set_group("IO"); 
	options.add(make_opt( src_filename, "in-image", 'i', "test image", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( ref_filename, "ref-image", 'r', "reference image", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( out_transform_filename, "transform", 't', "output transformation", 
			      CCmdOptionFlags::required_output, &transio));
	options.add(make_opt( out_inv_transform_filename, "inverse-transform", 'T', 
                              "inverse output transformation", 
			      CCmdOptionFlags::required_output, &transio));

        options.set_group("Registration"); 
        options.add(make_opt( cost, "ssd", "cost", 'c', "Image similarity function to be minimized"));
        options.add(make_opt( mg_levels, "levels", 'l', "multi-resolution levels"));
        options.add(make_opt( iterations_per_level, "niter", 'n', 
                              "maximum number of iterations of the optimizer at each multi-resolution level. "
                              "If only one value is given, the this will be used foe all levels, "
                              "otherwise the number of values must coincide with the number of registration levels."
                              "(default=100)"));

	options.add(make_opt( regularizer, "sor:kernel=fluid", "regularizer", 'R', 
			      "Regularization for the force to transformation update")); 
	
        options.add(make_opt( epsilon_per_level, "frel", 0, 
                              "Breaking condition: relative change of the cost function. "
                              "If only one value is given, the this will be used foe all levels, "
                              "otherwise the number of values must coincide with the number of registration levels. ""regularizer"));
        
                              
        if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

        // set some defaults if not given (This should go into the command line parameter) 
        if (iterations_per_level.empty()) 
                iterations_per_level[0] = 100; 
        
        if (epsilon_per_level.empty()) 
                epsilon_per_level[0] = 1e-7; 
        
        check_number_of_levels_consistency(iterations_per_level.size(), mg_levels, "iterations"); 
        check_number_of_levels_consistency(epsilon_per_level.size(), mg_levels, "epsilon"); 
        
        P3DImage src = load_image3d(src_filename); 
        P3DImage ref = load_image3d(ref_filename); 
        
        C3DSymFluidReistration registration(cost, mg_levels, iterations_per_level, epsilon_per_level, regularizer); 
        
        auto result = registration.run(get_asfloat_pixel(src), get_asfloat_pixel(ref));
        
        auto vftranscreator  = produce_3dtransform_factory("vf:imgkernel=[bspline:d=1],imgboundary=zero");
        
        
        P3DTransformation t_src_ref = wrap_vectorfield_in_transformation(*result.first, *vftranscreator); 
        
        if (!C3DTransformationIOPluginHandler::instance().save(out_transform_filename, *t_src_ref)) 
		throw create_exception<runtime_error>( "Unable to save transformation to '", 
                                                       out_transform_filename, "'"); 
        
        P3DTransformation t_ref_src = wrap_vectorfield_in_transformation(*result.second, *vftranscreator);
        
        if (!C3DTransformationIOPluginHandler::instance().save(out_inv_transform_filename, *t_src_ref)) 
		throw create_exception<runtime_error>( "Unable to save transformation to '", 
                                                       out_inv_transform_filename, "'"); 
        return EXIT_SUCCESS; 
}


MIA_MAIN(do_main); 


C3DSymFluidReistration::C3DSymFluidReistration(P3DImageCost cost, unsigned mg_levels, 
                                               const vector<unsigned>& iterations_per_level, 
                                               const vector<double>& epsilon_per_level,
					       P3DVectorfieldRegularizer regularizer):
	m_cost(cost),
	m_mg_levels(mg_levels),
	m_iterations_per_level(iterations_per_level), 
	m_epsilon_per_level(epsilon_per_level),
	m_regularizer(regularizer)
											
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

	auto callback = [&](const C1DParallelRange& range){
		//	CThreadMsgStream thread_stream;
		
		for (auto z = range.begin(); z != range.end();++z) { 
			auto i = Result->begin_at(0,0,z);
			for (unsigned int y = 0; y < size.y; y++)
				for (unsigned int x = 0; x < size.x; x++,++i){
					C3DFVector help(ix_mult * x, iy_mult * y, iz_mult * z);
					C3DFVector val = vf.get_interpol_val_at(help);
					*i = C3DFVector(val.x * x_mult,val.y * y_mult, val.z * z_mult);
				}
		}
	}; 
	pfor(C1DParallelRange(0, size.z, 1), callback);

	return Result;
}

class C3DSymScaledRegister {
public: 
        C3DSymScaledRegister(P3DImageCost cost, double iter,
			     double epsilon, P3DVectorfieldRegularizer regularizer); 
        void run (const C3DFImage& src, const C3DFImage& ref, TransformPair& transforms) const; 
private: 
        void deform(const C3DFImage& src, const C3DFVectorfield& t, C3DFImage& result) const;

        P3DImageCost m_cost; 
        unsigned m_iter; 
        double m_epsilon;
        C3DInterpolatorFactory m_ipfac; 
        float m_current_step;
	P3DVectorfieldRegularizer m_regularizer; 
}; 

TransformPair
C3DSymFluidReistration::run(const C3DFImage& src, const C3DFImage& ref) const 
{
        TransformPair transforms;
        
        for (unsigned l = 0; l < m_mg_levels; ++l) {
                unsigned scale_block = 1 << (m_mg_levels - l - 1);
                
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
                
                unsigned level_iter = m_iterations_per_level.size() == 1 ? m_iterations_per_level[0]: 
                        m_iterations_per_level[l]; 
                unsigned level_epsilon = m_epsilon_per_level.size() == 1 ? m_epsilon_per_level[0]:
                        m_epsilon_per_level[l]; 

                C3DSymScaledRegister level_worker(m_cost, level_iter, level_epsilon, m_regularizer); 
                
                level_worker.run(scaled_src, scaled_ref, transforms);
                
        }
        return transforms; 
}

C3DSymScaledRegister::C3DSymScaledRegister(P3DImageCost cost, double iter,
					   double epsilon, P3DVectorfieldRegularizer regularizer):
	m_cost(cost), 
	m_iter(iter), 
	m_epsilon(epsilon), 
	m_ipfac("bspline:d=0", "zero"), 
	m_current_step(0.25),
	m_regularizer(regularizer)
{

}

void C3DSymScaledRegister::run (const C3DFImage& src, const C3DFImage& ref, TransformPair& transforms) const
{
	
        
        // deform the images with the input transformations
        C3DFImage src_tmp(src.get_size()); 
        deform(src, *transforms.first, src_tmp); 
        
        C3DFImage ref_tmp(ref.get_size()); 
        deform(ref, *transforms.second, ref_tmp); 

        // now this runs ping-pong 
        unsigned iter = 0; 

        double cost = numeric_limits<double>::max(); 

        C3DFVectorfield grad(src.get_size()); 
        C3DFVectorfield v(src.get_size()); 

        while (iter < m_iter) {
                ++iter; 

		m_cost->set_reference(ref_tmp); 
                m_cost->evaluate_force(src_tmp, grad);
                
                float max_v = m_regularizer->run(v, grad, *transforms.first); 
                
		transforms.first->update_by_velocity(v, m_current_step / max_v); 
		transforms.second->update_as_inverse_of(*transforms.first, 1e-5, 20);

                deform(src, *transforms.first, src_tmp); 
                deform(ref, *transforms.second, ref_tmp);
                
                m_cost->set_reference(src_tmp); 
                m_cost->evaluate_force(ref_tmp, grad);

                max_v = m_regularizer->run(v, grad, *transforms.second); 
                
		transforms.second->update_by_velocity(v, m_current_step / max_v); 
		transforms.first->update_as_inverse_of(*transforms.first, 1e-5, 20);
                
                deform(src, *transforms.first, src_tmp); 
                deform(ref, *transforms.second, ref_tmp);

        }
}

void C3DSymScaledRegister::deform(const C3DFImage& src, const C3DFVectorfield& t, C3DFImage& result) const
{
        FDeformer3D src_deformer(t ,m_ipfac); 
        src_deformer(src,result); 
}

