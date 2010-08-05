/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#ifndef mia_3d_transform_hh
#define mia_3d_transform_hh

#include <mia/core/transformation.hh>
#include <mia/core/filter.hh>
#include <mia/3d/3DImage.hh>
#include <mia/3d/interpolator.hh>

NS_MIA_BEGIN


class C3DTransformation;
typedef SHARED_PTR(C3DTransformation) P3DTransformation;

/**
   Base class for 3D transformation
*/

class EXPORT_3D C3DTransformation: public Transformation<C3DImage, C3DInterpolatorFactory> {
public:
	C3DTransformation();

	bool save(const std::string& filename, const std::string& type) const;

	P3DTransformation upscale(const C3DBounds& size) const;

	void add(const C3DTransformation& a);

	size_t degrees_of_freedom() const;

	virtual const C3DBounds& get_size() const = 0;
private:

	virtual bool do_save(const std::string& filename, const std::string& type) const = 0;

	virtual P3DTransformation do_upscale(const C3DBounds& size) const = 0;

	virtual void do_add(const C3DTransformation& a) = 0;

	virtual size_t do_degrees_of_freedom() const = 0;
};

template <typename Transform>
struct C3DTransform : public TFilter<P3DImage> {
	C3DTransform(const C3DInterpolatorFactory& ipf, const Transform& trans):
		_M_ipf(ipf),
		_M_trans(trans){
	}
	template <typename T>
	P3DImage operator ()(const T3DImage<T>& image) const {
		T3DImage<T> *timage = new T3DImage<T>(image.get_size());

		std::auto_ptr<T3DInterpolator<T> > interp(_M_ipf.create(image.data()));


#ifdef __OPENMP
#pragma omp parallel for shared(interp, timage) private(z) schedule(static)
#endif
		for (size_t z = 0; z < image.get_size().z; ++z) {
			typename T3DImage<T>::iterator ra = timage->begin_at(0,0,z);
			typename T3DImage<T>::iterator re = timage->begin_at(0,0,z + 1);
			typename Transform::const_iterator v = _M_trans.begin_at(0,0,z);

			while (ra != re)
				*ra++ = (*interp)(*v++);

		}


		return P3DImage(timage);
	}
private:
	const C3DInterpolatorFactory& _M_ipf;
	const Transform& _M_trans;
};


template <typename Transform>
P3DImage transform3d(const C3DImage& image, const C3DInterpolatorFactory& ipf, const Transform& trans)
{
	return mia::filter(C3DTransform<Transform>(ipf, trans), image);
}

NS_MIA_END

#endif
