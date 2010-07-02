

/* -*- mia-c++  -*-
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

/*! \brief basic type of a plugin handler

A image to gradient norm filter

\file gradnorm.cc
\author Gert Wollny <wollny at acm.org>

*/


#include <limits>
#include <algorithm>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/filter/gradnorm.hh>



NS_BEGIN(gradnorm_2dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;

CGradnorm::CGradnorm()
{
}

template <class T>
CGradnorm::result_type CGradnorm::operator () (const T2DImage<T>& data) const
{
	TRACE("CGradnorm::operator ()");

	const C2DFVectorfield vf = get_gradient(data);

	T2DImage<float> *result = new T2DImage<float>(data.get_size(), data.get_attribute_list());
	float max;

	C2DFVectorfield::const_iterator  pfb = vf.begin();
	C2DFVectorfield::const_iterator  pfe = vf.end();
	if (pfb == pfe)
		throw invalid_argument("Gradnorm: Image should at least contain one pixel");

	T2DImage<float>::iterator i = result->begin();
	max = *i = pfb->norm();
	++i;
	++pfb;
	while (pfb != pfe) {
		float v = *i = pfb->norm();
		if (v > max)
			 max = v;
		++pfb;
		++i;
	}
	cvdebug() << "Gradnorm: max = " << max << "\n";
	if (max > 0) {
		max = 1.0 / max;
		transform(result->begin(), result->end(), result->begin(), bind2nd(multiplies<float>(), max));
	}

	return CGradnorm::result_type(result);
}

CGradnorm::result_type CGradnorm::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DGradnormFilterPlugin::C2DGradnormFilterPlugin():
	C2DFilterPlugin("gradnorm")
{
}

C2DGradnormFilterPlugin::ProductPtr C2DGradnormFilterPlugin::do_create()const
{
	return C2DGradnormFilterPlugin::ProductPtr(new CGradnorm());
}

const string C2DGradnormFilterPlugin::do_get_descr()const
{
	return "2D image to gradient norm filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DGradnormFilterPlugin();
}

NS_END
