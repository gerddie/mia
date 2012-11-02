/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <limits>
#include <queue>
#include <stdexcept>
// boost type trains are needed to check whether we are dealing with an integer pixel valued image
#include <boost/type_traits.hpp>

// this is for the definition of the 3D image plugin base classes
#include <mia/3d/filter/growmask.hh>
#include <mia/3d/imageio.hh>


NS_BEGIN(growmask_3dimage_filter)

// don't need to write all the names ...
NS_MIA_USE;
using namespace std;


class C3DDoGrowmask: public mia::C3DFilter {
public:
	C3DDoGrowmask(const C3DBitImage& start_mask, P3DShape neigborhood, float min);
	template <typename T>
	C3DDoGrowmask::result_type operator () (const mia::T3DImage<T>& data) const;
private:
	template <typename T>
	struct seed_t {
		seed_t( const C3DBounds& _p, T _v):
			p(_p), value(_v) {};
		C3DBounds p;
		T value;
	};

	template <typename T>
	void add_neigborhood(const C3DBounds& pos, const T3DImage<T>& image,
				    const C3DBitImage& mask,
				    T value, queue <seed_t<T> >& pool)const ;
	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
	C3DBitImage m_start_mask;
	P3DShape m_neigborhood;
	float m_min;
};



C3DGrowmask::C3DGrowmask(const mia::C3DImageDataKey& reference, P3DShape neigborhood, float min):
	m_ref(reference),
	m_neigborhood(neigborhood),
	m_min(min)
{
}

template <typename T>
struct __dispatch_growmask {
	static P3DImage apply(const T3DImage<T>& /*start_mask*/, P3DShape /*shape*/, float /*min*/, const C3DImage& /*ref*/) {
		throw invalid_argument("C3DGrowmask::filter: Input mask must be of type bool");
	}
};

template <>
struct __dispatch_growmask<bool> {
	static P3DImage apply(const C3DBitImage& start_mask, P3DShape shape, float min, const C3DImage& ref) {
		return C3DDoGrowmask(start_mask, shape, min).filter(ref);
	}
};

/* This is the work horse operator of the filter. */
template <typename T>
C3DGrowmask::result_type C3DGrowmask::operator () (const mia::T3DImage<T>& data) const
{
	C3DImageIOPlugin::PData in_image_list = m_ref.get();

	if (!in_image_list || in_image_list->empty())
		throw invalid_argument("C3DGrowmask: no image available in data pool");

	P3DImage image = (*in_image_list)[0];

	return __dispatch_growmask<T>::apply(data, m_neigborhood, m_min, *image);
}


/* The actual filter dispatch function calls the filter by selecting the right pixel type through wrap_filter */
P3DImage C3DGrowmask::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image);
}

/* The factory constructor initialises the plugin name, and takes care that the plugin help will show its parameters */
C3DGrowmaskImageFilterFactory::C3DGrowmaskImageFilterFactory():
	C3DFilterPlugin("growmask"),
	m_min(1.0)
{
	add_parameter("ref", new CStringParameter(m_ref_filename, true, "reference image for mask region growing", 
			      &C3DImageIOPluginHandler::instance()));
	add_parameter("shape", make_param(m_shape, "6n", false, "neighborhood mask"));
	add_parameter("min", new CFloatParameter(m_min, -numeric_limits<float>::max(),
						 numeric_limits<float>::max(), false,
						 "lower threshold for mask growing"));
}

/* The factory create function creates and returns the filter with the given options*/
C3DFilter *C3DGrowmaskImageFilterFactory::do_create()const
{
	// load reference image
	C3DImageDataKey ref_data = C3DImageIOPluginHandler::instance().load_to_pool(m_ref_filename);
	return new C3DGrowmask(ref_data,m_shape, m_min);
}

/* This function sreturns a short description of the filter */
const string C3DGrowmaskImageFilterFactory::do_get_descr()const
{
	return "Use an input binary mask and a reference gray scale image to do region growing "
		"by adding the neighborhood pixels of an already added pixel "
		"if the have a lower intensity that is above the given threshold.";
}

/*
  Finally the C-interface function that returns the filter factory. It's linking style is "C"
  to avoid the name-mangling of C++. A clean name is needed, because this function is aquired
  by its "real"(compiled) name.
*/
extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DGrowmaskImageFilterFactory();
}


C3DDoGrowmask::C3DDoGrowmask(const C3DBitImage& start_mask, P3DShape neigborhood, float min):
	m_start_mask(start_mask),
	m_neigborhood(neigborhood),
	m_min(min)
{
	assert(start_mask.get_pixel_type() == it_bit);
}

template <typename T>
void C3DDoGrowmask::add_neigborhood(const C3DBounds& pos, const T3DImage<T>& image,
				    const C3DBitImage& mask,
				    T value, queue <seed_t<T> >& pool) const
{
	for (C3DShape::const_iterator i = m_neigborhood->begin();
	     i != m_neigborhood->end(); ++i) {

		C3DBounds x( pos.x + i->x,pos.y + i->y, pos.z + i->z);

		if (!(x < image.get_size()))
			continue;

		T tv = image(x);
		if ( ( tv <= value && tv >= m_min) && !mask(x) )
			pool.push(seed_t<T>(x, tv));
	}
}


template <typename T>
C3DDoGrowmask::result_type C3DDoGrowmask::operator () (const T3DImage<T>& data) const
{
	queue <seed_t<T> > pool;

	if (data.get_size() != m_start_mask.get_size())
		throw invalid_argument("C3DGrowmask::filter: seed mask and reference must be ofthe same size");
	C3DBitImage *r = new C3DBitImage(m_start_mask);
	r->set_attributes(data.begin_attributes(), data.end_attributes()); 
	P3DImage result(r);

	// first initialize the seed queue
	C3DBitImage::iterator ir = r->begin();
	typename T3DImage<T>::const_iterator d = data.begin();

	C3DBounds pos;
	for (pos.z = 0; pos.z < data.get_size().z; ++pos.z)
		for (pos.y = 0; pos.y < data.get_size().y; ++pos.y)
			for (pos.x = 0; pos.x < data.get_size().x; ++pos.x, ++ir, ++d) {
				if (*ir)
					add_neigborhood(pos, data, *r, *d, pool);
			}

	// then grow
	while (!pool.empty()) {

		seed_t<T> v = pool.front();
		pool.pop();

		if ((*r)(v.p)) // duplicate?
			continue;

		(*r)(v.p) = true;
		add_neigborhood(v.p, data, *r, v.value, pool);

	}
	return result;
}

P3DImage C3DDoGrowmask::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image);
}

NS_END

