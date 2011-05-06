/* -*- mia-c++  -*-
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

/*
\author Gert Wollny <wollny at die.upm.ed>

*/


#include <mia/2d/cost/ssd2.hh>

NS_BEGIN(mia_2dcost_ssd2)
using namespace mia;
using namespace std;

template <class VF, template <typename> class Image >
struct FForceAndValue: public TFilter<double> {
	FForceAndValue(const VF& image_gradient, VF& force):
		m_image_gradient(image_gradient),
		m_force(force)
	{
	}

	template <typename T, typename S>
	double operator()( const Image<T>& src, const Image<S>& ref)
	{
		double result = 0.0;

		typename Image<T>::const_iterator isrc = src.begin();
		typename Image<S>::const_iterator iref = ref.begin();
		typename VF::const_iterator igrad = m_image_gradient.begin();
		typename VF::iterator iforce = m_force.begin();

		while (isrc != src.end()) {
			const double delta = *isrc++ - *iref++;
			cvdebug() << "delta = " << delta << " grad = " << *igrad<<"\n";
			*iforce++ = delta * *igrad++;
			result += delta * delta;
			cvdebug() << "delta = " << delta << "\n";
		}

		return 0.5 * result / src.size();
	}
	const C2DFVectorfield& m_image_gradient;
	C2DFVectorfield& m_force;
};


C2DSSDImageCost::C2DSSDImageCost(const mia::C2DImageDataKey& src_key, const mia::C2DImageDataKey& ref_key,
		mia::P2DInterpolatorFactory ipf,
		float weight):
	C2DImageCostBase(src_key, ref_key, ipf, weight)
{
}

double C2DSSDImageCost::do_evaluate_with_images(const C2DImage& floating, const mia::C2DImage& ref,
						mia::C2DFVectorfield& force) const
{
	assert(floating.get_size() == ref.get_size());
	C2DFVectorfield g = get_gradient(floating);

	FForceAndValue<C2DFVectorfield, T2DImage> fav(g, force);
	return mia::accumulate(fav, floating, ref);
}

NS_END
