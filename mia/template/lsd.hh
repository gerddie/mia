/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/parameter.hh>
#include <mia/core/property_flags.hh>

#include <numeric>
#include <limits>

NS_BEGIN(NS)

///  @cond DOC_PLUGINS 
struct valpos {
	double val; 
	int pos; 
}; 


template <typename TCost> 
class TLSDImageCost: public TCost {
public: 	
	typedef typename TCost::Data Data; 
	typedef typename TCost::Force Force; 
private:

	class CRefPrepare :public mia::TFilter<void> {
	public: 
		CRefPrepare(std::vector<double>& QtQinv, std::vector<int>& Q_mappping):
			m_QtQinv(QtQinv), 
			m_Q_mappping(Q_mappping)
		{
		}
		template <typename DataTempl> 
		void operator()(const DataTempl& ref); 
		
		std::vector<double>& m_QtQinv; 
		std::vector<int>& m_Q_mappping; 
	}; 
	
	class RunCost : mia::TFilter<double>  {
	public: 
		typedef TFilter<double>::result_type result_type; 
		RunCost( const std::vector<double>& QtQinv, const std::vector<int>& Q_mappping); 
		
		template <typename DataTempl> 
		double operator()(const DataTempl& ref)const; 
		
		template <typename DataTempl> 
		double operator()(const DataTempl& ref, Force& force)const; 
	private: 
		const std::vector<double>& m_QtQinv;
		const std::vector<int>& m_Q_mappping; 
	}; 
	

	
	virtual double do_value(const Data& a, const Data& b) const;
	
	virtual double do_evaluate_force(const Data& a, const Data& /*b*/, Force& force) const; 
	
	virtual void post_set_reference(const Data& ref); 

	std::vector<double> m_QtQinv;
	std::vector<int>    m_Q_mappping;
};


inline bool operator < (const valpos& a, const valpos& b)
{
	return a.val < b.val; 
}

template <typename CP, typename C> 
class TLSDImageCostPlugin: public CP {
public: 
	TLSDImageCostPlugin();
	virtual TLSDImageCost<C> *do_create()const;
	const std::string do_get_descr()const; 
};

template <typename TCost> 
template <typename DataTempl> 
void TLSDImageCost<TCost>::CRefPrepare::operator()(const DataTempl& ref)
{
	size_t npixels = ref.get_size().x * ref.get_size().y; 
	std::vector<valpos> buffer(npixels); 
	m_Q_mappping.resize(npixels); 
	m_QtQinv.resize(npixels); 

	int idx = 0;
	for( auto x = ref.begin(); x != ref.end(); ++x, ++idx) {
		double value = *x; 
		valpos vp = {value, idx}; 
		buffer[idx] = vp; 
	}
	std::sort(buffer.begin(), buffer.end());
	
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
	std::transform(m_QtQinv.begin(), m_QtQinv.end(), m_QtQinv.begin(), 
		       [](double x){return 1.0 / x; }); 
}

template <typename TCost> 
void TLSDImageCost<TCost>::post_set_reference(const Data& ref)
{
	CRefPrepare rp(m_QtQinv, m_Q_mappping);
	mia::accumulate(rp, ref); 
}


template <typename TCost> 
double TLSDImageCost<TCost>::do_value(const Data& a, const Data& /*b*/) const
{
	RunCost rf(m_QtQinv, m_Q_mappping); 
	return mia::filter(rf, a); 
}

template <typename TCost> 
double TLSDImageCost<TCost>::do_evaluate_force(const Data& a, const Data& /*b*/, Force& force) const
{
	RunCost rf(m_QtQinv, m_Q_mappping); 
	return mia::filter_and_output(rf, a, force); 
}

template <typename TCost> 
TLSDImageCost<TCost>::RunCost::RunCost(const std::vector<double>& QtQinv, const std::vector<int>& Q_mappping):
	m_QtQinv(QtQinv), 
	m_Q_mappping(Q_mappping)
{
}

template <typename TCost> 
template <typename DataTempl> 
double  TLSDImageCost<TCost>::RunCost::operator()(const DataTempl& a)const
{
	double val1 = 0.0; 
	double val2 = 0.0; 
	std::vector<double> sums(m_QtQinv.size(), 0.0); 
	
	auto idx = m_Q_mappping.begin(); 
	for (auto ia = a.begin(); ia != a.end(); ++ia, ++idx) {
		val1 += *ia * *ia;  
		sums[*idx] += *ia; 
	}
	
	
	for(size_t i = 0; i < sums.size(); ++i)
		val2 += sums[i] * sums[i] * m_QtQinv[i]; 
	
	return 0.5 * (val1 - val2); 
}

template <typename TCost> 
template <typename DataTempl> 
double  TLSDImageCost<TCost>::RunCost::operator()(const DataTempl& a, Force& force)const
{
	double value = 0.0; 
	std::vector<double> sums(m_QtQinv.size(), 0.0); 
	
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

template <typename CP, typename C> 
TLSDImageCostPlugin<CP,C>::TLSDImageCostPlugin():
	CP("lsd")
{
}

template <typename CP, typename C> 
TLSDImageCost<C> *TLSDImageCostPlugin<CP,C>::do_create()const
{
	return new TLSDImageCost<C>(); 
}

template <typename CP, typename C> 
const std::string TLSDImageCostPlugin<CP,C>::do_get_descr()const
{
	return "Least-Squares Distance measure"; 
}

///  @endcond
NS_END
