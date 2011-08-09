/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

/* 
   LatexBeginPluginDescription{2D image filters}
   
   \subsection{Morphological filters}
   \label{filter2d:morph}
   
   \begin{description}
   
   \item [Plugin:] dilate, erode, close, open
   \item [Description:] Apply the according morphological operation using a given structuring element 
   \item [Input:] Abitrary gray scale or binary image 
   \item [Output:] The filtered image of the same pixel type and dimension 
   
   \plugtabstart
   shape &  string & definition of the structuring element as provided by the shape plugins \ref{sec:2dshapes} & 
         sphere:r=2    \\
   hint  &  string & A hint to speed up the processing of binary images (black|white), should indicate 
         what kind of pixel is more presnt in the image  & black \\\hline 
   \plugtabend
   
   \end{description}

   LatexEnd  
 */

#include <iomanip>
#include <limits>
#include <mia/2d/shape.hh>
#include <mia/2d/filter/morphological.hh>



NS_BEGIN(morphological_2dimage_filter)

NS_MIA_USE
using namespace std;
using namespace boost;
namespace bfs=boost::filesystem;

void C2DMorphFilterFactory::prepare_path() const
{
	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("..")/bfs::path("shapes"));
	C2DShapePluginHandler::set_search_path(kernelsearchpath);
}

C2DDilate::C2DDilate(P2DShape shape, bool hint):
	m_shape(shape),
	m_more_dark(hint)

{
}

template <typename T>
struct __dispatch_dilate {
	static T2DImage<T> *apply(const T2DImage<T>& image, const C2DShape& shape, bool /*black*/) {
		const C2DBounds& size = image.get_size();

		T2DImage<T> *result = new T2DImage<T>(image);

		typename T2DImage<T>::const_iterator src_i = image.begin();
		typename T2DImage<T>::iterator res_i = result->begin();

		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x,  ++src_i, ++res_i) {

				C2DShape::const_iterator sb = shape.begin();
				C2DShape::const_iterator se = shape.end();

				while (sb != se) {
					C2DBounds nl(x + sb->x, y + sb->y);
					if (nl < size) {
						T val = image(nl);
						if (*res_i < val )
							*res_i = val;
					}
					++sb;
				}
			}
		return result;
	}
};

template <>
struct __dispatch_dilate<bool> {
	static C2DBitImage *apply(const C2DBitImage& image, const C2DShape& shape, bool more_black) {

		const C2DBounds& size = image.get_size();
		C2DBitImage *result = 0;

		if (more_black) {
			result = new C2DBitImage(size, image.get_attribute_list());
			C2DBitImage::const_iterator src_i = image.begin();
			for (size_t y = 0; y < size.y; ++y)
				for (size_t x = 0; x < size.x; ++x,  ++src_i) {
					if ( *src_i ) {
						C2DShape::const_iterator sb = shape.begin();
						C2DShape::const_iterator se = shape.end();

						while (sb != se) {
							C2DBounds nl(x - sb->x, y - sb->y);

							if (nl < size) {
								(*result)(nl) = true;
							}
							++sb;
						}
					}
				}
		}else {
			result = new C2DBitImage(image);
			C2DBitImage::iterator res_i = result->begin();
			for (size_t y = 0; y < size.y; ++y)
				for (size_t x = 0; x < size.x; ++x,  ++res_i) {
					if ( ! *res_i ) {

						C2DShape::const_iterator sb = shape.begin();
						C2DShape::const_iterator se = shape.end();

						while (sb != se) {
							C2DBounds nl(x + sb->x, y + sb->y);
							if (nl < size) {
								if ( image(nl) ) {
									*res_i = true;
									break;
								}
							}
							++sb;
						}
					}
				}

		}
		return result;
	}
};

template <typename T>
typename C2DFilter::result_type C2DDilate::operator () (const T2DImage<T>& image)const
{

	return P2DImage(__dispatch_dilate<T>::apply(image, *m_shape, m_more_dark));
}

C2DFilter::result_type C2DDilate::do_filter (const C2DImage& image)const
{
	return ::mia::filter(*this, image);
}

C2DDilateFilterFactory::C2DDilateFilterFactory():
	C2DMorphFilterFactory("dilate")
{
}


C2DMorphFilterFactory::C2DMorphFilterFactory(const char *name):
	C2DFilterPlugin(name),
	m_shape_descr("sphere:r=2"),
	m_hint("black")
{
	add_parameter("shape", new CStringParameter(m_shape_descr, false, "structuring element"));
	add_parameter("hint", new CStringParameter(m_hint, false, "a hint at the main image content (black|white)"));
}


C2DFilter *C2DMorphFilterFactory::do_create()const
{
	cvdebug() << "create shape from " << m_shape_descr << '\n';
	P2DShape shape(C2DShapePluginHandler::instance().produce(m_shape_descr.c_str()));

	if (!shape)
		throw runtime_error(string("unable to create a shape from '") + m_shape_descr +string("'"));

	bool bhint = true;

	if (m_hint == string("black"))
		bhint = true;
	else if (m_hint == string("white"))
		bhint = false;
	else
		throw invalid_argument(string("hint '") + m_hint + string("' not supported"));
	return dodo_create(shape, bhint);
}


C2DFilter *C2DDilateFilterFactory::dodo_create(P2DShape shape, bool bhint) const
{
	return new C2DDilate(shape, bhint);
}

const string C2DDilateFilterFactory::do_get_descr()const
{
	return "2d image stack dilate filter";
}

C2DErode::C2DErode(P2DShape shape, bool hint):
	m_shape(shape),
	m_more_dark(hint)

{
}

template <typename T>
struct __dispatch_erode {
	static T2DImage<T> *apply(const T2DImage<T>& image, const C2DShape& shape, bool /*black*/) {
		const C2DBounds size = image.get_size();

		T2DImage<T> *result = new T2DImage<T>(image);

		typename T2DImage<T>::const_iterator src_i = image.begin();
		typename T2DImage<T>::iterator res_i = result->begin();


		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x,  ++src_i, ++res_i) {

				C2DShape::const_iterator sb = shape.begin();
				C2DShape::const_iterator se = shape.end();

				while (sb != se) {
					C2DBounds nl(x + sb->x, y + sb->y);
					if (nl < size) {
						T val = image(nl);
						if (*res_i > val )
							*res_i = val;
					}
					++sb;
					}
			}
		return result;
	}
};

template <>
struct __dispatch_erode<bool> {
	static C2DBitImage *apply(const C2DBitImage& image, const C2DShape& shape, bool more_black) {

		const C2DBounds& size = image.get_size();
		C2DBitImage *result = 0;

		if (more_black) {
			result = new C2DBitImage(image);
			C2DBitImage::iterator res_i = result->begin();
			for (size_t y = 0; y < size.y; ++y)
				for (size_t x = 0; x < size.x; ++x,  ++res_i) {
					if ( *res_i ) {

						C2DShape::const_iterator sb = shape.begin();
						C2DShape::const_iterator se = shape.end();

						while (sb != se) {
							C2DBounds nl(x + sb->x, y + sb->y);
							if (nl < size) {
								if ( !image(nl) ) {
									*res_i = false;
									break;
								}
							}
							++sb;
						}
					}
				}
		}else {
			result = new C2DBitImage(size, image.get_attribute_list());
			fill(result->begin(), result->end(), true);
			C2DBitImage::const_iterator src_i = image.begin();
			for (size_t y = 0; y < size.y; ++y)
				for (size_t x = 0; x < size.x; ++x,  ++src_i) {

					if ( !*src_i ) {
						C2DShape::const_iterator sb = shape.begin();
						C2DShape::const_iterator se = shape.end();

						while (sb != se) {
							C2DBounds nl(x - sb->x, y - sb->y);

							if (nl < size) {
								(*result)(nl) = false;
							}
							++sb;
						}
					}
				}


		}
		return result;
	}
};

template <typename T>
typename C2DFilter::result_type C2DErode::operator () (const T2DImage<T>& image)const
{

	return P2DImage(__dispatch_erode<T>::apply(image, *m_shape, m_more_dark));
}

C2DFilter::result_type C2DErode::do_filter (const C2DImage& image)const
{
	return ::mia::filter(*this, image);
}

C2DErodeFilterFactory::C2DErodeFilterFactory():
	C2DMorphFilterFactory("erode")
{
}

C2DFilter *C2DErodeFilterFactory::dodo_create(P2DShape shape, bool bhint) const
{
	return new C2DErode(shape, bhint);
}

const string C2DErodeFilterFactory::do_get_descr()const
{
	return "2d image stack erode filter";
}

C2DOpenClose::C2DOpenClose(P2DShape shape, bool hint, bool open):
	m_erode(shape, hint),
	m_dilate(shape, hint),
	m_open(open)
{
}

P2DImage C2DOpenClose::do_filter(const C2DImage& src) const
{
	if (m_open) {
		P2DImage tmp = m_erode.filter(src);
		return m_dilate.filter(*tmp);
	}else{
		P2DImage tmp = m_dilate.filter(src);
		return m_erode.filter(*tmp);
	}
}


C2DOpenFilterFactory::C2DOpenFilterFactory():
	C2DMorphFilterFactory("open")
{
}

C2DFilter *C2DOpenFilterFactory::dodo_create(P2DShape shape, bool hint) const
{
	return new C2DOpenClose(shape, hint, true);
}

const string C2DOpenFilterFactory::do_get_descr()const
{
	return "morphological open";
}

C2DCloseFilterFactory::C2DCloseFilterFactory():
	C2DMorphFilterFactory("close")
{
}

C2DFilter *C2DCloseFilterFactory::dodo_create(P2DShape shape, bool hint)const
{
	return new C2DOpenClose(shape, hint, false);
}

const string C2DCloseFilterFactory::do_get_descr()const
{
	return "morphological close";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *p = new C2DErodeFilterFactory();
	p->append_interface(new C2DDilateFilterFactory());
	p->append_interface(new C2DOpenFilterFactory());
	p->append_interface(new C2DCloseFilterFactory());
	return p;
}

NS_END
