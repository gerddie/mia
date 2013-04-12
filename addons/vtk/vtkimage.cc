/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <vtk/vtkimage.hh>

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkBitArray.h>
#include <vtkSignedCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkIntArray.h>
#include <vtkUnsignedIntArray.h>

#ifdef LONG_64BIT
#include <vtkLongArray.h>
#include <vtkUnsignedLongArray.h>
#endif
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h> 

#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>

#include <vtkDataSetWriter.h>
#include <vtkDataSetReader.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>

using namespace mia; 
using namespace std; 
using namespace vtkimage; 

// LONG_64BIT seems to be buggy in vtkDataReader
#ifdef LONG_64BIT
#undef LONG_64BIT
#endif 

template <typename T> 
struct __vtk_data_array {
	typedef void type;
        typedef __false_type supported;			\
};  


#define VTK_ARRAY_TRANSLATE(TYPE, VTK_TYPE) \
	template <>			    \
	struct __vtk_data_array<TYPE> {	    \
		typedef VTK_TYPE type;	    \
		typedef __true_type supported; \
	};


VTK_ARRAY_TRANSLATE(bool, vtkBitArray); 
VTK_ARRAY_TRANSLATE(signed char, vtkSignedCharArray); 
VTK_ARRAY_TRANSLATE(unsigned char, vtkUnsignedCharArray); 
VTK_ARRAY_TRANSLATE(signed short, vtkShortArray); 
VTK_ARRAY_TRANSLATE(unsigned short, vtkUnsignedShortArray); 
VTK_ARRAY_TRANSLATE(signed int, vtkIntArray); 
VTK_ARRAY_TRANSLATE(unsigned int, vtkUnsignedIntArray); 

#ifdef LONG_64BIT
VTK_ARRAY_TRANSLATE(signed long, vtkLongArray); 
VTK_ARRAY_TRANSLATE(unsigned long, vtkUnsignedLongArray); 
#endif

VTK_ARRAY_TRANSLATE(float, vtkFloatArray); 
VTK_ARRAY_TRANSLATE(double, vtkDoubleArray); 



template <typename T> 
C3DImage *read_image(const C3DBounds& size, vtkDataArray *scalars) 
{
	typedef typename __vtk_data_array<T>::type myDataArray; 
	
	myDataArray *my_scalars = dynamic_cast<myDataArray*>(scalars); 
	if (!my_scalars) 
		throw create_exception<logic_error>("CVtk3DImageIOPlugin::load: input image scalar type bogus"); 
	
	T3DImage<T> *result = new  T3DImage<T>(size); 
	auto ir = result->begin(); 
	for (size_t i = 0; i < result->size(); ++i, ++ir) 
		*ir = my_scalars->GetValue(i); 

	return result; 
}

static C3DImage *image_vtk_to_mia(vtkImageData *vtk_image, const string& fname) 
{
	int dim = vtk_image->GetDataDimension();
	if (dim != 3)
		throw create_exception<invalid_argument>("CVtk3DImageIOPlugin::load (", fname ,
							 "): Expect 3 dimensions but got ", dim); 

	int components = vtk_image->GetNumberOfScalarComponents(); 
	if (components != 1) 
		throw create_exception<invalid_argument>("CVtk3DImageIOPlugin::load (", fname ,
							 "): only scalar pixel values are allowed, "
							 "but got ", components, " components"); 
	
	auto dims = vtk_image->GetDimensions();
	C3DBounds size(dims[0], dims[1], dims[2]); 
	
	if (vtk_image->GetNumberOfPoints() != size.product()) 
		throw create_exception<invalid_argument>("CVtk3DImageIOPlugin::load (", fname ,
							 "):Got ", vtk_image->GetNumberOfPoints(), 
							 " values but require ", size.product());

	auto points = vtk_image->GetPointData(); 
	if (!points) 
		throw create_exception<invalid_argument>("CVtk3DImageIOPlugin::load (", fname ,
							 "): No point data available");

	int index; 
	auto array = points->GetArray("image", index); 
	if (!array) 
		throw create_exception<invalid_argument>("CVtk3DImageIOPlugin::load (", fname ,
							 "): no image found");
	
	C3DImage *result_image = nullptr; 
	switch 	 (array->GetDataType()) {
	case VTK_BIT:            result_image=read_image<bool>(size, array); break; 
	case VTK_SIGNED_CHAR:    result_image=read_image<signed char>(size, array); break; 
	case VTK_UNSIGNED_CHAR:  result_image=read_image<unsigned char>(size, array); break; 
	case VTK_SHORT:          result_image=read_image<signed short>(size, array); break; 
	case VTK_UNSIGNED_SHORT: result_image=read_image<unsigned short>(size, array); break; 
	case VTK_INT:            result_image=read_image<signed int>(size, array); break;  
	case VTK_UNSIGNED_INT:   result_image=read_image<unsigned int>(size, array); break; 
#ifdef LONG_64BIT
	case VTK_LONG:           result_image=read_image<signed long>(size, array); break; 
	case VTK_UNSIGNED_LONG:  result_image=read_image<unsigned long>(size, array); break; 
#endif 
	case VTK_FLOAT:          result_image=read_image<float>(size, array); break; 
	case VTK_DOUBLE:         result_image=read_image<double>(size, array); break;  
	default:
		throw create_exception<invalid_argument>("CVtk3DImageIOPlugin::load (", fname ,"): "
							 "data type ", vtk_image->GetScalarTypeAsString(), 
							 "(", array->GetDataType(), ") not supported"); 
	}
	return result_image; 
}


template <typename T, typename supported> 
struct __dispatch_convert {
	static vtkSmartPointer<vtkDataArray> apply(const T3DImage<T>& /*input*/)  {
		throw create_exception<invalid_argument>("CVtk3DImageIOPlugin: no support saving '", 
							 __type_descr<T>::value ,"' pixel values");
	}
}; 

template <typename T> 
struct __dispatch_convert<T, __true_type> {
	static vtkSmartPointer<vtkDataArray> apply (const T3DImage<T>& input)  {
		typedef typename __vtk_data_array<T>::type myDataArray; 
		auto array = myDataArray::New(); 
		array->SetNumberOfComponents(1); 
		array->SetNumberOfTuples(input.size()); 
		
		auto ii = input.begin(); 
		for (size_t i = 0; i < input.size(); ++i, ++ii) 
			array->SetValue(i,*ii); 
		
		return vtkSmartPointer<vtkDataArray>(array); 
	}
}; 


class FSetArray: public TFilter<vtkSmartPointer<vtkDataArray> >  {
public:	

	template <typename T> 
	vtkSmartPointer<vtkDataArray> operator () (const T3DImage<T>& input) const {
		typedef typename __vtk_data_array<T>::supported supported; 
		return __dispatch_convert<T, supported>::apply(input); 
	}
}; 


static vtkSmartPointer<vtkImageData> image_mia_to_vtk(const C3DImage& mia_image) 
{
	auto outimage = vtkSmartPointer<vtkImageData>::New();
	outimage->SetOrigin(0,0,0); 
	auto dx = mia_image.get_voxel_size(); 
	outimage->SetSpacing(dx.x, dx.y, dx.z); 
	outimage->SetDimensions(mia_image.get_size().x, mia_image.get_size().y, mia_image.get_size().z); 

        FSetArray set_array; 
	auto array = mia::filter(set_array, mia_image);
	array->SetName("image"); 
	
	outimage->GetPointData()->AddArray(array); 
	return outimage; 
	
}

CVtk3DImageIOPlugin::CVtk3DImageIOPlugin():
	C3DImageIOPlugin("vtk")
{
	// indicate support for all pixel types available (only scalar types are possible)
	add_supported_type(it_bit);
	add_supported_type(it_sbyte);
	add_supported_type(it_ubyte);
	add_supported_type(it_sshort);
	add_supported_type(it_ushort);
	add_supported_type(it_sint);
	add_supported_type(it_uint);

#ifdef LONG_64BIT
	add_supported_type(it_slong);
	add_supported_type(it_ulong);
#endif 

	add_supported_type(it_float);
	add_supported_type(it_double);

       
	add_suffix(".vtk");
	add_suffix(".VTK");
	add_suffix(".vtkimage");
	add_suffix(".VTKimage");

}

CVtk3DImageIOPlugin::PData CVtk3DImageIOPlugin::do_load(const string&  fname) const
{
	
	auto reader = vtkSmartPointer<vtkDataSetReader>::New(); 
	reader->SetFileName(fname.c_str()); 
	if (!reader->OpenVTKFile()) 
		return CVtk3DImageIOPlugin::PData(); 
	
	reader->Update(); 
	
	auto vtk_image = dynamic_cast<vtkImageData*>(reader->GetOutput());

	if (!vtk_image)
		return PData(); 
	

	CVtk3DImageIOPlugin::PData result(new Data); 	
	result->push_back(P3DImage(image_vtk_to_mia(vtk_image, fname))); 
	return result; 
}





bool CVtk3DImageIOPlugin::do_save(const string& fname, const Data& data) const
{
	if (data.size() != 1)
		throw create_exception<invalid_argument>("CVtk3DImageIOPlugin::save only supports writing one image, "
							 "but you passed in ", data.size(), " images"); 

	const auto& image = *data[0];
	
	auto outimage = image_mia_to_vtk(image); 
	
	// add writing of the image
	auto writer = vtkSmartPointer<vtkDataSetWriter>::New(); 
	writer->SetInput(outimage); 
	writer->SetFileTypeToBinary();
	writer->SetFileName(fname.c_str()); 
	writer->Write();

	return true; 
}

const string CVtk3DImageIOPlugin::do_get_descr() const
{
	return "3D vtk image legacy in- and output (experimental). No attributes besides the voxel size is stored or read.";
}


CVtkXML3DImageIOPlugin::CVtkXML3DImageIOPlugin():
	C3DImageIOPlugin("vti")
{
	// indicate support for all pixel types available (only scalar types are possible)
	add_supported_type(it_bit);
	add_supported_type(it_sbyte);
	add_supported_type(it_ubyte);
	add_supported_type(it_sshort);
	add_supported_type(it_ushort);
	add_supported_type(it_sint);
	add_supported_type(it_uint);

#ifdef LONG_64BIT
	add_supported_type(it_slong);
	add_supported_type(it_ulong);
#endif 

	add_supported_type(it_float);
	add_supported_type(it_double);

       
	add_suffix(".vti");
	add_suffix(".VTI");

}

CVtkXML3DImageIOPlugin::PData CVtkXML3DImageIOPlugin::do_load(const string&  fname) const
{
	
	auto reader = vtkSmartPointer<vtkXMLImageDataReader>::New(); 
	reader->SetFileName(fname.c_str()); 
	reader->Update(); 
	
	auto vtk_image = reader->GetOutput();

	if (!vtk_image)
		return PData(); 
	

	CVtkXML3DImageIOPlugin::PData result(new Data); 	
	result->push_back(P3DImage(image_vtk_to_mia(vtk_image, fname))); 
	return result; 
}





bool CVtkXML3DImageIOPlugin::do_save(const string& fname, const Data& data) const
{
	if (data.size() != 1)
		throw create_exception<invalid_argument>("CVtkXML3DImageIOPlugin::save only supports writing one image, "
							 "but you passed in ", data.size(), " images"); 

	const auto& image = *data[0];
	
	auto outimage = image_mia_to_vtk(image); 
	
	// add writing of the image
	auto writer = vtkSmartPointer<vtkXMLImageDataWriter>::New(); 
	writer->SetInput(outimage); 
	writer->SetFileName(fname.c_str()); 
	writer->Write();

	return true; 
}

const string CVtkXML3DImageIOPlugin::do_get_descr() const
{
	return "3D image XML in- and output (experimental). No attributes besides the voxel size is stored or read.";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	auto plugin =  new CVtk3DImageIOPlugin();
	plugin->append_interface(new CVtkXML3DImageIOPlugin); 
	return plugin; 
}
