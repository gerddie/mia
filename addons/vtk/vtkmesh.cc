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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>


#include <vtk/vtkmesh.hh>
#include <mia/mesh/triangulate.hh>


NS_BEGIN(vtkmia)
using namespace mia; 
using namespace std; 

CVtkMeshIO::CVtkMeshIO(): 
	CMeshIOPlugin("vtk")
{
	add_suffix(".vtk");
	add_suffix(".VTK");
	add_suffix(".vtkmesh");
	add_suffix(".VTKMESH");

}

const char * const  CVtkMeshIO::s_scale_array = "scale"; 
const char * const  CVtkMeshIO::s_normal_array = "normals"; 
const char * const  CVtkMeshIO::s_color_array = "colors"; 


static CVtkMeshIO::PVertexfield read_vertices( /*const*/ vtkPolyData& mesh) 
{
	auto n_vertices = mesh.GetNumberOfPoints(); 
	cvdebug()<< "Got " << n_vertices << " vertices \n"; 
	auto vertices = CVtkMeshIO::PVertexfield(new CVtkMeshIO::CVertexfield(n_vertices)); 
	
	// copy all vertices
	auto iv = vertices->begin(); 
	for(auto i = 0; i < n_vertices; i++, ++iv) {
		double p[3];
		mesh.GetPoint(i,p);
		iv->x = p[0];
		iv->y = p[1]; 
		iv->z = p[2]; 
	}
	return vertices; 
}

static CVtkMeshIO::PTrianglefield read_triangles(const CVtkMeshIO::CVertexfield& /*vertices*/, /*const*/ vtkPolyData& mesh)
{
	// read all cells, if a cell is formed of more than 3 corners, then triangulate, 
	// if it hes less then 3 corners, ignore it (no wireframes supported here

        auto triangles = CVtkMeshIO::PTrianglefield(new CVtkMeshIO::CTrianglefield ()); 
	vtkIdType npts, *pts;
	auto strips = mesh.GetStrips(); 
	
	while(strips->GetNextCell(npts,pts)) {
		cvdebug() << "Read cell of " << npts << " vertices\n"; 
		if (npts == 3) {
			CVtkMeshIO::CTrianglefield::value_type t(pts[0], pts[1], pts[2]); 
			triangles->push_back(t); 
		}else{ // this is a real triangle strip 
			if (npts < 3) {
				cvinfo() << "ignoring a" << ((npts == 2) ? "n edge " : " point") << "\n";
				continue; 
			}
			
			for (int i = 2; i < npts; ++i)  {
				CVtkMeshIO::CTrianglefield::value_type t; 
				t.x = pts[i-2]; 
				// in triangle strips, every other triangle is winded in the other direction 
				// misses test to see if we got it right
				if (i & 1) {
					t.z = pts[i-1]; 
					t.y = pts[i];
				} else {
					t.y = pts[i-1]; 
					t.z = pts[i];
				}
				triangles->push_back(t); 
			}
		}
	}
	
	// we may also read polygons using the triangulator 
        // TPolyTriangulator<CVtkMeshIO::CVertexfield, vector<vtkIdType>> triangulator(vertices); 
	

	cvdebug()<< "Got " << triangles->size() << " triangles \n"; 
	return triangles; 
}

static void read_scalars(CTriangleMesh& mesh, /*const*/ vtkPointData& point_data)
{
	auto abstract_scalars = point_data.GetScalars(); 
        if (!abstract_scalars) 
		return; 

	auto scales = dynamic_cast<vtkFloatArray*>(abstract_scalars);
	if (!scales) {
		cvinfo() << " got scales field, but is not of type 'vtkFloatArray'\n"; 
		return;
	}
	
	if (scales->GetNumberOfComponents() != 1)  {
		cvinfo() << " got scales field, but it has " << scales->GetNumberOfComponents() << " " 
			 << " instead of one.\n"; 
		return; 
	}
	
	auto n_scale = scales->GetNumberOfTuples(); 
	if (n_scale != mesh.vertices_size()) {
		cverr() << "Have " << mesh.vertices_size() << " but got " << n_scale 
			<< " scale values, ignoring the scales\n"; 
		return; 
	}

	auto is = mesh.scale_begin(); 
	for (auto i = 0; i < n_scale; ++i, ++is)
		*is = scales->GetValue(i); 
}

static void read_normals(CTriangleMesh& mesh, /*const*/ vtkPointData& point_data)
{
	auto abstract_normals = point_data.GetNormals(); 
	if (!abstract_normals) 
		return; 
	
	auto normals = dynamic_cast<vtkFloatArray*>(abstract_normals);
	if (!normals) {
		cvinfo() << " got normals field, but is not of type 'vtkFloatArray'\n"; 
		return; 
	}
	
	if (normals->GetNumberOfComponents() != 3)  {
                cvinfo() << " got normals field, but it has " << normals->GetNumberOfComponents() << " " 
			 << " instead of three.\n"; 
		return; 
	}
	
	auto n_normals = normals->GetNumberOfTuples(); 
	if (n_normals != mesh.vertices_size()) {
		cverr() << "Have " << mesh.vertices_size() << " but got " << n_normals 
			<< " normals values, ignoring the normals\n"; 
		return; 
	}

	
	auto is = mesh.normals_begin(); 
	for (auto i = 0; i < n_normals; ++i, ++is) {
		normals->GetTupleValue(i, &is->x); 
		cvdebug() << i << ": read normal " << *is << "\n"; 
	}
}

static void read_colors(CTriangleMesh& mesh, /*const*/ vtkPointData& point_data)
{
	if (!point_data.HasArray(CVtkMeshIO::s_color_array)) 
		return; 

	auto colors = dynamic_cast<vtkFloatArray*>(point_data.GetArray(CVtkMeshIO::s_color_array));
	if (!colors) {
		cvinfo() << " got colors field, but is not of type 'vtkFloatArray'\n"; 
		return;
	}
	
	if (colors->GetNumberOfComponents() != 3)  {
		cvinfo() << " got colors field, but it has " << colors->GetNumberOfComponents() << " " 
			 << " instead of three.\n"; 
		return; 
	}
	
	auto n_colors = colors->GetNumberOfTuples(); 
	if (n_colors != mesh.vertices_size()) {
		cverr() << "Have " << mesh.vertices_size() << " but got " << n_colors 
			<< " colors values, ignoring the colors field\n"; 
		return; 
	}

	
	auto is = mesh.color_begin(); 
	for (auto i = 0; i < n_colors; ++i, ++is)
		colors->GetTupleValue(i, &is->x); 
}

PTriangleMesh CVtkMeshIO::do_load(string const &  filename) const
{
	TRACE_FUNCTION; 
	
	auto reader = vtkSmartPointer<vtkPolyDataReader>::New();
	reader->SetFileName(filename.c_str());
	reader->Update(); 
	auto mesh = reader->GetOutput(); 
	if (!mesh)
		return PTriangleMesh(); 

	
	auto vertices = read_vertices(*mesh); 
	auto triangles = read_triangles(*vertices, *mesh); 
	
        auto out_mesh = PTriangleMesh(new CTriangleMesh(triangles, vertices));
	
	// now check for scales, normals, and colors
	auto point_data = mesh->GetPointData();
	
	read_scalars(*out_mesh, *point_data); 
	read_normals(*out_mesh, *point_data); 
	read_colors(*out_mesh, *point_data); 
	
	return out_mesh; 
}

bool CVtkMeshIO::do_save(string const &  filename, const CTriangleMesh& mesh) const
{
	// construct VTK mesh 
	auto points = vtkSmartPointer<vtkPoints>::New();
	for_each(mesh.vertices_begin(), mesh.vertices_end(), 
		 [&points](const CTriangleMesh::vertex_type& v) {points->InsertNextPoint(v.x, v.y, v.z);}); 

	auto triangles = vtkSmartPointer<vtkCellArray>::New();
	for_each(mesh.triangles_begin(), mesh.triangles_end(),  
		 [&triangles](const CTriangleMesh::triangle_type& x)->void {
			 vtkIdType p[] = {static_cast<int>(x.x), 
					  static_cast<int>(x.y), 
					  static_cast<int>(x.z)}; 
			 triangles->InsertNextCell(3, p); 
		 });

	auto data = vtkSmartPointer<vtkPolyData>::New();
	data->SetPoints(points);
	data->SetStrips(triangles);

	auto point_data = data->GetPointData(); 
	
	// now convert the optional data 
	if (mesh.get_available_data() & CTriangleMesh::ed_scale) {
		auto scales = vtkSmartPointer<vtkFloatArray>::New();
		scales->SetName(s_scale_array); 
		for_each(mesh.scale_begin(), mesh.scale_end(), 
			  [&scales](CTriangleMesh::scale_type s){scales->InsertNextValue(s);}); 
		point_data->SetScalars(scales); 
	}

	if (mesh.get_available_data() & CTriangleMesh::ed_color) {
		auto colors = vtkSmartPointer<vtkFloatArray>::New();
		colors->SetName(s_color_array);
		colors->SetNumberOfComponents (3); 
		for_each(mesh.color_begin(), mesh.color_end(), 
			 [&colors](CTriangleMesh::color_type c) -> void {
				 colors->InsertNextTuple(&c.x);
			 }); 
		point_data->AddArray(colors); 
	}

	if (mesh.get_available_data() & CTriangleMesh::ed_normal) {
		auto normals = vtkSmartPointer<vtkFloatArray>::New();
		normals->SetName(s_normal_array); 
		normals->SetNumberOfComponents (3); 
		for_each(mesh.normals_begin(), mesh.normals_end(), 
			 [&normals](CTriangleMesh::normal_type n) -> void {
				 normals->InsertNextTuple(&n.x);
			 }); 
		point_data->SetNormals(normals); 
	}
	
	// write it 
	auto writer = vtkSmartPointer<vtkPolyDataWriter>::New();
	writer->SetFileName(filename.c_str());
	writer->SetFileTypeToBinary(); 
#if  VTK_MAJOR_VERSION < 6 
	writer->SetInput(data); 
#else 
	writer->SetInputData(data); 
#endif 
	return writer->Write(); 
}

const string  CVtkMeshIO::do_get_descr() const
{
	return "A subset of VTK mesh in-and output: Triangle meshes are written, and triangle "
		"meshes and triangle strips are read. Additional per-vertex attributes are supported: 'normals', "
		"'colors' for three component colors, and 'scale' for a scalar value attached to each vertex. "
		"The data is written by the vtkPolyDataWriter in binary format.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	//vista::prepare_vista_logging();
	return new CVtkMeshIO;
}

NS_END
