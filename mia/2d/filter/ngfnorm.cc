/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

#include <limits>
#include <algorithm>

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/nfg.hh>
#include <mia/2d/filter/ngfnorm.hh>



/* 
   LatexBeginPlugin{2D image filters}
   
   \subsection{Norm of a normalized gradient field of the image}
   \label{filter2d:ngfnorm}
   
   \begin{description}
   
   \item [Plugin:] ngfnorm
   \item [Description:] This filter evaluates the norm of a normalized gradient field of the input image
   \item [Input:] An abitrary gray scale image. 
   \item [Input:] The norm of the normalized gradient field of the input image as a single float valued 
             image of the same dimension as th einput image. 
   
   \end{description}
   This plugin doesn't take parameters. 

   LatexEnd  
 */



NS_BEGIN(ngfnorm_2dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;

CNgfnorm::CNgfnorm()
{
}

template <class T>
CNgfnorm::result_type CNgfnorm::operator () (const T2DImage<T>& data) const
{
	TRACE("CNgfnorm::operator ()");

	const C2DFVectorfield vf = get_nfg(data);

	T2DImage<float> *result = new T2DImage<float>(data.get_size(), data.get_attribute_list());
	float max;

	C2DFVectorfield::const_iterator  pfb = vf.begin();
	C2DFVectorfield::const_iterator  pfe = vf.end();
	if (pfb == pfe)
		throw invalid_argument("Ngfnorm: Image should at least contain one pixel");

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
	cvdebug() << "Ngfnorm: max = " << max << "\n";
	if (max > 0) {
		max = 1.0 / max;
		transform(result->begin(), result->end(), result->begin(), bind2nd(multiplies<float>(), max));
	}

	return CNgfnorm::result_type(result);
}

CNgfnorm::result_type CNgfnorm::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DNgfnormFilterPlugin::C2DNgfnormFilterPlugin():
	C2DFilterPlugin("ngfnorm")
{
}

C2DNgfnormFilterPlugin::ProductPtr C2DNgfnormFilterPlugin::do_create()const
{
	return C2DNgfnormFilterPlugin::ProductPtr(new CNgfnorm());
}

const string C2DNgfnormFilterPlugin::do_get_descr()const
{
	return "2D image to normalized-gradiend-field-norm filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DNgfnormFilterPlugin();
}

NS_END
