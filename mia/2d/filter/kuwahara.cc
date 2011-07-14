/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <limits>
#include <mia/2d/2dfilter.hh>
#include <libmia/filter2dimage.hh>


namespace kuwahara_2dimage_filter {
NS_MIA_USE;
using namespace std; 

static char const * plugin_name = "kuwahara";
static const CIntOption param_hw("w", "filter parameter", 1, 1, numeric_limits<int>::max()); 

class C2DKuwahara : public C2DFilter {
public:
	C2DKuwahara(const CParsedOptions& options); 

	template <class Data2D>
	typename C2DKuwahara::result_type operator () (const Data2D& data) const ;
private: 
	template <class Data2D>
	typename Data2D::value_type run(const Data2D& image, int x, int y)const; 
	
	template <class Data2D>
	void run_sub(const Data2D& image, int cx, int cy, float& mu, float& sigma)const;
	
	int m_l; 
	int m_kh; 

	
};

typedef C2DImageFilter<C2DKuwahara> C2DKuwaharaImageFilter;
//template class mia::C2DImageFilter<C2DKuwahara>; 


class C2DKuwaharaImageFilterFactory: public C2DFilterPlugin {
public: 
	C2DKuwaharaImageFilterFactory();
	virtual C2DFilter *create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
};

C2DKuwahara::C2DKuwahara(const CParsedOptions& options):
	m_l(param_hw.get_value(options)),
	m_kh(2 * m_l +1)
{
}

template <class Data2D>
void C2DKuwahara::run_sub(const Data2D& image, int cx, int cy, float& mu, float& sigma)const
{
	int n = 0; 
	const int ymax = min(cy +  m_kh, (int)image.get_size().y); 
	const int xmax = min(cx +  m_kh, (int)image.get_size().x); 
	mu = 0.0f; 
	sigma = 0.0f; 
	
	for (int y = max(cy, 0);  y < ymax ; ++y) 
		for (int x = max(cx, 0); x < xmax; ++x) {
			++n;
			const float val = image(x,y); 
			mu += val; 
			sigma += val * val; 
		}
	if (n > 1) {
		mu /= n; 
		sigma = (sigma - n * mu * mu) / (n - 1); 
	}else
		sigma = 0; 
}

template <class Data2D>
typename Data2D::value_type C2DKuwahara::run(const Data2D& image, int x, int y)const
{
	float mu[5]; 
	float sigma[5]; 
	
	run_sub(image, x - m_kh + 1, y - m_kh + 1, mu[0], sigma[0]); 
	run_sub(image, x - m_kh + 1, y            , mu[1], sigma[1]); 
	run_sub(image, x            , y - m_kh + 1, mu[2], sigma[2]); 
	run_sub(image, x            , y            , mu[3], sigma[3]);
	run_sub(image, x - m_l     , y - m_l     , mu[4], sigma[4]);
	
	int best = 0; 
	
	for (int i = 1; i < 4; ++i) {
		if (sigma[best] > sigma[i]) {
			best = i; 
		}
	}
	return static_cast<typename Data2D::value_type>(mu[best]); 
}


template <class Data2D>
typename C2DKuwahara::result_type C2DKuwahara::operator () (const Data2D& data) const
{
	cvdebug() << "C2DKuwahara::operator () begin\n";

	Data2D *result = new Data2D(data.get_size()); 

	typename Data2D::iterator i = result->begin(); 
	
	for (int y = 0; y < (int)data.get_size().y; ++y) {
		for (int x = 0; x < (int)data.get_size().x; ++x, ++i)
			*i = run(data, x, y);  
	}
	
	cvdebug() << "C2DKuwahara::operator () end\n";
	return P2DImage(result); 
}

C2DKuwaharaImageFilterFactory::C2DKuwaharaImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_hw); 
}

C2DFilter *C2DKuwaharaImageFilterFactory::create(const CParsedOptions& options) const
{
	return new C2DKuwaharaImageFilter(options); 
}

const string C2DKuwaharaImageFilterFactory::do_get_descr()const
{
	return "2D image kuwahara filter"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DKuwaharaImageFilterFactory(); 
}

} // end namespace kuwahara_2dimage_filter
