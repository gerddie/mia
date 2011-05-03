/* -*- mona-c++  -*-
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

#include <algorithm>
#include <functional>
#include <cmath>
#include <boost/type_traits.hpp>

#include <mia/2d/2dfilter.hh>
#include <libmona/histogram.hh>
#include <libmona/monaAlgorithms.hh>

namespace ianiso_2dimage_filter {
NS_MIA_USE;
using namespace std;

// scale factor for MAD to zero-mean normal distribution  
static const float zmn_weight = 1.4826; 


static char const *plugin_name = "ianiso";
static const CIntOption param_iterations("iter", "number of iterations", 100, 1, 10000); 
static const CFloatOption param_epsilon("epsilon", "delta value to finish iteration", 1, 0.001, 100); 

inline  float psi(float x, float sigma) {
	if (x > sigma || -x > sigma)
		return 0.0; 
	float val = x / sigma; 
	val *= val; 
	val = 1.0 - val; 
	return x * val * val; 
}



class C2DAnisoDiff: public TUnaryImageFilter<int> {
public:
	C2DAnisoDiff(int maxiter, float epsilon); 

	template <class Data2D>
	typename C2DAnisoDiff::result_type operator () (Data2D& data) const ;

	void work(C2DSLImage& work_copy)const; 
private: 
	float estimate_MAD(const C2DSLImage& data, vector<C2DSLImage>& buffer)const; 
	
	float diffuse(C2DSLImage& data, float sigma, float gamma, const vector<C2DSLImage>& buffer)const; 


	int m_maxiter; 
	float m_epsilon; 
};


class CAnisoDiff2DImageFilter: public C2DImageFilterBase {
	C2DAnisoDiff m_filter; 
public:
	CAnisoDiff2DImageFilter(int maxiter, float epsilon);

	virtual void do_filter(P2DImage& image) const;
};

C2DAnisoDiff::C2DAnisoDiff(int maxiter, float epsilon):
	m_maxiter(maxiter),
	m_epsilon(epsilon)
{
}

class CAnisoDiff2DImageFilterFactory: public C2DFilterPlugin {
public: 
	CAnisoDiff2DImageFilterFactory();
	virtual C2DFilterPlugin::ProductPtr create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
};

typedef pair<float, float> HScale; 

inline bool operator < (const HScale& a, const HScale& b)
{
	return a.first < b.first; 
}


/* estimate the MAD */
float C2DAnisoDiff::estimate_MAD(const C2DSLImage& data, vector<C2DSLImage>& buffer)const
{
	
	long img_min, img_max; 
	get_min_max(data.begin(), data.end(), img_min, img_max);

	// this is to avoid some warnings 
	float dist = img_max - img_min; 

	cvdebug() << "Histogram spread = " << dist << " in ["<< (float)img_min <<", "<< (float)img_max  <<"]\n"; 

	vector<unsigned long> histogram(img_max); 

	cvdebug() << "evaluate gradient\n"; 
	C2DSLImage::const_iterator id = data.begin(); 
	advance(id, data.get_size().x); 
	// evaluate the gradient values 
	unsigned int nelements = 0; 

	for (size_t y = 1; y < data.get_size().y - 1; ++y) {
		++id; 

		for (size_t x = 1; x < data.get_size().x - 1; ++x, ++id) {
			if (*id > 0) {
				long idd = *id; 
				
				long d1 = id[-data.get_size().x]  - idd; 
				buffer[0](x,y) = d1; 
				
				long d2 = id[-1]  - idd; 
				buffer[1](x,y) = d2; 
				
				long d3 = id[+1] - idd;
				buffer[2](x,y) = d3; 
				
				long d4 = id[data.get_size().x]  - idd; 
				buffer[3](x,y) = d4; 
			
				++histogram[ d1 > 0 ? d1 : -d1 ];
				++histogram[ d2 > 0 ? d2 : -d2 ];	
				++histogram[ d3 > 0 ? d3 : -d3 ];
				++histogram[ d4 > 0 ? d4 : -d4 ];
				nelements += 4; 
			}
		}
		++id; 
	}
	cvdebug() << "used: " <<nelements << "\n"; 
	cvdebug() << "evaluate median\n"; 
	
	// need to know how many elements we are
	float nelemnts_median = nelements / 2.0;
	
	long median = img_min; 
	float thresh = 0; 
	for (vector<unsigned long>::const_iterator i = histogram.begin(); 
	     (i != histogram.end() && thresh < nelemnts_median); ++i, ++median) {
		thresh += *i;
	}
	
	cvdebug() << "median = " << median << "\n"; 

	// evaluate the distance between median and actual value
	vector<HScale> new_hist;
	float val = img_min;
	for (vector<unsigned long>::const_iterator i = histogram.begin(); 
	     i != histogram.end(); ++i, val += 1.0 ) {
		float v = val - median; 
		new_hist.push_back(HScale( v > 0 ? v : -v, *i)); 
	}

	sort(new_hist.begin(), new_hist.end()); 
	
#if 0
	for (vector<HScale>::const_iterator k = new_hist.begin(); k != new_hist.end(); ++k)
		cvdebug() << k->first << " : " << k->second << "\n"; 
#endif

	
	thresh = 0; 
	vector<HScale>::const_iterator melement = new_hist.begin(); 
	vector<HScale>::const_iterator old_element = new_hist.end(); 
	while (melement != new_hist.end() && thresh < nelemnts_median) {
		thresh += melement->second;
		old_element = melement; 
		++melement; 
	}
	
	// weight the enclosing neighbours
	if (old_element != new_hist.end()) {
		const float f1 = (thresh - nelemnts_median) / old_element->second; 
		return (1.0 - f1)  * melement->first + f1 *  old_element->first; 
	}else
		return melement->first; 
}
float C2DAnisoDiff::diffuse(C2DSLImage& data, float sigma, float gamma, const vector<C2DSLImage>& buffer)const
{
	vector<C2DSLImage::const_iterator> ib(4); 
	
	for (int i = 0; i < 4; ++i)
		ib[i] = buffer[i].begin(); 

	float sum = 0.0; 
	for (C2DSLImage::iterator id = data.begin(); id != data.end();  ++id) {
		if (*id != 0) { // skip all the black pixels, they should be f
			float delta = 0.0; 
			for (int i = 0; i < 4; ++i) {
				delta += psi(*(ib[i]), sigma);
			}
			delta *= gamma; 
			
			const long delta2 = (long)(delta + 0.5); 
			sum += delta2 * delta2; 
		
			*id = *id + delta2;
		}
		for (int i = 0; i < 4; ++i) 
			++ib[i]; 
	}
	return sum; 
}

void C2DAnisoDiff::work(C2DSLImage& work_copy)const 
{
	int iter = 0; 

	transform(work_copy.begin(), work_copy.end(), work_copy.begin(), bind2nd(multiplies<long>(), 16)); 

	vector<C2DSLImage> buffer; 
	for (int i = 0; i < 4; ++i)
		buffer.push_back(C2DSLImage(work_copy.get_size())); 
	
	char endline = cverb.show_debug() ? '\n' : '\r'; 

	float delta; 
	do {
		++iter; 
		cvmsg() <<iter <<": "; 
		float sigma_e = zmn_weight *  estimate_MAD(work_copy, buffer); 
		cvmsg() << " sigma_e = " << sigma_e <<" "; 

		if (sigma_e == 0.0) { // image contains only one colour
			break; 
		}
		float sigma = sqrt(5.0) *  sigma_e; 
		

		
		float gamma = 1.0 / (psi(sigma_e, sigma) * 4.0); 
				     
		cvmsg() <<" gamma = "<<gamma <<" "; 
		delta = diffuse(work_copy, sigma, gamma, buffer); 
		
		cvmsg() <<" delta " << delta << "       " << endline; 
		
		
	} while (delta > m_epsilon && iter < m_maxiter); 
	cvmsg() << '\n'; 
	
}

template <typename Data2D, bool supported>
struct __dispatch {
	static void apply(Data2D& image, const C2DAnisoDiff& diff) {
		throw invalid_argument("input image type not supported by this plugin, please use 'aniso'");
	}
};


template <typename Data2D>
struct __dispatch<Data2D, true> {
	static void apply(Data2D& image, const C2DAnisoDiff& diff) {

		C2DSLImage work_copy(image.get_size());
		copy(image.begin(), image.end(), work_copy.begin()); 
		
		diff.work(work_copy); 
		
		typename Data2D::iterator di = image.begin();
		for (C2DSLImage::const_iterator wi = work_copy.begin();
		     wi != work_copy.end(); ++wi, ++di) {
			*di = static_cast<typename Data2D::value_type>((*wi + 8) / 16); 
		}
	}
};


template <class Data2D>
typename C2DAnisoDiff::result_type C2DAnisoDiff::operator () (Data2D& image) const
{
	const bool is_supported = ::boost::is_integral<typename Data2D::value_type>::value; 
	__dispatch<Data2D, is_supported>::apply(image, *this); 
	return 0; 
}

CAnisoDiff2DImageFilter::CAnisoDiff2DImageFilter(int maxiter, float epsilon):
	m_filter(maxiter, epsilon)
{
}

void CAnisoDiff2DImageFilter::do_filter(P2DImage& image) const
{
	wrap_filter(m_filter, image); 
}

CAnisoDiff2DImageFilterFactory::CAnisoDiff2DImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_iterations); 
	add_help(param_epsilon); 
}

C2DFilterPlugin::ProductPtr CAnisoDiff2DImageFilterFactory::create(const CParsedOptions& options) const
{
	int max_iter = param_iterations.get_value(options); 
	float epsilon = param_epsilon.get_value(options); 
	return C2DFilterPlugin::ProductPtr(new CAnisoDiff2DImageFilter(max_iter, epsilon)); 
}

const string CAnisoDiff2DImageFilterFactory::do_get_descr()const
{
	return "2D image anisotropic filter (with errors!)"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CAnisoDiff2DImageFilterFactory(); 
}






} // end namespace ianiso_2dimage_filter
