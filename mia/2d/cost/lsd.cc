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

#include <mia/core/filter.hh>
#include <mia/2d/cost/lsd.hh>
#include <boost/lambda/lambda.hpp>


NS_BEGIN(lsd_2dimage_cost)

using std::sort; 
using std::vector; 
using boost::lambda::_1; 
using namespace mia; 


struct valpos {
	double val; 
	int pos; 
}; 

bool operator < (const valpos& a, const valpos& b)
{
	return a.val < b.val; 
}


C2DLSDImageCost::C2DLSDImageCost()
{
}

class CRefPrepare :public TFilter<void> {
public: 
	CRefPrepare(vector<double>& QtQinv, vector<int>& Q_mappping):
		m_QtQinv(QtQinv), 
		m_Q_mappping(Q_mappping)
		{
		}
	template <typename T> 
	void operator()(const T2DImage<T>& ref); 

	vector<double>& m_QtQinv; 
	vector<int>& m_Q_mappping; 
}; 

template <typename T> 
void CRefPrepare::operator()(const T2DImage<T>& ref)
{
	size_t npixels = ref.get_size().x * ref.get_size().y; 
	vector<valpos> buffer(npixels); 
	m_Q_mappping.resize(npixels); 
	m_QtQinv.resize(npixels); 

	int idx = 0;
	for( auto x = ref.begin(); x != ref.end(); ++x, ++idx) {
		double value = *x; 
		valpos vp = {value, idx}; 
		buffer[idx] = vp; 
	}
	sort(buffer.begin(), buffer.end());
	
	idx = 0;
	auto x = buffer.begin(); 
	double oldv = x->val; 
	++x; 
	++m_QtQinv[idx]; 

	// build histogram and prepare target mapping
	m_Q_mappping[x->pos] = idx; 

	while ( x != buffer.end() )  {
		if (x->val != oldv) {
			oldv = x->val; 
			++idx;
		}
		++m_QtQinv[idx];
		m_Q_mappping[x->pos] = idx; 
		++x; 
	}
	++idx; 
	m_QtQinv.resize(idx);
	transform(m_QtQinv.begin(), m_QtQinv.end(), m_QtQinv.begin(), 1.0/ _1); 
}
	

void C2DLSDImageCost::post_set_reference(const C2DImage& ref)
{
	CRefPrepare rp(m_QtQinv, m_Q_mappping);
	accumulate(rp, ref); 
}


class RunCost : TFilter<double>  {
public: 
	typedef TFilter<double>::result_type result_type; 
	RunCost(float scale, const vector<double>& QtQinv, const vector<int>& Q_mappping); 

	template <typename T> 
	double operator()(const T2DImage<T>& ref)const; 

	template <typename T> 
	double operator()(const T2DImage<T>& ref, C2DFVectorfield& force)const; 
private: 
	float m_scale; 
	const vector<double>& m_QtQinv;
	const vector<int>& m_Q_mappping; 
}; 

double C2DLSDImageCost::do_value(const C2DImage& a, const C2DImage& /*b*/) const
{
	RunCost rf(1.0, m_QtQinv, m_Q_mappping); 
	return mia::filter(rf, a); 
}


double C2DLSDImageCost::do_evaluate_force(const C2DImage& a, const C2DImage& /*b*/, 
					 float scale, C2DFVectorfield& force) const
{
	RunCost rf(scale, m_QtQinv, m_Q_mappping); 
	return mia::filter_and_output(rf, a, force); 
}

RunCost::RunCost(float scale, const vector<double>& QtQinv, const vector<int>& Q_mappping):
	m_scale(scale), 
	m_QtQinv(QtQinv), 
	m_Q_mappping(Q_mappping)
{
}

template <typename T> 
double  RunCost::operator()(const T2DImage<T>& a)const
{
	double val1 = 0.0; 
	double val2 = 0.0; 
	vector<double> sums(m_QtQinv.size(), 0.0); 

	auto idx = m_Q_mappping.begin(); 
	for (auto ia = a.begin(); ia != a.end(); ++ia, ++idx) {
		val1 += *ia * *ia;  
		sums[*idx] += *ia; 
	}
	
	
	for(size_t i = 0; i < sums.size(); ++i)
		val2 += sums[i] * sums[i] * m_QtQinv[i]; 
	
	return 0.5 * (val1 - val2); 
}

template <typename T> 
double  RunCost::operator()(const T2DImage<T>& a, C2DFVectorfield& force)const
{
	double value = 0.0; 
	vector<double> sums(m_QtQinv.size(), 0.0); 
	
	auto idx = m_Q_mappping.begin(); 
	for (auto ia = a.begin(); ia != a.end(); ++ia, ++idx) {
		value += *ia * *ia; 
		sums[*idx] += *ia; 
	}
	
	auto s = sums.begin(); 
	for (auto q = m_QtQinv.begin(); q != m_QtQinv.end(); ++q, ++s) {
		value -= *q * *s * *s; 
		*s *= *q; 
	}
	
	auto gradient = get_gradient(a);
	auto iforce = force.begin(); 
	auto igrad = gradient.begin(); 
	idx = m_Q_mappping.begin(); 
	for (auto ia = a.begin(); ia != a.end(); ++ia, ++iforce, ++igrad, ++idx)
		*iforce = *igrad * (*ia - sums[*idx]);
	
	return 0.5 * value; 
	
}


C2DLSDImageCostPlugin::C2DLSDImageCostPlugin():
	C2DImageCostPlugin("lsd")
{
}

C2DImageCostPlugin::ProductPtr	C2DLSDImageCostPlugin::do_create()const
{
	return C2DImageCostPlugin::ProductPtr(new C2DLSDImageCost()); 
}

const std::string C2DLSDImageCostPlugin::do_get_descr()const
{
	return "Least-Squares Distance measure"; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DLSDImageCostPlugin();
}

NS_END
