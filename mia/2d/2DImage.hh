/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef mia_2d_2dimage_hh
#define mia_2d_2dimage_hh

#include <mia/core/attributes.hh>
#include <mia/core/pixeltype.hh>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/type_traits.hh>
#include <mia/2d/2DVectorfield.hh>

NS_MIA_BEGIN

#define ATTR_IMAGE_KMEANS_CLASSES "kmeans"

/**
\brief This is the base class for 2D images that can hold generic pixel data

This class is the abstract base class for 2D images that can be passed around
without the program knowing, what actual pixel type an image holds.
It provides access to the image size and the pixel type, and it also
is a container for generic attributes.
*/
class EXPORT_2D C2DImage: public CAttributedData {
public:
	/** The type description provides information about the data type that is
        used by the plug-in system */
        static const char *data_descr;

        /// a definition of the image dimension type for
	typedef C2DBounds dimsize_type;

	/// 2D Image pointer type 
	typedef std::shared_ptr<C2DImage > Pointer;

        virtual  ~C2DImage();

        /// \returns the pixel type
	EPixelType get_pixel_type() const;

        /// \returns the size of the image
	const C2DBounds& get_size() const;

        /// \returns the size of a pixel in real-world units
	C2DFVector get_pixel_size() const;

        /// \param  pixel set the pixel real world size to the given value
	void set_pixel_size(const C2DFVector& pixel);

        /**
        returns a copy of this image
        Abstract method
        */
	virtual C2DImage* clone() const __attribute__((warn_unused_result))  = 0;

 protected:
         /** Constructor initializes the size and the pixel type
         */
         C2DImage(const C2DBounds& size, EPixelType type);

         /** Constructor initializes from the attributes, the size and the pixel type
         */
         C2DImage(const CAttributedData& attributes, const C2DBounds& size, EPixelType type);

         /** standard constructor initializes the size to (0,0), and the pixel type
         to "unknown"
         */
         C2DImage();

 private:
	C2DBounds m_size;
	EPixelType m_pixel_type;
};

/// Shared pointer representation of the 2D Image
typedef C2DImage::Pointer P2DImage;

/// helper type for image series 
typedef std::vector<P2DImage> C2DImageSeries;

typedef std::vector<C2DImageSeries> C2DImageSeriesGroup; 
typedef std::map<std::string, C2DImageSeriesGroup> C2DImageGroupedSeries; 


/**
\brief This is the template version of a 2D image that is used for holding real data.

The purpouse of this class is to hold actual pixel data and provide access to it.
*/

template <typename T>
class EXPORT_2D T2DImage : public C2DImage {
public:
	/// define the super class of this class for generic processing 
	typedef C2DImage Super;

	/// \cond SELFEXPLAINING 
	typedef typename T2DDatafield<T>::iterator iterator;
	typedef typename T2DDatafield<T>::const_iterator const_iterator;
	typedef typename T2DDatafield<T>::const_reference const_reference;
	typedef typename T2DDatafield<T>::reference reference;
	typedef typename T2DDatafield<T>::const_pointer const_pointer;
	typedef typename T2DDatafield<T>::pointer pointer;
	typedef typename T2DDatafield<T>::value_type value_type;
	typedef typename T2DDatafield<T>::difference_type difference_type;
	typedef typename T2DDatafield<T>::size_type size_type;
	/// \endcond

	/**
	   Create a 2D image with the given size and initialize it with the given data 
	   \param size 
	   \param init_data must at least be of size (size.x*size.y)
	*/
	T2DImage(const C2DBounds& size, const T* init_data);

	/**
	   Create a 2D image with the given size and initialize it with the given data 
	   \param size 
	   \param init_data must at least be of size (size.x*size.y)
	*/
	T2DImage(const C2DBounds& size, const typename T2DDatafield<T>::data_array& init_data);
	/**
	   Create a 2D image with thegiven size and attach the given meta-data list. 
	   \param size image size 
	   \param attr meta-data to be added 
	 */
	T2DImage(const C2DBounds& size, const CAttributedData& attr);

	/**
	   Create a 2D image with the given size and initialize it with the given data 
	   \param size 
	*/
	T2DImage(const C2DBounds& size);

	/**
	   Copy constructor 
	 */
	T2DImage(const T2DImage& orig);

	/**
	   Constructor to create the image by using a 2D data field 
	   \param orig the input data field 
	*/
	T2DImage(const T2DDatafield<T>& orig);

	/**
	   Constructor to create the image by using a 2D data field and a given meta data set. 
	   \param orig the input data field 
	   \param attr the meta data 
	*/
	T2DImage(const T2DDatafield<T>& orig, const CAttributedData& attr);
	
	T2DImage();

	/**
	   \returns a dynamically created copy of the image
	 */
	virtual C2DImage* clone() const __attribute__((warn_unused_result));
	
	/// forwarding function to access the underlying T2DDatafield
	const_reference operator()(size_t  x, size_t  y) const {
		return m_image(x,y);
	}
	
	/// forwarding function to access the underlying T2DDatafield
	reference operator()(size_t  x, size_t  y){
		return m_image(x,y);
	}
	
	/// forwarding function to access the underlying T2DDatafield
	const_reference operator[](size_t  idx) const {
		return m_image[idx];
	}
	
	/// forwarding function to access the underlying T2DDatafield
	reference operator[](size_t  idx){
		return m_image[idx];
	}
	
	/// forwarding function to access the underlying T2DDatafield
	const_reference operator()(const C2DBounds& l) const{
		return m_image(l.x,l.y);
	}
	
	/// forwarding function to access the underlying T2DDatafield
	reference operator()(const C2DBounds& l){
		return m_image(l.x,l.y);
	}

	/// forwarding function to access the underlying T2DDatafield
	const_iterator begin()const {
		return m_image.begin();
	}

	/// forwarding function to access the underlying T2DDatafield
	const_iterator end()const {
		return m_image.end();
	}

	/// forwarding function to access the underlying T2DDatafield
	iterator begin() {
		return m_image.begin();
	}

	/// forwarding function to access the underlying T2DDatafield
	iterator end() {
		return m_image.end();
	}

	/// forwarding function to access the underlying T2DDatafield
	const_iterator begin_at(size_t x, size_t y)const {
		return m_image.begin_at(x,  y);
	}

	/// forwarding function to access the underlying T2DDatafield
	iterator begin_at(size_t x, size_t y) {
		return m_image.begin_at(x,  y);
	}

	/// forwarding function to access the underlying T2DDatafield
	size_t size() const;

	/// get direct access to the data field 
	const T2DDatafield<T>& data() const;

	/// forwarding function to access the underlying T2DDatafield
	void get_data_line_x(size_t y, std::vector<T>& buffer) const;

	/// forwarding function to access the underlying T2DDatafield
	void get_data_line_y(size_t x, std::vector<T>& buffer) const;
	
	/// forwarding function to access the underlying T2DDatafield
	void put_data_line_x(size_t y, const std::vector<T>& buffer);

	/// forwarding function to access the underlying T2DDatafield
	void put_data_line_y(size_t x, const std::vector<T>& buffer);
	
	/** 
	    evaluate the image gradient afthe given grid position by using centered finite differences 
	    \param idx linear index into the image data 
	    \returns the approximated gradient 
	 */
	C2DFVector get_gradient(size_t idx) const;
	
	/**
	   Evaluate the gradient by using linerly interpolated finite differences 
	   \param p continious coordinate into the image domain 
	   \returns gradient at position p 
	 */
	C2DFVector get_gradient(const C2DFVector& p) const;
private:
	T2DDatafield<T> m_image;
};

class CImageComparePrinter: public TFilter<int> {
public:
	template <typename T, typename S>
	int operator () (const T2DImage<T>& src, const T2DImage<S>& ref) const {
		typename T2DImage<T>::const_iterator si = src.begin();
		typename T2DImage<T>::const_iterator se = src.end();
		typename T2DImage<S>::const_iterator ri = ref.begin();

		while (si != se)
			cvdebug() << *si++ << " expect " << *ri++ << "\n";
		return 0;
	}
};


template <typename S> 
struct plugin_data_type<T2DImage<S> > {
	typedef C2DImage type; 
}; 

/**
   Evaluate if two images are equal in size, pixel type and all its pixels. 
   Meta data is not considered, nor is the pixel size 
   \param a
   \param b
   \returns result of comparison 
   \remark pixel size should probably also compared 
   \remark who calls this function anyway? 
 */
EXPORT_2D bool operator == (const C2DImage& a, const C2DImage& b);

/**
   Evaluate if two images are not equal in size, pixel type or all its pixels. 
   Meta data is not considered, nor is the pixel size. 
*/
inline bool operator != (const C2DImage& a, const C2DImage& b)
{
	return ! (a == b );
}


/**
   Evaluate the gradient of an image using finite differences
   \remark should be changed to use interpolator
 */
EXPORT_2D C2DFVectorfield get_gradient(const C2DImage& image);



/// 2D image with binary values 
typedef T2DImage<bool> C2DBitImage;

/// 2D image with signed 8 bit integer values 
typedef T2DImage<signed char> C2DSBImage;

/// 2D image with unsigned 8 bit integer values 
typedef T2DImage<unsigned char> C2DUBImage;

/// 2D image with signed 16 bit integer values 
typedef T2DImage<signed short> C2DSSImage;

/// 2D image with unsigned 16 bit integer values 
typedef T2DImage<unsigned short> C2DUSImage;

/// 2D image with signed 32 bit integer values 
typedef T2DImage<signed int> C2DSIImage;

/// 2D image with unsigned 32 bit integer values 
typedef T2DImage<unsigned int> C2DUIImage;

#ifdef HAVE_INT64
/// 2D image with signed 64 bit integer values 
typedef T2DImage<mia_int64> C2DSLImage;

/// 2D image with unsigned 64 bit integer values 
typedef T2DImage<mia_uint64> C2DULImage;
#endif

/// 2D image with single precsion floating point values 
typedef T2DImage<float> C2DFImage;

/// 2D image with double  precsion floating point values 
typedef T2DImage<double> C2DDImage;

/**
   \brief Helper class for 2D filter application 
   
   This class specializes the __bind_all template for 2D images 
   to enable the use of  mia::filter, mia::accumulate, and the likes.  
 */
template <>
struct Binder<C2DImage> {
	/// trait to have a common name for all the derived classes
	typedef __bind_all<T2DImage> Derived;
};

/**
   Specialization of the attribute to string conversion for 2D Vectors. 
 */
template <>
struct dispatch_attr_string<C2DFVector> {
	/**
	   Convert the vector to a string 
	   \param value 
	   \returns the values corresponding to the vector elements as separated by spaces 
	 */
	static std::string val2string(const C2DFVector& value) {
		std::stringstream sval;
		sval << value.x << " " << value.y;
		return sval.str();
	}
	/**
	   Convert a string to 2D vector 
	   \param str a string of two values separated by a whitespace 
	   \returns 2D vector with the elements set accordingly 
	 */
	static C2DFVector string2val(const std::string& str) {
		std::istringstream sval(str);
		C2DFVector value;
		sval >> value.x >> value.y;
		return value;
	}
};

/**
   \brief functor to convert an image with an abitrary pixel type to single floating point pixels 
   
   This functor provides the often used funcionality to convert a 2D image from 
   any pixel representation to a single precision floating point representation.  
   The data is just copied. 
   For conversion with scaling and proepry clamping you should use the convert filter 
   provided through C2DFilterPluginHandler. 
   
 */
struct FConvert2DImage2float: public TFilter<C2DFImage> {

	/**
	   Operator to do the actual conversion. 
	   \param image input image 
	   \returns the image converted floating point pixel values 
	 */
	template <typename T>
	C2DFImage operator () (const T2DImage<T> &image) const {
		C2DFImage result(image.get_size());
		copy(image.begin(), image.end(), result.begin());
		return result;
	}
};


/// typedef for the C2DFVector to std::string translator 
typedef TTranslator<C2DFVector> C2DFVectorTranslator;


NS_MIA_END

#endif
