/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2009 - 2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#ifndef mia_2d_transform_hh
#define mia_2d_transform_hh

#include <mia/core/transformation.hh>
#include <mia/core/filter.hh>
#include <mia/2d/matrix.hh>
#include <mia/2d/2DImage.hh>
#include <mia/2d/interpolator.hh>

NS_MIA_BEGIN


class C2DTransformation;
typedef SHARED_PTR(C2DTransformation) P2DTransformation;

/**
   Base class for 2D transformation
*/

class EXPORT_2D C2DTransformation: public Transformation<C2DImage, C2DInterpolatorFactory> {
public:
	typedef C2DImage Data;
	typedef C2DInterpolatorFactory Interpolator;

	using Transformation<C2DImage, C2DInterpolatorFactory>::operator ();

	C2DTransformation();
	virtual C2DTransformation *clone() const = 0;
	virtual void reinit() const;
	virtual bool save(const std::string& filename, const std::string& type) const = 0;
	virtual P2DTransformation upscale(const C2DBounds& size) const = 0;
	virtual void add(const C2DTransformation& a) = 0;
	virtual void update(float step, const C2DFVectorfield& a) = 0;
	virtual size_t degrees_of_freedom() const = 0;
	virtual void set_identity() = 0;
	virtual C2DFMatrix derivative_at(int x, int y) const = 0;
	virtual C2DFVectorfield translate(const C2DFVectorfield& gradient) const = 0;
	virtual std::vector<float> get_parameters() const = 0; 
	virtual void set_parameters(const std::vector<float>& params) = 0; 
	virtual float get_max_transform() const = 0;
	virtual const C2DBounds& get_size() const = 0;
	virtual float pertuberate(C2DFVectorfield& v) const = 0;
	virtual C2DFVector apply(const C2DFVector& x) const = 0;
	virtual C2DFVector operator () (const C2DFVector& x) const = 0;
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const = 0;
	virtual float divergence() const = 0; 
	virtual float curl() const = 0; 

};

template <typename Transform>
struct C2DTransform : public TFilter<P2DImage> {
	C2DTransform(const C2DInterpolatorFactory& ipf, const Transform& trans):
		_M_ipf(ipf),
		_M_trans(trans){
	}
	template <typename T>
	P2DImage operator ()(const T2DImage<T>& image) const {
		T2DImage<T> *timage = new T2DImage<T>(image.get_size());

		std::auto_ptr<T2DInterpolator<T> > interp(_M_ipf.create(image.data()));

		typename T2DImage<T>::iterator r = timage->begin();
		typename Transform::const_iterator v = _M_trans.begin();

		for (size_t y = 0; y < image.get_size().y; ++y)
			for (size_t x = 0; x < image.get_size().x; ++x, ++r, ++v) {
				*r = (*interp)(*v);
			}

		return P2DImage(timage);
	}
private:
	const C2DInterpolatorFactory& _M_ipf;
	const Transform& _M_trans;
};


template <typename Transform>
P2DImage transform2d(const C2DImage& image, const C2DInterpolatorFactory& ipf, const Transform& trans)
{
	return mia::filter(C2DTransform<Transform>(ipf, trans), image);
}

NS_MIA_END

#endif
