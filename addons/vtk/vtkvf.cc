/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <vtk/vtkvf.hh>

#include <vtkStructuredGridReader.h>
#include <vtkStructuredGridWriter.h>
#include <vtkStructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>

using namespace mia; 
using namespace std; 
using namespace vtkvf; 

CVtk3DVFIOPlugin::CVtk3DVFIOPlugin():
	C3DVFIOPlugin("vtk")
{
	add_supported_type(it_float);
	add_suffix(".vtk");
	add_suffix(".VTK");
	add_suffix(".vtkvf");
	add_suffix(".VTKVF");

}

CVtk3DVFIOPlugin::PData CVtk3DVFIOPlugin::do_load(const string&  filename) const
{
	auto reader = vtkSmartPointer<vtkStructuredGridReader>::New(); 
	reader->SetFileName(filename.c_str()); 
	auto iovf = reader->GetOutput(); 
	iovf->Update(); 
	if (!iovf)
		return PData(); 
	
	int dim = iovf->GetDataDimension();
	if (dim != 3) {
		cvinfo() << "Expect 3 dimensions but got " << dim << "\n"; 
		return PData(); 
	}

	auto dims = iovf->GetDimensions();
	C3DBounds size(dims[0], dims[1], dims[2]); 
	
	PData result(new C3DIOVectorfield(size));
	
	auto points = iovf->GetPoints(); 


	if (points->GetNumberOfPoints() != size.product()) 
		throw create_exception<invalid_argument>("Got ", points->GetNumberOfPoints(), 
							 " vectors but require ", size.product());
	vtkIdType i = 0; 
	for (auto iv = result->begin(); iv != result->end(); ++iv, ++i) {
		double x[3]; 
		points->GetPoint (i, x); 
		iv->x = x[0]; 
		iv->y = x[1]; 
		iv->z = x[2]; 
	}
	return result; 
}

bool CVtk3DVFIOPlugin::do_save(const string& fname, const C3DIOVectorfield& data) const
{
	int dims[3];
	dims[0] = data.get_size().x; 
	dims[1] = data.get_size().y; 
	dims[2] = data.get_size().z; 

	auto sgrid = vtkSmartPointer<vtkStructuredGrid>::New();
	sgrid->SetDimensions(dims);

	auto points = vtkSmartPointer<vtkPoints>::New();
	points->Allocate(data.get_size().product());

	int offset = 0; 
	float v[3]; 
	for (auto iv = data.begin(); iv != data.end(); ++iv, ++offset) {
		v[0] = iv->x; 
		v[1] = iv->y; 
		v[2] = iv->z; 
		points->InsertPoint(offset, v); 
	}
	sgrid->SetPoints(points); 
	
	auto writer = vtkSmartPointer<vtkStructuredGridWriter>::New(); 
	writer->SetFileName(fname.c_str()); 
	writer->SetFileTypeToBinary();
	writer->SetInput(sgrid); 
	return writer->Write();
}

const string CVtk3DVFIOPlugin::do_get_descr() const
{
	return "3D Vector field in- and output (experimental). No attributes are stored or read."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CVtk3DVFIOPlugin();
}
