/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <vtk/vtkvf.hh>

#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPointsWriter.h>
#include <vtkStructuredPoints.h>
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


template <typename T>
C3DIOVectorfield *read_field_data(const C3DBounds& size, void *scalars)
{
       cvdebug() << "VTKVF read vector data of type " <<  __type_descr<T>::value << "\n";
       const T *my_scalars = reinterpret_cast<const T *>(scalars);

       if (!my_scalars)
              throw create_exception<logic_error>("CVtk3DImageIOPlugin::load: input image scalar type bogus");

       C3DIOVectorfield *result = new  C3DIOVectorfield(size);
       copy(my_scalars, my_scalars + 3 * result->size(), &(*result)[0].x);
       return result;
}

CVtk3DVFIOPlugin::PData CVtk3DVFIOPlugin::do_load(const string&  filename) const
{
       auto reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
       reader->SetFileName(filename.c_str());
       reader->Update();
       auto iovf = reader->GetOutput();

       if (!iovf)
              return PData();

       int dim = iovf->GetDataDimension();

       if (dim != 3) {
              cvinfo() << "Expect 3 dimensions but got " << dim << "\n";
              return PData();
       }

       auto dims = iovf->GetDimensions();
       C3DBounds size(dims[0], dims[1], dims[2]);
       int components = iovf->GetNumberOfScalarComponents();

       if (components != 3)
              throw create_exception<invalid_argument>("3D Vtkvf load (", filename,
                            "): only 3D vectors are allowed, "
                            "but got ", components, " component elements");

       auto array = iovf->GetScalarPointer();
       C3DIOVectorfield *result_field = nullptr;

       switch 	 (iovf->GetScalarType()) {
       case VTK_FLOAT:
              result_field = read_field_data<float>(size, array);
              break;

       case VTK_DOUBLE:
              result_field = read_field_data<double>(size, array);
              break;

       default:
              throw create_exception<invalid_argument>("3D Vtkvf load (", filename, "): "
                            "data type ", iovf->GetScalarTypeAsString(),
                            "(", iovf->GetScalarType(), ") not supported");
       }

       return PData(result_field);
}

bool CVtk3DVFIOPlugin::do_save(const string& fname, const C3DIOVectorfield& data) const
{
       auto outfield = vtkSmartPointer<vtkStructuredPoints>::New();
       outfield->SetOrigin(0, 0, 0);
       outfield->SetSpacing(1.0, 1.0, 1.0);
       outfield->SetDimensions(data.get_size().x, data.get_size().y, data.get_size().z);
#if  VTK_MAJOR_VERSION < 6
       outfield->SetScalarType(VTK_FLOAT);
       outfield->SetNumberOfScalarComponents(3);
       outfield->AllocateScalars();
#else
       outfield->AllocateScalars(VTK_FLOAT, 3);
#endif
       float *out_ptr =  reinterpret_cast<float *>(outfield->GetScalarPointer());
       copy(&data[0].x, &data[0].x + data.size() * 3, out_ptr);
       auto writer = vtkSmartPointer<vtkStructuredPointsWriter>::New();
       writer->SetFileName(fname.c_str());
       writer->SetFileTypeToBinary();
#if  VTK_MAJOR_VERSION < 6
       writer->SetInput(outfield);
#else
       writer->SetInputData(outfield);
#endif
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
