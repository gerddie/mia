/*
** Copyrigh (C) 2007 Gert Wollny <gert at die.upm.es>
**   E.S.T.I. Telecomunication, Universidad Politecnica, Madrid
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/


#include <iomanip>
#include <limits>
#include <boost/static_assert.hpp>
#include <mia/3d/filter/morphological.hh>

NS_BEGIN(morph_3dimage_filter)

NS_MIA_USE
using namespace std;
using namespace boost;
namespace bfs=boost::filesystem;


void C3DMorphFilterFactory::prepare_path() const
{

}


C3DDilate::C3DDilate(P3DShape shape, bool hint):
	m_shape(shape),
	m_more_dark(hint)

{
}

template <typename T>
struct __dispatch_dilate {
	static T3DImage<T> *apply(const T3DImage<T>& image, const C3DShape& shape, bool /*black*/) {
		const C3DBounds& size = image.get_size();

		T3DImage<T> *result = new T3DImage<T>(image);

		typename T3DImage<T>::const_iterator src_i = image.begin();
		typename T3DImage<T>::iterator res_i = result->begin();

		for (size_t z = 0; z < size.z; ++z)
			for (size_t y = 0; y < size.y; ++y)
				for (size_t x = 0; x < size.x; ++x,  ++src_i, ++res_i) {

					C3DShape::const_iterator sb = shape.begin();
					C3DShape::const_iterator se = shape.end();

					while (sb != se) {
						C3DBounds nl(x + sb->x, y + sb->y, z + sb->z);
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
	static C3DBitImage *apply(const C3DBitImage& image, const C3DShape& shape, bool more_black) {

		const C3DBounds& size = image.get_size();
		C3DBitImage *result = 0;

		if (more_black) {
			result = new C3DBitImage(size, image.get_attribute_list());
			C3DBitImage::const_iterator src_i = image.begin();
			for (size_t z = 0; z < size.z; ++z)
				for (size_t y = 0; y < size.y; ++y)
					for (size_t x = 0; x < size.x; ++x,  ++src_i) {
						if ( *src_i ) {
							C3DShape::const_iterator sb = shape.begin();
							C3DShape::const_iterator se = shape.end();

							while (sb != se) {
								C3DBounds nl(x + sb->x, y + sb->y, z + sb->z);

								if (nl < size) {
									(*result)(nl) = true;
								}
								++sb;
							}
						}
					}
		}else {
			result = new C3DBitImage(image);
			C3DBitImage::iterator res_i = result->begin();
			for (size_t z = 0; z < size.z; ++z)
				for (size_t y = 0; y < size.y; ++y)
					for (size_t x = 0; x < size.x; ++x,  ++res_i) {
						if ( ! *res_i ) {

							C3DShape::const_iterator sb = shape.begin();
							C3DShape::const_iterator se = shape.end();

							while (sb != se) {
								C3DBounds nl(x - sb->x, y - sb->y, z - sb->z);
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
typename C3DFilter::result_type C3DDilate::operator () (const T3DImage<T>& image)const
{
	return P3DImage(__dispatch_dilate<T>::apply(image, *m_shape, m_more_dark));
}

C3DFilter::result_type C3DDilate::do_filter (const C3DImage& image)const
{
	return ::mia::filter(*this, image);
}

C3DDilateFilterFactory::C3DDilateFilterFactory():
	C3DMorphFilterFactory("dilate")
{
}


C3DMorphFilterFactory::C3DMorphFilterFactory(const char *name):
	C3DFilterPlugin(name),
	m_shape_descr("sphere:r=2"),
	m_hint("black")
{
	add_parameter("shape", new CStringParameter(m_shape_descr, false, "structuring element"));
	add_parameter("hint", new CStringParameter(m_hint, false, "a hint at the main image content (black|white)"));
}


C3DFilterPlugin::ProductPtr C3DMorphFilterFactory::do_create()const
{
	cvdebug() << "create shape from " << m_shape_descr << '\n';
	P3DShape shape(C3DShapePluginHandler::instance().produce(m_shape_descr.c_str()));

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


C3DDilateFilterFactory::ProductPtr C3DDilateFilterFactory::dodo_create(P3DShape shape, bool bhint) const
{
	return C3DDilateFilterFactory::ProductPtr (new C3DDilate(shape, bhint));
}

const string C3DDilateFilterFactory::do_get_descr()const
{
	return "2d image stack dilate filter";
}

bool  C3DDilateFilterFactory::do_test() const
{
	return false;
}


C3DErode::C3DErode(P3DShape shape, bool hint):
	m_shape(shape),
	m_more_dark(hint)

{
}

template <typename T>
struct __dispatch_erode {
	static T3DImage<T> *apply(const T3DImage<T>& image, const C3DShape& shape, bool /*black*/) {
		const C3DBounds size = image.get_size();

		T3DImage<T> *result = new T3DImage<T>(image);

		typename T3DImage<T>::const_iterator src_i = image.begin();
		typename T3DImage<T>::iterator res_i = result->begin();

		for (size_t z = 0; z < size.z; ++z)
			for (size_t y = 0; y < size.y; ++y)
				for (size_t x = 0; x < size.x; ++x,  ++src_i, ++res_i) {

					C3DShape::const_iterator sb = shape.begin();
					C3DShape::const_iterator se = shape.end();

					while (sb != se) {
						C3DBounds nl(x + sb->x, y + sb->y, z + sb->z);
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
	static C3DBitImage *apply(const C3DBitImage& image, const C3DShape& shape, bool more_black) {

		const C3DBounds& size = image.get_size();
		C3DBitImage *result = 0;

		if (more_black) {
			result = new C3DBitImage(image);
			C3DBitImage::iterator res_i = result->begin();
			for (size_t z = 0; z < size.z; ++z)
				for (size_t y = 0; y < size.y; ++y)
					for (size_t x = 0; x < size.x; ++x,  ++res_i) {
						if ( *res_i ) {

							C3DShape::const_iterator sb = shape.begin();
							C3DShape::const_iterator se = shape.end();

							while (sb != se) {
								C3DBounds nl(x - sb->x, y - sb->y, z - sb->z);
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
			result = new C3DBitImage(size, image.get_attribute_list());
			fill(result->begin(), result->end(), true);
			C3DBitImage::const_iterator src_i = image.begin();
			for (size_t z = 0; z < size.z; ++z)
				for (size_t y = 0; y < size.y; ++y)
					for (size_t x = 0; x < size.x; ++x,  ++src_i) {

						if ( !*src_i ) {
							C3DShape::const_iterator sb = shape.begin();
							C3DShape::const_iterator se = shape.end();

							while (sb != se) {
								C3DBounds nl(x + sb->x, y + sb->y, z + sb->z);

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
typename C3DFilter::result_type C3DErode::operator () (const T3DImage<T>& image) const
{
	return P3DImage(__dispatch_erode<T>::apply(image, *m_shape, m_more_dark));
}

mia::P3DImage C3DErode::do_filter (const C3DImage& image)const
{
	return ::mia::filter(*this, image);
}

C3DErodeFilterFactory::C3DErodeFilterFactory():
	C3DMorphFilterFactory("erode")
{
}

C3DErodeFilterFactory::ProductPtr C3DErodeFilterFactory::dodo_create(P3DShape shape, bool bhint) const
{
	return C3DErodeFilterFactory::ProductPtr (new C3DErode(shape, bhint));
}

const string C3DErodeFilterFactory::do_get_descr()const
{
	return "2d image stack erode filter";
}

bool C3DErodeFilterFactory::do_test() const
{
	return false;
}

C3DOpenClose::C3DOpenClose(P3DShape shape, bool hint, bool open):
	m_erode(shape, hint),
	m_dilate(shape, hint),
	m_open(open)
{
}

P3DImage C3DOpenClose::do_filter(const C3DImage& src) const
{
	if (m_open) {
		P3DImage tmp = m_erode.filter(src);
		return m_dilate.filter(*tmp);
	}else{
		P3DImage tmp = m_dilate.filter(src);
		return m_erode.filter(*tmp);
	}
}


C3DOpenFilterFactory::C3DOpenFilterFactory():
	C3DMorphFilterFactory("open")
{
}

C3DFilterPlugin::ProductPtr C3DOpenFilterFactory::dodo_create(P3DShape shape, bool hint) const
{
	return C3DFilterPlugin::ProductPtr(new C3DOpenClose(shape, hint, true));
}

const string C3DOpenFilterFactory::do_get_descr()const
{
	return "morphological open";
}

bool C3DCloseFilterFactory::do_test() const
{
	cvwarn() << "C3DOpenFilterFactory not tested\n";
	return true;
}

C3DCloseFilterFactory::C3DCloseFilterFactory():
	C3DMorphFilterFactory("close")
{
}

C3DFilterPlugin::ProductPtr C3DCloseFilterFactory::dodo_create(P3DShape shape, bool hint)const
{
	return C3DFilterPlugin::ProductPtr(new C3DOpenClose(shape, hint, false));
}

const string C3DCloseFilterFactory::do_get_descr()const
{
	return "morphological close";
}

bool C3DOpenFilterFactory::do_test() const
{
	cvwarn() << "C3DOpenFilterFactory not tested\n";
	return true;
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *p = new C3DErodeFilterFactory();
	p->append_interface(new C3DDilateFilterFactory());
	p->append_interface(new C3DOpenFilterFactory());
	p->append_interface(new C3DCloseFilterFactory());
	return p;
}


NS_END
