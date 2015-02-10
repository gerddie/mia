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

#include <mia/core.hh>
#include <mia/mesh/triangularMesh.hh>
#include <mia/3d/imageio.hh>
#include <mia/3d/filter.hh>
#include <mia/3d/interpolator.hh>
#include <mia/internal/main.hh>

NS_MIA_USE;
using namespace std;


const SProgramDescription g_general_help = {
        {pdi_group, "Creation, analysis, and filtering of triangular 3D meshes"}, 
	{pdi_short, "Fit a mesh by using a deformable model."}, 
	{pdi_description, "This program runs a deformable model to adapt a mesh to an iso-value "
	 "within a given image. <FIXME: Reference>"}, 
	{pdi_example_descr, "Run the deforemable model on input.vmesh with 200 iterations adapting to a value of 128 "
         "in the image ref.v and save the result to deformed.vmesh"}, 
	{pdi_example_code, "-i input.vmesh -o deformed.vmesh --iso 128 --maxiter 200"}
}; 

class DeformableModel {
public: 
        DeformableModel(); 

        void add_options(CCmdOptionList& options); 
        
        PTriangleMesh run(const CTriangleMesh& mesh, const C3DFImage& reference);

private: 

        typedef set<unsigned int> CNeighbors; 
        struct SLocation {
                SLocation(CTriangleMesh::normal_type& n):m_normal(&n){}; 
                SLocation(const SLocation& other) = default; 
                SLocation() = default; 
                
                const CTriangleMesh::normal_type& normal()const {return *m_normal;}
                CTriangleMesh::normal_type *m_normal; 
                CNeighbors vertices; 

        };

        typedef vector<SLocation> CModel; 

        CModel prepare_model(CTriangleMesh& mesh); 

        float m_iso_value; 
        float m_smoothing_weight;  
        float m_gradient_weight; 
        float m_intensity_weight; 
        float m_intensity_scaling; 
        unsigned m_maxiter; 
        float m_epsilon; 

}; 

DeformableModel::DeformableModel():
        m_iso_value(64.0f), 
        m_smoothing_weight(.04f),  
        m_gradient_weight(.04f), 
        m_intensity_weight(.02f), 
        m_intensity_scaling(1.0f),
        m_maxiter(200),
        m_epsilon(0.001f)
{
}


void DeformableModel::add_options(CCmdOptionList& options)
{
        options.set_group("Model parameters"); 
        options.add(make_opt(m_smoothing_weight, EParameterBounds::bf_min_closed, {0}, "smoothing-weight", 0,
                             "Weight of the inner force used to smooth the mesh"));
        options.add(make_opt(m_gradient_weight, "gradient-weight", 0, "Weight of the gradient force drive the mesh deformation. "
                             "Use a negative value to invert the search direction.")); 
        options.add(make_opt(m_intensity_weight, EParameterBounds::bf_min_closed, {0}, 
                             "intensity-weight", 0, "Weight of the force resulting from the intensity difference "
                             "at the vertex position versus the reference intensity 'iso'.")); 
        options.add(make_opt(m_intensity_scaling, EParameterBounds::bf_min_open, {0}, "intensity-scaling", 0, 
                             "Scaling of the raw intensity difference."));
        options.add(make_opt(m_iso_value, "iso", 's', "Intensity value the mesh verices should adapt to.")); 
        
        options.set_group("Processing"); 
        options.add(make_opt(m_maxiter, EParameterBounds::bf_min_open, {0}, "maxiter", 'm', 
                             "Maximum number of iterations."));
        options.add(make_opt(m_epsilon, EParameterBounds::bf_min_open, {0}, "epsilon", 'e', 
                             "Stop iteration when the maximum shift of the vertices falls below this value")); 
}

PTriangleMesh DeformableModel::run(const CTriangleMesh& mesh, const C3DFImage& reference)
{
        C3DInterpolatorFactory ipf("bspline:d=1", "zero");

        unique_ptr<T3DConvoluteInterpolator<float>> R(ipf.create(reference.data())); 
        const auto gradient = get_gradient(reference);


        PTriangleMesh result(new CTriangleMesh(mesh)); 
        result->evaluate_normals();
        
        CModel model = prepare_model(*result); 
        
	double  sum_dist = 0.0; 
        float max_shift = numeric_limits<float>::max(); 
        unsigned  iter = 0; 

        vector<C3DFVector> out_vertex(model.size()); 
        
        while (iter++ < m_maxiter && max_shift > m_epsilon) {
                
                sum_dist = 0.0; 
		max_shift = 0.0; 


                for (unsigned i = 0; i < model.size(); ++i) {
			
                        auto& vertex = result->vertex_at(i); 
                        
			// values for external forces
			float iso_delta = (m_iso_value - (*R)(vertex))/m_intensity_scaling; 
			
			float grad_scale =  dot(gradient(vertex), model[i].normal());
			float f3 = tanh( iso_delta );
			float f2 = grad_scale * f3 / 100.0;
			float f1 = m_gradient_weight * f2 + m_intensity_weight * f3;

                        C3DFVector shift = f1 * model[i].normal(); 
                        
                        // evaluate internal force 
			C3DFVector center = C3DFVector::_0; 

                        int n_neightbors = model[i].vertices.size(); 
                        if ( n_neightbors > 0) {
                                
                                for (auto iv : model[i].vertices)
                                        center += result->vertex_at(iv); 
                                
				center /= n_neightbors; 
                                shift += m_smoothing_weight * center; 
			}
                        
			sum_dist += iso_delta > 0 ? iso_delta : -iso_delta;
			
			out_vertex[i] = vertex + shift; 
			
			float snorm = shift.norm();
			
			if (max_shift < snorm)
				max_shift = snorm;
			
		}
                
                // copy resulting vertices and re-evaluate normales 
                copy(out_vertex.begin(), out_vertex.end(), result->vertices_begin()); 
		result->evaluate_normals();
                
		cvmsg() << "[" << iter << "]: distance = " 
                        << sum_dist << "; max shift = " 
                        << max_shift << "                    \r"; 
	}
	cvmsg() << endl; 
        return result; 
}

DeformableModel::CModel DeformableModel::prepare_model(CTriangleMesh& mesh)
{
	CModel model(mesh.vertices_size()); 
        
	CTriangleMesh::normal_iterator nb = mesh.normals_begin();

        for (size_t i = 0; i < mesh.vertices_size(); ++i, ++nb) {
                model[i] = SLocation(*nb); 
        }
	
	CTriangleMesh::triangle_iterator tb = mesh.triangles_begin();
	CTriangleMesh::triangle_iterator te = mesh.triangles_end();
	
	while (tb != te) {
		SLocation& nx = model[tb->x];
		nx.vertices.insert(tb->x); 
		nx.vertices.insert(tb->z);
		
		SLocation& ny = model[tb->y];
 		ny.vertices.insert(tb->x); 
		ny.vertices.insert(tb->z);

                SLocation& nz = model[tb->z];
		nz.vertices.insert(tb->x); 
		nz.vertices.insert(tb->y);
		
		++tb; 
	}
        return model; 
}

int do_main( int argc, char *argv[] )
{

	string in_filename;
	string out_filename;
        string ref_filename;
        
        DeformableModel model; 

        P3DFilter smoothing; 
        
	const auto& meshio = CMeshIOPluginHandler::instance(); 
	const auto& imgio = C3DImageIOPluginHandler::instance(); 

	CCmdOptionList options(g_general_help);
        options.set_group("File I/O"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input mesh to be adapted", 
                              CCmdOptionFlags::required_input, &meshio));
	options.add(make_opt( out_filename, "out-file", 'o', "output mesh that has been deformed", 
                              CCmdOptionFlags::required_output, &meshio));
        options.add(make_opt( ref_filename, "ref-file", 'r', "reference image", 
                              CCmdOptionFlags::required_input, &imgio));

        model.add_options(options); 

        options.set_group("Preprocessing"); 
        options.add(make_opt(smoothing, "gauss:w=2", "image-smoothing", 0, 
                             "Prefilter to smooth the reference image."));
        
        if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


        auto mesh = meshio.load(in_filename); 
	if (!mesh) 
		throw create_exception<invalid_argument>( "No mesh found in '", in_filename, "'"); 
        
        auto ref = load_image3d(ref_filename); 
        ref = run_filters(ref, "convert:map=copy,repn=float", smoothing); 
        const C3DFImage& reference = dynamic_cast<const C3DFImage&>(*ref); 
        
        auto deformed = model.run(*mesh,  reference);
        
        if ( !meshio.save(out_filename, *deformed) )
		throw create_exception<runtime_error>( "Unable to save result to '", out_filename, "'");
        
	return EXIT_SUCCESS;

}

MIA_MAIN(do_main); 
