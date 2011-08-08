/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <mia/2d/2DImage.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/filter.hh>


NS_MIA_BEGIN
using namespace std;

const char *C2DImage::data_descr = "2dimage";


C2DImage::C2DImage(const C2DBounds& size, EPixelType pixel_type):
	m_size(size),
	m_pixel_type(pixel_type)
{
}

C2DImage::C2DImage(const CAttributedData& attributes, const C2DBounds& size, EPixelType type):
	CAttributedData(attributes),
	m_size(size),
	m_pixel_type(type)
{
}

C2DImage::~C2DImage()
{
}

EPixelType C2DImage::get_pixel_type() const
{
	return m_pixel_type;
}

const C2DBounds& C2DImage::get_size() const
{
	return m_size;
}

C2DFVector C2DImage::get_pixel_size() const
{
        const PAttribute attr = get_attribute("pixel");
        if (!attr) {
                cvinfo() << "C2DImage::get_pixel_size(): pixel size not defined\n";
                return C2DFVector(1,1);
        }

        const TAttribute<C2DFVector> * vs = dynamic_cast<TAttribute<C2DFVector> *>(attr.get());
        if (!vs){
                cvinfo() << "C2DImage::get_pixel_size(): pixel size wrong type\n";
                return C2DFVector(1,1);
        }

        return *vs;
}

void C2DImage::set_pixel_size(const C2DFVector& pixel)
{
        set_attribute("pixel", PAttribute(new TAttribute<C2DFVector>(pixel)));
}


template <typename T>
C2DImage* T2DImage<T>::clone() const
{
	return new T2DImage<T>(*this);
}

template <typename T>
T2DImage<T>::T2DImage(const C2DBounds& size, const T* init_data):
	C2DImage(size, (EPixelType)pixel_type<T>::value),
	m_image(size, init_data)
{
}

template <typename T>
T2DImage<T>::T2DImage(const C2DBounds& size, const typename T2DDatafield<T>::data_array& init_data):
	C2DImage(size, (EPixelType)pixel_type<T>::value),
	m_image(size, init_data)
{
}

template <typename T>
T2DImage<T>::T2DImage(const C2DBounds& size):
	C2DImage(size, (EPixelType)pixel_type<T>::value),
	m_image(size)
{
}

template <typename T>
T2DImage<T>::T2DImage(const T2DDatafield<T>& orig):
	C2DImage(orig.get_size(), (EPixelType)pixel_type<T>::value),
	m_image(orig)

{
}

template <typename T>
T2DImage<T>::T2DImage(const T2DDatafield<T>& orig, const CAttributedData& attr):
	C2DImage(attr, orig.get_size(), (EPixelType)pixel_type<T>::value),
	m_image(orig)
{
}


template <typename T>
T2DImage<T>::T2DImage(const C2DBounds& size, const CAttributedData& attr):
	C2DImage(attr, size, (EPixelType)pixel_type<T>::value),
	m_image(size)
{
}

template <typename T>
T2DImage<T>::T2DImage(const T2DImage<T>& orig):
	C2DImage(orig),
	m_image(orig.m_image)
{
}

template <typename T>
T2DImage<T>::T2DImage():
	C2DImage(C2DBounds(0,0), (EPixelType)pixel_type<T>::value),
	m_image(C2DBounds(0,0))
{
}

template <typename T>
size_t T2DImage<T>::size() const
{
	return m_image.size();
}

template <typename T>
const T2DDatafield<T>& T2DImage<T>::data() const
{
	return m_image;
}


template <typename T>
void T2DImage<T>::get_data_line_x(size_t y, std::vector<T>& buffer) const
{
	m_image.get_data_line_x(y, buffer);
}

template <typename T>
void T2DImage<T>::get_data_line_y(size_t x, std::vector<T>& buffer) const
{
	m_image.get_data_line_y(x, buffer);
}

template <typename T>
void T2DImage<T>::put_data_line_x(size_t y, const std::vector<T>& buffer)
{
	m_image.put_data_line_x(y, buffer);
}

template <typename T>
void T2DImage<T>::put_data_line_y(size_t x, const std::vector<T>& buffer)
{
	m_image.put_data_line_y(x, buffer);
}

//template <typename T>
//T2DDatafield<T>& T2DImage<T>::data()
//{
//	return m_image;
//}

template <class T>
C2DFVector T2DImage<T>::get_gradient(size_t idx) const
{
	return C2DFVector(0.5f * ( m_image[idx + 1] - m_image[idx - 1]),
			  0.5f * ( m_image[idx + m_image.get_size().x] -
				   m_image[(int)idx - (int)m_image.get_size().x]));
}

template <class T>
C2DFVector T2DImage<T>::get_gradient(const T2DVector<float>& p) const
{
	unsigned int   x = (unsigned int  )p.x;
	unsigned int   y = (unsigned int  )p.y;
	const int sizex = get_size().x;

	float xm = p.x - x;  float xp = 1.0f - xm;
	float ym = p.y - y;  float yp = 1.0f - ym;

	const T *help = &m_image[x + sizex * y];

	const T H00  = help[0];
	const T H0_1 = help[ 0 - sizex];
	const T H01  = help[sizex];
	const T H02  = help[ 2 * sizex];

	const T H10  = help[1];
	const T H1_1 = help[1 - sizex];
	const T H11  = help[1+sizex];
	const T H12  = help[1 + 2 * sizex];

	const T H_10 = help[-1];
	const T H_11 = help[sizex - 1];
	const T H21  = help[2 + sizex];
	const T H20  = help[2];

	return
		T2DVector<float>(
			(( xp * ( H10 - H_10 ) + xm * ( H20 - H00 ) ) * yp +
			 ( xp * ( H11 - H_11 ) + xm * ( H21 - H01 ) ) * ym)  * 0.5f,

			(( yp * ( H01 - H0_1 ) + ym * ( H02 - H00 ) ) * xp +
			 ( yp * ( H11 - H1_1 ) + ym * ( H12 - H10 ) ) * xm)  * 0.5f);
}


template <>
C2DFVector T2DImage<bool>::get_gradient(const C2DFVector& p) const
{
	const int sizex = get_size().x;

	unsigned int   x = (unsigned int  )p.x;
	unsigned int   y = (unsigned int  )p.y;

	float xm = p.x - x;  float xp = 1.0f - xm;
	float ym = p.y - y;  float yp = 1.0f - ym;

	int idx = x + get_size().x * y;

	const float H00  = m_image[idx];
	const float H0_1 = m_image[idx - sizex];
	const float H01  = m_image[idx + sizex];
	const float H02  = m_image[idx + 2*sizex];

	const float H10  = m_image[idx + 1];
	const float H1_1 = m_image[idx + 1-sizex];
	const float H11  = m_image[idx + 1+sizex];
	const float H12  = m_image[idx + 1+2*sizex];

	const float H_10 = m_image[idx -1];
	const float H_11 = m_image[idx + sizex-1];
	const float H21  = m_image[idx + 2+sizex];
	const float H20  = m_image[idx + 2];

	return
		T2DVector<float>((( xp * ( H10 - H_10 ) + xm * ( H20 - H00 ) ) * yp +
				  ( xp * ( H11 - H_11 ) + xm * ( H21 - H01 ) ) * ym )  * 0.5f,
				 (( xp * ( H01 - H0_1 ) + xm * ( H02 - H00 ) ) * yp +
				  ( xp * ( H11 - H1_1 ) + xm * ( H12 - H10 ) ) * ym )  * 0.5f);
}


struct FGradientEvaluator: public TFilter<C2DFVectorfield> {
	template <typename T>
	C2DFVectorfield operator ()( const T2DImage<T>& image) const {
		C2DFVectorfield result(image.get_size());
		const C2DFVector zero(0.0f, 0.0f);
		C2DFVectorfield::iterator r = result.begin();

		size_t idx = image.get_size().x;
		fill(r, r + image.get_size().x, zero);
		advance(r, image.get_size().x);

		for (size_t y = 1; y < image.get_size().y - 1 ; ++y)
			for (size_t x = 0; x < image.get_size().x ; ++x, ++r, ++idx) {
				if (x > 0 && x < image.get_size().x - 1)
					*r = image.get_gradient(idx);
				else
					*r = zero;
			}
		fill(r, r + image.get_size().x, zero);
		return result;
	}
};

EXPORT_2D C2DFVectorfield get_gradient(const C2DImage& image)
{
	FGradientEvaluator ge;
	return filter(ge, image);
}

class CImageCompare: public TFilter<bool>  {
public:
	template <typename  T>
	CImageCompare::result_type operator()(const T2DImage<T>& a, const T2DImage<T>& b) const
	{
		return equal(a.begin(), a.end(), b.begin());
	}
};

EXPORT_2D bool operator == (const C2DImage& a, const C2DImage& b)
{
	if (a.get_pixel_type() != b.get_pixel_type())
		return false;

	if (a.get_size() != b.get_size())
		return false;

	CImageCompare compare;

	return mia::filter_equal(compare, a, b);
}

template class T2DImage<bool>;
template class T2DImage<signed char>;
template class T2DImage<unsigned char>;
template class T2DImage<signed short>;
template class T2DImage<unsigned short>;
template class T2DImage<signed int>;
template class T2DImage<unsigned int>;
#ifdef HAVE_INT64
template class T2DImage<mia_int64>;
template class T2DImage<mia_uint64>;
#endif
template class T2DImage<float>;
template class T2DImage<double>;

NS_MIA_END
