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

#ifndef mia_3d_image_hh
#define mia_3d_image_hh

#include <mia/3d/vectorfield.hh>
#include <mia/3d/orientation.hh>
#include <mia/2d/image.hh>

#include <mia/core/attributes.hh>
#include <mia/core/pixeltype.hh>
#include <mia/core/filter.hh>


NS_MIA_BEGIN

/**
   @ingroup basic 
   @brief The generic base type of a 3D image 
   
   The base type of a 3D image with is used to move around the images in the code.
   This type is not prepared to hold actual data. Instead the templated type T3DImage derived from this class
   is used to hold image data of a certain voxel type.
*/

class EXPORT_3D C3DImage  :  public CAttributedData{
protected:
	/**
	   Constructor to create the base sceleton of the image 
	   @param data attributes (meta data) that belong to the image but is normally not required for 
	   processing 
	   @param type pixel type of this image  
	 */
	C3DImage(const CAttributedData& data, EPixelType type);
	/**
	   Constructor to create the base sceleton of the image 
	   @param type pixel type of this image  
	*/
	C3DImage(EPixelType type);
	
	/// standard constructor 
	C3DImage();
public:
	/// data type description for the plug-in path component 
	static const char *data_descr;
	
	/// generic type for the dimension of the image 
	typedef C3DBounds dimsize_type;

	/// Pointer type of the image 
	typedef std::shared_ptr<C3DImage > Pointer;

	virtual  ~C3DImage();

	/**
	   \returns the type of the pixels(voxels) in the image
	 */
	EPixelType get_pixel_type() const;


	/**
	   \returns the number of voxels 
	 */
	virtual size_t size() const =  0;

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

	///@returns the orientation of the image 
	C3DOrientationAndPosition get_orientation() const;

	/** Set the orientation of the image 
	    @param orient 
	    @remark orientation is currently not really used
	*/
	void set_orientation(const C3DOrientationAndPosition& orient);
private:
	EPixelType m_pixel_type;
};

/// define a shortcut to the 3D image shared pointer. 
typedef C3DImage::Pointer P3DImage;


/// helper type for image series 
typedef std::vector<P3DImage> C3DImageSeries;
typedef std::shared_ptr<C3DImageSeries> P3DImageSeries; 

/**
   @ingroup basic 
   @brief Specific type of the 3D images that hold real pixel data 

   This template defines a 3D image holding a certain type \a T as pixel type. 
*/
template <typename T>
class EXPORT_3D T3DImage : public C3DImage {
public:
	/// define the super class of this class for generic processing 
	typedef C3DImage Super;

	/// \cond SELFEXPLAINING 
	typedef typename T3DDatafield<T>::iterator iterator;
	typedef typename T3DDatafield<T>::const_iterator const_iterator;
	typedef typename T3DDatafield<T>::const_reference const_reference;
	typedef typename T3DDatafield<T>::reference reference;
	typedef typename T3DDatafield<T>::const_pointer const_pointer;
	typedef typename T3DDatafield<T>::pointer pointer;
	typedef typename T3DDatafield<T>::value_type value_type;
	typedef typename T3DDatafield<T>::difference_type difference_type;
	typedef typename T3DDatafield<T>::size_type size_type;
	typedef typename T3DDatafield<T>::range_iterator range_iterator; 
	typedef typename T3DDatafield<T>::const_range_iterator const_range_iterator; 
	typedef	typename T2DDatafield<T>::data_array data_array;
	/// \endcond
	
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


	T3DImage(const C3DBounds& size, const data_array& init_data); 
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

	/// forwarding function to access the underlying T3DDatafield
	T2DImage<T> get_data_plane_xy(size_t  z)const;

	/// forwarding function to access the underlying T3DDatafield
        T2DImage<T> get_data_plane_yz(size_t  x)const;

	/// forwarding function to access the underlying T3DDatafield
        T2DImage<T> get_data_plane_xz(size_t  y)const;

	/// forwarding function to access the underlying T3DDatafield
	void put_data_plane_xy(size_t  z, const T2DImage<T>& p);

	/// forwarding function to access the underlying T3DDatafield
        void put_data_plane_yz(size_t  x, const T2DImage<T>& p);

	/// forwarding function to access the underlying T3DDatafield
        void put_data_plane_xz(size_t  y, const T2DImage<T>& p);

	
	/// element access operator - read only
	const_reference operator()(size_t  x, size_t  y, size_t  z) const {
		return m_image(x,y,z);
	}

	/// element access operator - read/write
	reference operator()(size_t  x, size_t  y, size_t  z){
		return m_image(x,y,z);
	}

	/// element access operator - read only
	const_reference operator()(const C3DBounds& l) const{
		return m_image(l.x,l.y, l.z);
	}

	/// element access operator - read/write
	reference operator()(const C3DBounds& l){
		return m_image(l.x,l.y, l.z);
	}

	/// constant iterator
	const_iterator begin()const {
		return m_image.begin();
	}

	/// constant iterator
	const_iterator end()const {
		return m_image.end();
	}

	/// forwarding function to access the specified range of the underlying T3DDatafield 
	range_iterator begin_range(const C3DBounds& begin, const C3DBounds& end) {
		return m_image.begin_range(begin, end); 
	}
	
	/// forwarding function to access the specified range of the underlying T3DDatafield 
	range_iterator end_range(const C3DBounds& begin, const C3DBounds& end){
		return m_image.end_range(begin, end); 
	}

	/// forwarding function to access the specified range of the underlying T3DDatafield 
	const_range_iterator begin_range(const C3DBounds& begin, const C3DBounds& end)const {
		return m_image.begin_range(begin, end); 
	}

	/// forwarding function to access the specified range of the underlying T3DDatafield 
	const_range_iterator end_range(const C3DBounds& begin, const C3DBounds& end) const{
		return m_image.end_range(begin, end); 
	}



	/// \returns the all over number of pixels/voxels
	size_t size() const;

	/// read/write iterator, issues copy-on-write
	iterator begin() {
		return m_image.begin();
	}

	/// read/write iterator, issues copy-on-write
	iterator end() {
		return m_image.end();
	}

	/// constant iterator starting at the given location
	const_iterator begin_at(size_t x, size_t y, size_t z) const {
		return m_image.begin_at(x,  y, z);
	}

	/// read/write iterator starting at the given location
	iterator begin_at(size_t x, size_t y, size_t z) {
		return m_image.begin_at(x,  y,  z);
	}

        /** a linear read only access operator */
        const_reference operator[](int i)const
        {
		return m_image[i];
        }

        /** A linear read/write access operator. The refcount of Data must be 1,
            else the program will abort with a failed assertion (if assert is enabled)
        */
        reference operator[](int i)
        {
		return m_image[i];
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
		return m_image.template get_gradient<float>(index);
	}

	/// \returns the 3D size of the image
	virtual const C3DBounds& get_size() const;

	/// \returns the physical voxel size 
	virtual C3DFVector get_voxel_size() const;

	/// set the voxel size on world units
	virtual void set_voxel_size(const C3DFVector& voxel);

private:
	T3DDatafield<T> m_image;
};


/**
   \brief functor to convert an image with an abitrary pixel type to single floating point pixels 
   
   This functor provides the often used funcionality to convert a 3D image from 
   any pixel representation to a pixel type O representation.  
   The data is just copied. 
   For conversion with scaling and proepry clamping you should use the convert filter 
   provided through C3DFilterPluginHandler. 
   \tparam O output pixel type
   
 */
template <typename O>
struct FConvert3DImageToPixeltypeO: public TFilter<T3DImage<O> > {

	/**
	   Operator to do the actual conversion. 
	   \param image input image 
	   \returns the image converted floating point pixel values 
	 */
	template <typename T>
	T3DImage<O> operator () (const T3DImage<T> &image) const {
		T3DImage<O> result(image.get_size(), image);
		copy(image.begin(), image.end(), result.begin());
		return result;
	}

	/**
	   Operator to do redirect the base class representation through mia::filter
	   \param image input image 
	   \returns the image converted floating point pixel values 
	 */

	T3DImage<O> operator () (const C3DImage &image) const {
		return filter(*this, image); 
	}

	/**
	   Operator to do redirect the pointer representation through mia::filter
	   \param image input image pointer 
	   \returns the image converted floating point pixel values 
	 */

	T3DImage<O> operator () (P3DImage image) const {
		return filter(*this, *image); 
	}
};

/**
   \brief short name for 3DImage to float pixel repn copy functor

   Since copy-conversion to a floating pixel type image is used often 
   we provide here a typedef for the functor.  
*/
typedef FConvert3DImageToPixeltypeO<float> FCopy3DImageToFloatRepn; 


/**
   @ingroup basic 

   Stand alone function to evaluate the gradient of an image using finite differences. 
   The gradient at the boundaries is set to zero. 
   \param image 
   \returns a 3D vector field holding the gradient of the input image
 */

EXPORT_3D C3DFVectorfield get_gradient(const C3DImage& image);


/// \brief 3D image with binary values 
typedef T3DImage<bool> C3DBitImage;

/// \brief 3D image with signed 8 bit integer values 
typedef T3DImage<signed char> C3DSBImage;

/// \brief 3D image with unsigned 8 bit integer values 
typedef T3DImage<unsigned char> C3DUBImage;

/// \brief 3D image with signed 16 bit integer values 
typedef T3DImage<signed short> C3DSSImage;

/// \brief 3D image with unsigned 16 bit integer values 
typedef T3DImage<unsigned short> C3DUSImage;

/// \brief 3D image with signed 32 bit integer values 
typedef T3DImage<signed int> C3DSIImage;

/// \brief 3D image with unsigned 32 bit integer values 
typedef T3DImage<unsigned int> C3DUIImage;

#ifdef LONG_64BIT

/// \brief 3D image with signed 64 bit integer values 
typedef T3DImage<signed long> C3DSLImage;

/// \brief 3D image with unsigned 64 bit integer values 
typedef T3DImage<unsigned long> C3DULImage;
#endif

/// \brief 3D image with signed 32 bit floating point values 
typedef T3DImage<float> C3DFImage;

/// \brief 3D image with signed 64 bit floating point values 
typedef T3DImage<double> C3DDImage;

/**
   @cond INTERNAL 
   @ingroup traits 
*/

template <>
struct Binder<C3DImage> {
	typedef __bind_all<T3DImage> Derived;
};

/// @endcond

/**
   @ingroup basic 
   @brief a translater for 3D vectors to and from a std::string
*/
template <typename T>
class EXPORT_3D C3DValueAttributeTranslator: public CAttrTranslator {
public:
	static  void register_for(const std::string& key);
private:
	PAttribute do_from_string(const std::string& value) const;
};

/**
   This class is a hack to work around the vista voxel size stringyfied  value. 
   Normaly one would write "x,y,z" but in vista it is "x y z", which means a different 
   translator is needed as compared to a T3DVector. 
   For everything else the T3DVector interpretation is used (based on type_id); 
   

   @ingroup basic 
   @brief a 3D vector value used in attributes 
   @tparam T the data type of the vector elements 
*/
template <typename T>
class EXPORT_3D C3DValueAttribute : public CAttribute {
public:

	/**
	   Constructor to initialize the attribute by using a 3D Vector value 
	   @param value 
	 */
	C3DValueAttribute(const T3DVector<T>& value);
	
	/// \returns the value of the attribute as 3D vector 
	operator T3DVector<T>()const;

	/**
	   Obtain a run-time unique type description of the value type 
	   @returns the typeid of the T3DVector<T>
	 */
	const char *typedescr() const	{
		return typeid(T3DVector<T>).name();
	}
	
	// 
	int type_id() const {
		return 	 attribute_type<T3DVector<T>>::value; 
	}
private:
	std::string do_as_string() const;
	bool do_is_equal(const CAttribute& other) const;
	bool do_is_less(const CAttribute& other) const;
	T3DVector<T> m_value;
};

/**
   @ingroup basic 
   @brief a 3D floating point vector used for the voxel size attribute 
*/
typedef C3DValueAttribute<float> CVoxelAttribute;

/**
   @ingroup basic 
   @brief attribute translator for a 3D floating point vector used for the voxel size
*/
typedef C3DValueAttributeTranslator<float> CVoxelAttributeTranslator;

/**
   @ingroup basic 
   @brief a 3D integer vector
*/
typedef C3DValueAttribute<int> C3DIntAttribute;

/**
   @ingroup basic 
   @brief attribute translator for a 3D integer vector
*/
typedef C3DValueAttributeTranslator<int> C3DIntAttributeTranslator;

NS_MIA_END

#endif
