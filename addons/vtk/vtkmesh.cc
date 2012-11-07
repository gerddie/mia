/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <vtkPolyData.h>
#include <vtk/vtkmesh.hh>
#include <mia/mesh/triangulate.hh>

namespace vtkmia {
using namespace mia; 
using namespace std; 

CVtkMeshIO::CVtkMeshIO()
{
	add_suffix(".vtk");
	add_suffix(".VTK");
	add_suffix(".vtkmesh");
	add_suffix(".VTKMESH");

}


PTriangleMesh CVtkMeshIO::do_load(string const &  filename) const
{
	auto reader = vtkSmartPointer<vtkPolyDataReader>::New();
	reader->SetFileName(filename);
	auto mesh = reader->GetOutput(); 
	
	auto n_vertices = polydata->GetNumberOfPoints(); 
	auto vertices = PVertexfield(new CVertexfield(n_vertices)); 
	
	// copy all vertices
	auto iv = vertices->begin(); 
	for(auto i = 0; i < n_vertices; i++, ++iv) {
		double p[3];
		mesh->GetPoint(i,p);
		iv->x = p[0]; 
		iv->y = p[1]; 
		iv->z = p[2]; 
	}
	

	// read all cells, if a cell is formed of more than 3 corners, then triangulate, 
	// if it hes less then 3 corners, ignore it (no wireframes supported here
	TPolyTriangulator<CVertexfield, vector<vtkIdType>> triangulator; 
	auto triangles = PTrianglefield(new CTrianglefield); 
	vtkIdType npts, *pts;
	auto polys = mesh->GetPolys(); 
	while(polys->GetNextCell(npts,pts)) {
		if (npts == 3) {
			CTrianglefield::value_type t(pts[0], pts[1], pts[2]); 
			triangles->push_back(t); 
		}else{ // triangulate the polygon 
			vector<vtkIdType> polygon(&pts[0], &pts[npts]);
			CTrianglefield tf; 
			if (triangulator.triangulate(tf, polygon))
				triangles->insert(triangles->end(), tf.begin(), tf.end()); 
			else
				cvinfo() << "ignoring " << ((npts == 2) ? "edge " : "point ")
					 << polygon << "\n";
		}
	}	
	
        auto out_mesh = PTriangleMesh(new CTriangleMesh(triangles, vertices));
	
	// now check for scales, normals, and colors
	auto point_data = mesh->GetPointData();
	
	if (point_data->HasArray(s_scale_array)) {
		auto scales = dynamic_cast<vtkFloatArray>(point_data->GetArray(s_scale_array));
		if (scales) {
			
		}
		
	}

	return out_mesh; 
}

bool CVtkMeshIO::do_save(string const &  filename, const CTriangleMesh& data) const
{
	// construct VTK mesh 
	auto points = vtkSmartPointer<vtkPoints>::New();
	for_each(data.vertices_begin(), data.vertices_end(), 
		 [points&](const CTriangleMesh::vertex_type& v) {points.InsertNextPoint(v.x, v.y, v.z);}); 

	auto triangles = vtkSmartPointer<vtkCellArray>::New();
	for_each(data.triangles_begin(), data.triangles_end(),  
		 [triangles&](const CTriangleMesh::triangle_type& x)->void {
			 vtkIdList p{x.x, x.y, x.z}; 
			 triangles->InsertNextCell(VTK_TRIANGLE, p); 
		 });

	auto data = vtkSmartPointer<vtkPolyData>::New();
	data->SetPoints(points);
	data->SetStrips(triangles);

	auto point_data = data.GetPointData(); 
	
	// now convert the optional data 
	if (mesh.get_available_data() & CTriangleMesh::ed_scale) {
		auto scales = vtkSmartPointer<vtkFloatArray>::New();
		scales->SetName(s_scale_array)
		for_each(data.scale_begin(), data.scale_end(), 
			  [scales&](CTriangleMesh::scale_type s){scales->InsertNextValue(s);}); 
		point_data.AddArray(scales); 
	}

	if (mesh.get_available_data() & CTriangleMesh::ed_color) {
		auto colors = vtkSmartPointer<vtkFloatArray>::New();
		colors->SetName(s_colors_array)
		for_each(data.scale_begin(), data.scale_end(), 
			 [colors&](CTriangleMesh::color_type c) -> void {
				 colors->InsertNextValue(c.x);
				 colors->InsertNextValue(c.y);
				 colors->InsertNextValue(c.z);
			 }); 
		point_data.AddArray(colors); 
	}

	if (mesh.get_available_data() & CTriangleMesh::ed_normal) {
		auto normals = vtkSmartPointer<vtkFloatArray>::New();
		normals->SetName(s_normals_array)
		for_each(data.scale_begin(), data.scale_end(), 
			 [normals&](CTriangleMesh::normal_type n) -> void {
				 normals->InsertNextValue(n.x);
				 normals->InsertNextValue(n.y);
				 normals->InsertNextValue(n.z);
			 }); 
		point_data.AddArray(normals); 
	}
	
	// write it 
	auto writer = vtkSmartPointer<vtkPolyDataWriter>::New();
	writer->SetFileName(filename);
	writer->SetInputData(data); 
	return writer->write(); 

}

string  CVtkMeshIO::do_get_descr() const
{
	return "VTK mesh in-and output"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	//vista::prepare_vista_logging();
	return new CVtkMeshIO;
}

}
