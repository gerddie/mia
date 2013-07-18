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

#include <mia/core/filter.hh>
#include <mia/3d/image.hh>


NS_MIA_BEGIN

const char *C3DImage::data_descr = "3dimage";

C3DImage::C3DImage(EPixelType pixel_type):
	m_pixel_type(pixel_type)
{
}

C3DImage::C3DImage(const CAttributedData& data, EPixelType type):
	CAttributedData(data),
	m_pixel_type(type)
{
}


C3DImage::~C3DImage()
{
}

EPixelType C3DImage::get_pixel_type() const
{
	return m_pixel_type;
}

C3DOrientationAndPosition C3DImage::get_orientation() const
{
	assert(0 && "to be implemented"); 
}

void C3DImage::set_orientation(const C3DOrientationAndPosition& orient)
{
	assert(0 && "to be implemented"); 
}

template <typename T>
T3DImage<T>::T3DImage(const C3DBounds& size, const T* init_data):
	C3DImage((EPixelType)pixel_type<T>::value),
	m_image(size, init_data)
{
}

template <typename T>
T3DImage<T>::T3DImage(const C3DBounds& size, const data_array& init_data):
	C3DImage((EPixelType)pixel_type<T>::value),
	m_image(size, init_data)
{
}


template <typename T>
T3DImage<T>::T3DImage(const C3DBounds& size, const CAttributedData& attr):
	C3DImage(attr, (EPixelType)pixel_type<T>::value),
	m_image(size)
{
}

template <typename T>
T3DImage<T>::T3DImage(const C3DBounds& size):
	C3DImage((EPixelType)pixel_type<T>::value),
	m_image(size)
{
}

template <typename T>
T3DImage<T>::T3DImage(const T3DImage<T>& orig):
	C3DImage(orig),
	m_image(orig.m_image)
{
}

template <typename T>
C3DImage::Pointer T3DImage<T>::clone() const
{
	return P3DImage(new T3DImage<T>(*this));
}

template <typename T>
void T3DImage<T>::get_data_line_x(int y, int z, std::vector<T>& buffer)const
{
	m_image.get_data_line_x(y, z, buffer);
}

template <typename T>
void T3DImage<T>::get_data_line_y(int x, int z, std::vector<T>& buffer)const
{
	m_image.get_data_line_y(x, z, buffer);
}


template <typename T>
void T3DImage<T>::get_data_line_z(int x, int y, std::vector<T>& buffer)const
{
	m_image.get_data_line_z(x, y, buffer);
}


template <typename T>
void T3DImage<T>::put_data_line_x(int y, int z, const std::vector<T> &buffer)
{
	m_image.put_data_line_x(y, z, buffer);
}


template <typename T>
void T3DImage<T>::put_data_line_y(int x, int z, const std::vector<T> &buffer)
{
	m_image.put_data_line_y(x, z, buffer);
}


template <typename T>
void T3DImage<T>::put_data_line_z(int x, int y, const std::vector<T> &buffer)
{
	m_image.put_data_line_z(x, y, buffer);
}

template <typename T>
T2DImage<T> T3DImage<T>::get_data_plane_xy(size_t  z)const
{
	return  T2DImage<T>(m_image.get_data_plane_xy(z), *this);
}

template <typename T>
T2DImage<T> T3DImage<T>::get_data_plane_yz(size_t  x)const
{
	return  T2DImage<T>(m_image.get_data_plane_yz(x), *this);
}

template <typename T>
T2DImage<T> T3DImage<T>::get_data_plane_xz(size_t  y)const
{
	return  T2DImage<T>(m_image.get_data_plane_xz(y), *this);
}

template <typename T>
void T3DImage<T>::put_data_plane_xy(size_t  z, const T2DImage<T>& p)
{
	m_image.put_data_plane_xy(z, p.data());
}

template <typename T>
void T3DImage<T>::put_data_plane_yz(size_t  x, const T2DImage<T>& p)
{
	m_image.put_data_plane_yz(x, p.data());
}

template <typename T>
void T3DImage<T>::put_data_plane_xz(size_t  y, const T2DImage<T>& p)
{
	m_image.put_data_plane_xz(y, p.data());
}


template <typename T>
T3DImage<T>::T3DImage():
	C3DImage((EPixelType)pixel_type<T>::value),
	m_image(C3DBounds(0,0,0))
{
}

template <typename T>
size_t T3DImage<T>::size() const
{
	return m_image.size();
}

template <typename T>
const T3DDatafield<T>& T3DImage<T>::data() const
{
	return m_image;
}

//template <typename T>
//T3DDatafield<T>& T3DImage<T>::data()
//{
//	return m_image;
//}

template <typename T>
const C3DBounds& T3DImage<T>::get_size() const
{
	return m_image.get_size();
}

template <class T>
void T3DImage<T>::set_voxel_size(const C3DFVector& voxel)
{
	set_attribute("voxel", PAttribute(new CVoxelAttribute(voxel)));
}

template <class T>
C3DFVector T3DImage<T>::get_voxel_size() const
{
	const PAttribute attr = get_attribute("voxel");
	if (!attr) {
		cvinfo() << "T3DImage<T>::get_voxel_size(): voxel size not defined, default to <1,1,1>\n";
		return C3DFVector(1,1,1);
	}

	const CVoxelAttribute * vs = dynamic_cast<const CVoxelAttribute *>(attr.get());
	if (!vs){
		cvinfo() << "T3DImage<T>::get_voxel_size(): voxel size wrong type, default to <1,1,1>\n";
		return C3DFVector(1,1,1);
	}

	return *vs;

}

struct FGetGradient3D: public TFilter< C3DFVectorfield> {
	template <typename T>
	C3DFVectorfield operator () ( const T3DImage<T>& image) const {

		C3DFVectorfield result(image.get_size());

		size_t slice_size = image.get_size().x * image.get_size().y; 
		size_t row_size = image.get_size().x; 
		
		auto v = result.begin(); 
		auto i = image.begin(); 

		++i; 
		*v++ = C3DFVector(); 

		for (size_t x = 1; x < image.get_size().x - 1; ++x, ++v, ++i)
			*v = C3DFVector( 0.5 * (i[1] - i[-1]), 0.0f, 0.0f);
		
		++i; 
		*v++ = C3DFVector(); 

		for (size_t y = 1; y < image.get_size().y - 1; ++y ) {
			*v = C3DFVector( 0.0f, 0.5 * (i[row_size] - i[-row_size]), 0.0f);
			++i; ++v; 
			for (size_t x = 1; x < image.get_size().x - 1; ++x, ++v, ++i) {
				*v = C3DFVector( 0.5 * (i[1] - i[-1]), 
						 0.5 * (i[row_size] - i[-row_size]), 0.0f); 
			}
			*v = C3DFVector( 0.0f, 0.5 * (i[row_size] - i[-row_size]), 0.0f);
			++i; ++v; 
		}

		++i; 
		*v++ = C3DFVector(); 

		for (size_t x = 1; x < image.get_size().x - 1; ++x, ++v, ++i)
			*v = C3DFVector( 0.5 * (i[1] - i[-1]), 0.0f, 0.0f);
		
		++i; 
		*v++ = C3DFVector(); 
		
		for (size_t z = 1; z < image.get_size().z - 1; ++z) {
			*v = C3DFVector(0.0, 0.0, 0.5 * (i[slice_size] - i[-slice_size]));
			++i; ++v;
			
			for (size_t x = 1; x < image.get_size().x - 1; ++x, ++v, ++i)
				*v = C3DFVector( 0.5 * (i[1] - i[-1]), 0.0f, 0.5 * (i[slice_size] - i[-slice_size]));
			
			*v = C3DFVector(0.0, 0.0, 0.5 * (i[slice_size] - i[-slice_size]));
			++i; ++v;

			for (size_t y = 1; y < image.get_size().y - 1; ++y ) {
				*v = C3DFVector( 0.0f, 0.5 * (i[row_size] - i[-row_size]), 
						 0.5 * (i[slice_size] - i[-slice_size]));
				++i; ++v; 
				for (size_t x = 1; x < image.get_size().x - 1; ++x, ++v, ++i) {
					*v = C3DFVector( 0.5 * (i[1] - i[-1]), 
							 0.5 * (i[row_size] - i[-row_size]), 
							 0.5 * (i[slice_size] - i[-slice_size])); 
				}
				*v = C3DFVector( 0.0f, 0.5 * (i[row_size] - i[-row_size]), 
						 0.5 * (i[slice_size] - i[-slice_size]));
				++i; ++v; 
			}
			*v = C3DFVector(0.0, 0.0, 0.5 * (i[slice_size] - i[-slice_size]));
			++i; ++v;
			
			for (size_t x = 1; x < image.get_size().x - 1; ++x, ++v, ++i)
				*v = C3DFVector( 0.5 * (i[1] - i[-1]), 0.0f, 0.5 * (i[slice_size] - i[-slice_size]));
			
			*v = C3DFVector(0.0, 0.0, 0.5 * (i[slice_size] - i[-slice_size]));
			++i; ++v;
		}
		
		++i; 
		*v++ = C3DFVector(); 

		for (size_t x = 1; x < image.get_size().x - 1; ++x, ++v, ++i)
			*v = C3DFVector( 0.5 * (i[1] - i[-1]), 0.0f, 0.0f);
		
		++i; 
		*v++ = C3DFVector(); 

		for (size_t y = 1; y < image.get_size().y - 1; ++y ) {
			*v = C3DFVector( 0.0f, 0.5 * (i[row_size] - i[-row_size]), 0.0f);
			++i; ++v; 
			for (size_t x = 1; x < image.get_size().x - 1; ++x, ++v, ++i) {
				*v = C3DFVector( 0.5 * (i[1] - i[-1]), 
						 0.5 * (i[row_size] - i[-row_size]), 0.0f); 
			}
			*v = C3DFVector( 0.0f, 0.5 * (i[row_size] - i[-row_size]), 0.0f);
			++i; ++v; 
		}
		
		++i; 
		*v++ = C3DFVector(); 

		for (size_t x = 1; x < image.get_size().x - 1; ++x, ++v, ++i)
			*v = C3DFVector( 0.5 * (i[1] - i[-1]), 0.0f, 0.0f);
		
		++i; 
		*v++ = C3DFVector(); 

		
		return result;
	}
};

/**
   Evaluate the gradient of an image by using symmetric finite differences. 
   @param image input image to calculate the gradient from 
   @returns the 3D vector field representing the gradient, boundaries are set to zero. 
*/

C3DFVectorfield get_gradient(const C3DImage& image)
{
	FGetGradient3D gg;
	return filter(gg, image);
}

template class T3DImage<bool>;
template class T3DImage<signed char>;
template class T3DImage<unsigned char>;
template class T3DImage<signed short>;
template class T3DImage<unsigned short>;
template class T3DImage<signed int>;
template class T3DImage<unsigned int>;

#ifdef LONG_64BIT
template class T3DImage<signed long>;
template class T3DImage<unsigned long>;
#endif

template class T3DImage<float>;
template class T3DImage<double>;

template <typename T>
C3DValueAttribute<T>::C3DValueAttribute(const T3DVector<T>& value):
	m_value(value)
{
}

template <typename T>
C3DValueAttribute<T>::operator T3DVector<T>()const
{
	return m_value;
}

template <typename T>
std::string C3DValueAttribute<T>::do_as_string() const
{
	std::stringstream s;
	s << m_value.x << " " << m_value.y << " " << m_value.z;
	return s.str();
}

template <typename T>
bool C3DValueAttribute<T>::do_is_equal(const CAttribute& other) const
{
	const C3DValueAttribute *va = dynamic_cast<const C3DValueAttribute*>(&other);
	if (!va)
		return false;
	return  m_value == va->m_value;
}

template <typename T>
bool C3DValueAttribute<T>::do_is_less(const CAttribute& other) const
{
	const C3DValueAttribute *va = dynamic_cast<const C3DValueAttribute*>(&other);
	if (!va)
		return strcmp(typedescr(), other.typedescr());
	return  m_value < va->m_value;
}

template <typename T>
PAttribute C3DValueAttributeTranslator<T>::do_from_string(const std::string& value) const
{
	std::istringstream inp(value);
	T3DVector<T> v;

	inp >> v.x >> v.y >> v.z;

	if (inp.good() || inp.eof())
		return PAttribute(new C3DValueAttribute<T>(v));
	throw std::invalid_argument(value + " does not represent a voxel size");
}

template <typename T>
void C3DValueAttributeTranslator<T>::register_for(const std::string& key)
{
	static C3DValueAttributeTranslator me;
	me.do_register(key);
}

template class C3DValueAttribute<float>;
template class  EXPORT_3D C3DValueAttributeTranslator<float>;
template class  EXPORT_3D C3DValueAttribute<int>;
template class  EXPORT_3D C3DValueAttributeTranslator<int>;


NS_MIA_END
