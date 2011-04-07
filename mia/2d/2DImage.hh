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
        static const char *type_descr;

        /// a definition of the image dimension type for
	typedef C2DBounds dimsize_type;
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
	virtual C2DImage* clone() const = 0;

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
typedef std::vector<P2DImage> C2DImageSeries;


/**
\brief This is the template version of a 2D image.
The purpouse of this class is to hold actual pixel data and provide access to it.
*/

template <typename T>
class EXPORT_2D T2DImage : public C2DImage {
public:
	typedef C2DImage Super;

	typedef typename T2DDatafield<T>::iterator iterator;
	typedef typename T2DDatafield<T>::const_iterator const_iterator;
	typedef typename T2DDatafield<T>::const_reference const_reference;
	typedef typename T2DDatafield<T>::reference reference;
	typedef typename T2DDatafield<T>::const_pointer const_pointer;
	typedef typename T2DDatafield<T>::pointer pointer;
	typedef typename T2DDatafield<T>::value_type value_type;
	typedef typename T2DDatafield<T>::difference_type difference_type;
	typedef typename T2DDatafield<T>::size_type size_type;

	T2DImage(const C2DBounds& size, const T* init_data);
	T2DImage(const C2DBounds& size, const typename T2DDatafield<T>::data_array& init_data);
	T2DImage(const C2DBounds& size, const CAttributedData& attr);
	T2DImage(const C2DBounds& size);
	T2DImage(const T2DImage& orig);

	T2DImage(const T2DDatafield<T>& orig);
	T2DImage(const T2DDatafield<T>& orig, const CAttributedData& attr);
	T2DImage();
	
	virtual C2DImage* clone() const;
	
	const_reference operator()(size_t  x, size_t  y) const {
		return m_image(x,y);
	}
	
	reference operator()(size_t  x, size_t  y){
		return m_image(x,y);
	}
	
	const_reference operator[](size_t  idx) const {
		return m_image[idx];
	}
	
	reference operator[](size_t  idx){
		return m_image[idx];
	}
	
	const_reference operator()(const C2DBounds& l) const{
		return m_image(l.x,l.y);
	}
	
	reference operator()(const C2DBounds& l){
		return m_image(l.x,l.y);
	}

	const_iterator begin()const {
		return m_image.begin();
	}

	const_iterator end()const {
		return m_image.end();
	}

	iterator begin() {
		return m_image.begin();
	}

	iterator end() {
		return m_image.end();
	}

	const_iterator begin_at(size_t x, size_t y)const {
		return m_image.begin_at(x,  y);
	}

	iterator begin_at(size_t x, size_t y) {
		return m_image.begin_at(x,  y);
	}

	size_t size() const;

	const T2DDatafield<T>& data() const;
	//T2DDatafield<T>& data();

	void get_data_line_x(size_t y, std::vector<T>& buffer) const;

	void get_data_line_y(size_t x, std::vector<T>& buffer) const;
	
	void put_data_line_x(size_t y, const std::vector<T>& buffer);

	void put_data_line_y(size_t x, const std::vector<T>& buffer);
	
	C2DFVector get_gradient(size_t idx) const;
	
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

EXPORT_2D bool operator == (const C2DImage& a, const C2DImage& b);

inline bool operator != (const C2DImage& a, const C2DImage& b)
{
	return ! (a == b );
}


/**
   Evaluate the gradient of an image using finite differences
   \remark should be changed to use interpolator
 */
EXPORT_2D C2DFVectorfield get_gradient(const C2DImage& image);



typedef T2DImage<bool> C2DBitImage;
typedef T2DImage<signed char> C2DSBImage;
typedef T2DImage<unsigned char> C2DUBImage;
typedef T2DImage<signed short> C2DSSImage;
typedef T2DImage<unsigned short> C2DUSImage;
typedef T2DImage<signed int> C2DSIImage;
typedef T2DImage<unsigned int> C2DUIImage;
#ifdef HAVE_INT64
typedef T2DImage<mia_int64> C2DSLImage;
typedef T2DImage<mia_uint64> C2DULImage;
#endif
typedef T2DImage<float> C2DFImage;
typedef T2DImage<double> C2DDImage;


template <>
struct Binder<C2DImage> {
	typedef __bind_all<T2DImage> Derived;
};


template <>
struct dispatch_attr_string<C2DFVector> {
	static std::string val2string(const C2DFVector& value) {
		std::stringstream sval;
		sval << value.x << " " << value.y;
		return sval.str();
	}
	static C2DFVector string2val(const std::string& str) {
		std::istringstream sval(str);
		C2DFVector value;
		sval >> value.x >> value.y;
		return value;
	}
};


struct FConvert2DImage2float: public TFilter<C2DFImage> {

	template <typename T>
	C2DFImage operator () (const T2DImage<T> &image) const {
		C2DFImage result(image.get_size());
		copy(image.begin(), image.end(), result.begin());
		return result;
	}
};

typedef TTranslator<C2DFVector> C2DFVectorTranslator;


NS_MIA_END

#endif
