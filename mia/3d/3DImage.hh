/*  -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
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

#ifndef mia_3d_3dimage_hh
#define mia_3d_3dimage_hh

#include <mia/3d/3DVectorfield.hh>
#include <mia/3d/orientation.hh>
#include <mia/2d/2DImage.hh>

#include <mia/core/attributes.hh>
#include <mia/core/pixeltype.hh>
#include <mia/core/filter.hh>


NS_MIA_BEGIN

/**
   The base type of a 3D image with is used to move around the images in the code.
   This type is not prepared to hold actual data. Instead the templated type T3DImage derived from this class
   is used to hold image data of a certain voxel type.
*/

class EXPORT_3D C3DImage  :  public CAttributedData{

protected:
	C3DImage(const CAttributedData& data, EPixelType type);
	C3DImage(EPixelType type);
	C3DImage();
public:
	static const char *type_descr;
	typedef C3DBounds dimsize_type;
	typedef std::shared_ptr<C3DImage > Pointer;

	virtual  ~C3DImage();


	/**
	   \returns the type of the pixels(voxels) in the image
	 */
	EPixelType get_pixel_type() const;

	/**
	   \returns the 3D size of the image
	 */
	virtual const C3DBounds& get_size() const =  0;

	/**
	   \returns a clone of the image with shared image data (copy-on-write is employed for iterator access)
	 */
	virtual Pointer clone() const = 0;

	///  \returns the voxel size in world units

	virtual C3DFVector get_voxel_size() const = 0;

	/// set the voxel size on world units
	virtual void set_voxel_size(const C3DFVector& voxel) = 0;

	
	E3DImageOrientation get_orientation() const;

	void set_orientation(E3DImageOrientation orient);

private:
	EPixelType _M_pixel_type;
};

/// define a shortcut to the 3D image shared pointer. 
typedef C3DImage::Pointer P3DImage;

/**
   This template defines a 3D image holding a certain type \a T as pixel type. 
*/
template <typename T>
class EXPORT_3D T3DImage : public C3DImage {
public:
	typedef C3DImage Super;

	typedef typename T3DDatafield<T>::iterator iterator;
	typedef typename T3DDatafield<T>::const_iterator const_iterator;
	typedef typename T3DDatafield<T>::const_reference const_reference;
	typedef typename T3DDatafield<T>::reference reference;
	typedef typename T3DDatafield<T>::const_pointer const_pointer;
	typedef typename T3DDatafield<T>::pointer pointer;
	typedef typename T3DDatafield<T>::value_type value_type;
	typedef typename T3DDatafield<T>::difference_type difference_type;
	typedef typename T3DDatafield<T>::size_type size_type;

	/**
	   Construct a new image of a given size and with the given input date.
	   \param size
	   \param init_data input data
	 */
	T3DImage(const C3DBounds& size, const T* init_data);

	/**
	   Construct a new image of a given size and with the given attribute list.
	   \param size
	   \param attr
	 */
	T3DImage(const C3DBounds& size, const CAttributedData& attr);
	/**
	   Construct a new image of a given size
	   \param size
	 */
	T3DImage(const C3DBounds& size);

	//T3DImage(const T3DDatafield<T>& size);

	/**
	   copy constructor
	 */
	T3DImage(const T3DImage& orig);

	/**
	   standart costructor creates an image of size (0,0,0)
	 */
	T3DImage();


	/// \returns a clone of the image with shared image data
	virtual Pointer clone() const;

        /** Get some Data along some line parallel to X axis */
        void get_data_line_x(int y, int z, std::vector<T>& buffer)const;

        /** Get some Data along some line parallel to Y axis */
        void get_data_line_y(int x, int z, std::vector<T>& buffer)const;

        /** Get some Data along some line parallel to Z axis */
        void get_data_line_z(int x, int y, std::vector<T>& buffer)const;

        /** Put some Data along some line parallel to X axis */
        void put_data_line_x(int y, int z, const std::vector<T> &buffer);

        /** Put some Data along some line parallel to Y axis */
        void put_data_line_y(int x, int z, const std::vector<T> &buffer);

        /** Put some Data along some line parallel to Z axis */
        void put_data_line_z(int x, int y, const std::vector<T> &buffer);


	T2DImage<T> get_data_plane_xy(size_t  z)const;

        T2DImage<T> get_data_plane_yz(size_t  x)const;

        T2DImage<T> get_data_plane_xz(size_t  y)const;

	void put_data_plane_xy(size_t  z, const T2DImage<T>& p);

        void put_data_plane_yz(size_t  x, const T2DImage<T>& p);

        void put_data_plane_xz(size_t  y, const T2DImage<T>& p);



	/// element access operator - read only
	const_reference operator()(size_t  x, size_t  y, size_t  z) const {
		return _M_image(x,y,z);
	}

	/// element access operator - read/write
	reference operator()(size_t  x, size_t  y, size_t  z){
		return _M_image(x,y,z);
	}

	/// element access operator - read only
	const_reference operator()(const C3DBounds& l) const{
		return _M_image(l.x,l.y, l.z);
	}

	/// element access operator - read/write
	reference operator()(const C3DBounds& l){
		return _M_image(l.x,l.y, l.z);
	}

	/// constant iterator
	const_iterator begin()const {
		return _M_image.begin();
	}

	/// constant iterator
	const_iterator end()const {
		return _M_image.end();
	}

	/// \returns the all over number of pixels/voxels
	size_t size() const;

	/// read/write iterator, issues copy-on-write
	iterator begin() {
		return _M_image.begin();
	}

	/// read/write iterator, issues copy-on-write
	iterator end() {
		return _M_image.end();
	}

	/// constant iterator starting at the given location
	const_iterator begin_at(size_t x, size_t y, size_t z) const {
		return _M_image.begin_at(x,  y, z);
	}

	/// read/write iterator starting at the given location
	iterator begin_at(size_t x, size_t y, size_t z) {
		return _M_image.begin_at(x,  y,  z);
	}

        /** a linear read only access operator */
        const_reference operator[](int i)const
        {
		return _M_image[i];
        }

        /** A linear read/write access operator. The refcount of Data must be 1,
            else the program will abort with a failed assertion (if assert is enabled)
        */
        reference operator[](int i)
        {
		return _M_image[i];
        }

	/// read only access to the underlying data
	const T3DDatafield<T>& data() const;

	/// read/write access to the underlying data
	//T3DDatafield<T>& data();

	/**
	   Evaluate the gradient at a given location of the image
	   \todo evil, remove it
	 */
	C3DFVector get_gradient(int index) const
	{
		return _M_image.template get_gradient<float>(index);
	}

	/// \returns the 3D size of the image
	virtual const C3DBounds& get_size() const;

	virtual C3DFVector get_voxel_size() const;

	/// set the voxel size on world units
	virtual void set_voxel_size(const C3DFVector& voxel);

private:
	T3DDatafield<T> _M_image;
};

/**
   Stand alone function to evaluate the gradient of an image using finite differences. 
   The gradient at the boundaries is set to zero. 
   \param image 
   \returns a 3D vector field holding the gradient of the input image
 */

EXPORT_3D C3DFVectorfield get_gradient(const C3DImage& image);


typedef T3DImage<bool> C3DBitImage;
typedef T3DImage<signed char> C3DSBImage;
typedef T3DImage<unsigned char> C3DUBImage;
typedef T3DImage<signed short> C3DSSImage;
typedef T3DImage<unsigned short> C3DUSImage;
typedef T3DImage<signed int> C3DSIImage;
typedef T3DImage<unsigned int> C3DUIImage;
#ifdef HAVE_INT64
typedef T3DImage<mia_int64> C3DSLImage;
typedef T3DImage<mia_uint64> C3DULImage;
#endif
typedef T3DImage<float> C3DFImage;
typedef T3DImage<double> C3DDImage;

template <>
struct Binder<C3DImage> {
	typedef __bind_all<T3DImage> Derived;
};


template <typename T>
class EXPORT_3D C3DValueAttribute : public CAttribute {
public:
	C3DValueAttribute(const T3DVector<T>& value);

	operator T3DVector<T>()const;

	const char *typedescr() const	{
		return typeid(T3DVector<T>).name();
	}
private:
	std::string do_as_string() const;
	bool do_is_equal(const CAttribute& other) const;
	bool do_is_less(const CAttribute& other) const;
	T3DVector<T> _M_value;
};

template <typename T>
class EXPORT_3D C3DValueAttributeTranslator: public CAttrTranslator {
public:
	static  void register_for(const std::string& key);
private:
	PAttribute do_from_string(const std::string& value) const;
};

typedef C3DValueAttribute<float> CVoxelAttribute;
typedef C3DValueAttributeTranslator<float> CVoxelAttributeTranslator;
typedef C3DValueAttribute<int> C3DIntAttribute;
typedef C3DValueAttributeTranslator<int> C3DIntAttributeTranslator;



NS_MIA_END

#endif
