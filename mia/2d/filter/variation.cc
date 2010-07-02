/* -*- mona-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*! \brief basic type of a plugin handler 

A 2D variation filter plugin 

\file variation_image3d_filter.hh
\author Gert Wollny <wollny at eva.mpg.de>

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <mia/2d/2dfilter.hh>
#include <libmona/filter.hh>


namespace variation_2dimage_filter {
NS_MIA_USE;
using namespace std; 

static char const *plugin_name = "variation";

static const CIntOption param_width("w", "window width for variation evaluation", 1,1,1000);
static const CIntOption param_need_float("float", "output variation as float image", 0,0,1);
static const CIntOption param_scale ("scale", "scale output to use full output intensity range", 0,0,1);

class C2DVarFilter: public C2DFilter {
	int _M_width; 
	bool _M_need_float; 
	bool _M_scale; 
public:
	C2DVarFilter(int hwidth, bool need_float, bool scale);
	
	template <class Data2D>
	typename C2DVarFilter::result_type operator () (const Data2D& data) const;
};

class C2DVarFilterImageFilter: public C2DImageFilterBase {
	C2DVarFilter _M_filter; 
public:
	C2DVarFilterImageFilter(int hwidth, bool need_float, bool scale);

	virtual P2DImage do_filter(const C2DImage& image) const;
};

class C2DVarFilterImageFilterFactory: public C2DFilterPlugin {
public: 
	C2DVarFilterImageFilterFactory();
	virtual C2DFilterPlugin::ProductPtr create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
private: 
//	virtual int do_test() const; 
};


	C2DVarFilter::C2DVarFilter(int hwidth, bool need_float, bool scale): 
	_M_width(hwidth), 
	_M_need_float(need_float),
	_M_scale(scale)
{
}

template <typename T>
struct SRemap {
	SRemap(float mmax):_M_f(mmax == 1.0 ? 1.0 : numeric_limits<T>::max() / mmax){};
	T operator () (float x) const {
		return T(_M_f * x); 
	}
private:
	float _M_f; 
};

template <class Data2D>
typename C2DVarFilter::result_type C2DVarFilter::operator () (const Data2D& data) const
{
	C2DFImage *fimage = new C2DFImage(data.get_size()); 
	
	C2DFImage::iterator r = fimage->begin(); 
	
	float max_val = 0.0f; 
	
	for (int y = 0; y < (int)data.get_size().y; ++y)
		for (int x = 0; x < (int)data.get_size().x; ++x, ++r) {
			*r = 0.0; 
			int n = 0; 
			float sum = 0.0f; 
			float sum2 = 0.0f; 
			
			for (int iy = max(0, y - _M_width); iy < min (y + _M_width, (int)data.get_size().y); ++iy)
				for (int ix = max(0, x - _M_width); ix < min (x + _M_width, (int)data.get_size().x); ++ix) {
					float val = data(ix,iy); 
					++n; 
					sum += val; 
					sum2 += val * val; 
				}
			if (n > 1) 
				*r = sqrt((sum2 - sum * sum / n) / (n - 1)); 
			if (*r > max_val)
				max_val = *r; 
			
		}
	
	if (_M_need_float)
		return P2DImage(fimage); 
	else {
		
		Data2D *result = new Data2D(data.get_size()); 
		cvdebug() << "max val = " << max_val << '\n'; 
		transform(fimage->begin(), fimage->end(), result->begin(), SRemap<typename Data2D::value_type>(_M_scale? max_val : 1.0)); 
		delete fimage; 
		return P2DImage(result); 
	}
}

C2DVarFilterImageFilter::C2DVarFilterImageFilter(int hwidth, bool need_float, bool scale):
		_M_filter(hwidth, need_float, scale)
{
}

P2DImage C2DVarFilterImageFilter::do_filter(const C2DImage& image) const
{
	return wrap_filter(_M_filter, image); 
}

C2DVarFilterImageFilterFactory::C2DVarFilterImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_width); 
	add_help(param_need_float); 
	add_help(param_scale); 
}

C2DFilterPlugin::ProductPtr C2DVarFilterImageFilterFactory::create(const CParsedOptions& options) const
{
	int w = param_width.get_value(options); 
	bool need_float = (param_need_float.get_value(options) == 1);
	bool scale = (param_scale.get_value(options) == 1);
	
	return C2DFilterPlugin::ProductPtr(new C2DVarFilterImageFilter(w, need_float, scale)); 
}

const string C2DVarFilterImageFilterFactory::do_get_descr()const
{
	return "a 2D image variation evaluation filter"; 
}

#if 0
bool C2DVarFilterImageFilterFactory::do_test() const
{
	cvfail() << do_get_descr() <<": NO TEST IMPLEMENTED\n"; 
	return true; 
}
#endif

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DVarFilterImageFilterFactory(); 
}






} // end namespace variation_2dimage_filter
