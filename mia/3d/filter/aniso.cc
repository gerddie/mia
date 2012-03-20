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

#include <mia/core/msgstream.hh>
#include <mia/core/disctmap.hh>
#include <mia/core/histogram.hh>
#include <mia/3d/3dfilter.hh>

NS_MIA_USE
using namespace std;


typedef float (*FEdgeStopping)(float x, float sigma); 
float psi_tuckey(float x, float sigma) 
{
	if (x > sigma || -x > sigma)
		return 0.0; 
	float val = x / sigma; 
	val *= val; 
	val = 1.0 - val; 
	return x * val * val; 
}

float psi_pm1(float x, float sigma)
{
	const float val = x / sigma; 
	return 2.0f * x / (2.0f + val * val);
}

float psi_pm2(float x, float sigma)
{
	const float val = x / sigma; 
	return x * expf(- val * val * 0.25f); 
}

static const TDictMap<FEdgeStopping>::Table edge_stop_map[] = {
	{ "tuckey", psi_tuckey},
	{ "pm1", psi_pm1},
	{ "pm2", psi_pm2},
	{ NULL, 0}
};

typedef struct {
	int xp, xm, yp, ym, zp, zp; 
} SGradients; 


typedef vector<SGradients> CGradientField; 

typedef std::pair<float, float> HScale; 
inline bool operator < (const HScale& a, const HScale& b)
{
	return a.first < b.first; 
}


template <typename T>
static inline get_gradient(T3DImage<T>::const_iterator i, SGradients& g) 
{
	int idd = *i; 
	g.zm =  idd - i[-nxy];
	g.ym =  idd - i[-size.x];  
	g.xm =  idd - i[-1]; 
	g.xm =  idd - i[ 1]; 
	g.ym =  idd - i[size.x];  
	g.zm =  idd - i[nxy];
}

void test_get_gradient()
{
	unsigned char init_data[9] = { 0,  0, 0,  0, 30,  0, 0,  0, 0, 
				       0, 20, 0, 40, 30, 10, 0, 40, 0, 
				       0,  0, 0,  0, 25,  0, 0,  0, 0 }; 
	C3DBounds size(3,3,3); 
	C3DUBImage ubimage(size, init_data); 
	
	SGradients g; 

	get_gradient(ubimage.begin() + 13, g); 
	assert(g.xm == -10); 
	assert(g.xp == 20); 
	assert(g.ym == 10); 
	assert(g.yp == -10); 
	assert(g.zm ==   0); 
	assert(g.zp ==   5); 
	
}

template <typename T> 
float evaluate_MAD(const T3DImage<T>& image, CGradientField& grad)
{
	assert(grad.size() == image.size()); 

	typedef  CHistogram<CHistogramFeeder<T> > CHisto; 
	CHisto h = CHisto(CHistogramFeeder<T>(0,0,0)); 

	C3DBounds size = image.get_size(); 
	
	size_t nxy = size.x * size.y;
	
	T3DImage<T>::const_iterator i = image.begin() + nxy; 
	CGradientField::iterator    g = grad.begin()  + nxy; 
	
	size_t nelements = 0; 
	
	for (size_t z = 1; z < size.z - 1; ++z) {
		i + = size.x; 
		g + = size.x; 
		for (size_t y = 1; y < size.y - 1; ++y) {
			++id; 
			++g; 
			for (size_t x = 1; x < size.x - 1; ++x, ++i, ++g) {
				if (*id > 0) {
					get_gradient(i, *g); 
					h.push(g->xm > 0 ? g->xm : -g->xm);
					h.push(g->xp > 0 ? g->xp : -g->xp);
					h.push(g->ym > 0 ? g->ym : -g->ym);
					h.push(g->yp > 0 ? g->yp : -g->yp);
					h.push(g->zm > 0 ? g->zm : -g->zm);
					h.push(g->zp > 0 ? g->zp : -g->zp);
					nelements += 6;
				}
			}
			++g;
			++i; 
		}
		i += size.x;
		g += size.x;
	}

	T median = h.median(); 
	
	CHisto h2 = CHisto(CHisto(CHistogramFeeder<T>(0,0,0))); 
	
	T n = 0; 
			   
	for (CHisto::const_iterator i = h.begin(); i != h.end(); ++i, n++) {
		int k = n - median;
		h2.push(k > 0 ? k : -k , *i); 
	}
	
	return h2.median(); 
}
 



class C3DAnisoDiffFilter: public C3DFilter {
public: 
	C3DAnisoDiffFilter(FEdgeStopping es, int maxiter, float eps, float edge_stop); 
	
	void self_test() const; 
private: 
	int   m_maxiter; 
	float m_epsilon; 
	float m_edge_stop; 

}; 

class C3DAnisoFilterFactory: public C3DFilterPlugin {
public: 
	C3DAnisoFilterFactory();
private: 
	virtual C3DFilterPlugin::ProductPtr do_create()const;
	virtual const string do_get_descr()const; 
	virtual void do_test() const; 
	
	TDictMap<EEdgeStop> m_edge_stop_idmap; 

	int   m_maxiter; 
	float m_epsilon; 
	float m_edge_stop; 
	string m_edge_stop_func; 
};

C3DAnisoFilterFactory::C3DAnisoFilterFactory():
	C3DFilterPlugin("anisodiff"),
	m_edge_stop_idmap(edge_stop_map), 
	m_maxiter(100), 
	m_epsilon(1.0), 
	m_edge_stop(5.0), 
	m_edge_stop_func("tuckey")
{
	add_parameter("iter", CParamList::PParameter(new CIntParameter(m_maxiter, 1, numeric_limits<int>::max(), 
								       false, "maximum number of iterations"))); 
	add_parameter("e", CParamList::PParameter(new CFloatParameter(m_epsilon, numeric_limits<float>::min(), numeric_limits<float>::max(), 
								      false, "delta value to finish iteration"))); 
	add_parameter("k", CParamList::PParameter(new CFloatParameter(m_epsilon, -1, numeric_limits<float>::max(), 
								      false, "edge stopping value (<=0.0 = adaptive) ")));
	add_parameter("edgestop", 
		      CParamList::PParameter(new CStringParameter(m_edge_stop_func, false, "Edge stopping function (pm1|pm2|tuckey)")));

}

C3DFilter *C3DAnisoFilterFactory::do_create()const
{
	FEdgeStopping estop = m_edge_stop_idmap.get_value(m_edge_stop_func.c_str()); 

	return new C3DAnisoDiffFilter(estop, m_maxiter, m_epsilon, m_edge_stop); 
}

const string C3DAnisoFilterFactory::do_get_descr()const
{
	return "anisotropic diffusion filter" 
}

void C3DAnisoFilterFactory::do_test() const
{
	test_get_gradient(); 

	// test MAD
	C3DBounds size(7,8,9); 
	C3DUBImage image(size); 
	CGradientField grad(image.size()); 
	SGradients ginit = {0,0,0,0,0,0}; 
	fill(grad.begin(), grad.end(), ginit); 
	usigned char k = 0; 
	
	for (C3DUBImage::iterator i = image.begin(); i != image.end(); ++i, ++k)
		*i = 2 * k; 
	
	
       
	
	


	C3DAnisoDiffFilter aif_tuckey(psi_tuckey, 10, 0.1, 0.0); 
	aif_tuckey.self_test(); 
	
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DAnisoFilterFactory(); 
}

