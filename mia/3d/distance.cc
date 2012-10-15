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

#include <mia/core/filter.hh>
#include <mia/3d/distance.hh>
#include <mia/2d/distance.hh>
#include <mia/core/distance.hh>

NS_MIA_BEGIN
using std::vector; 
using std::copy; 
using std::numeric_limits; 

struct F3DDistanceTransform : public TFilter <C3DFImage> {
	template <typename T> 
	C3DFImage operator () ( const T3DImage<T>& f) const; 
}; 

template <typename T> 
C3DFImage F3DDistanceTransform::operator () ( const T3DImage<T>& image) const
{
	C3DFImage result(image.get_size(), image);
	vector<float> buffer(image.get_size().x); 
	vector<T> in_buffer(image.get_size().x); 
	for (size_t z = 0; z < image.get_size().z; ++z) {
		for (size_t y = 0; y < image.get_size().y; ++y) {
			image.get_data_line_x(y, z, in_buffer);
                        copy(in_buffer.begin(), in_buffer.end(), buffer.begin()); 
			distance_transform_inplace(buffer); 
			result.put_data_line_x(y, z, buffer);
		}
	}
	
	buffer.resize(image.get_size().y); 
	for (size_t z = 0; z < image.get_size().z; ++z) {
		for (size_t x = 0; x < image.get_size().x; ++x) {
			result.get_data_line_y(x, z, buffer);
			distance_transform_inplace(buffer); 
			result.put_data_line_y(x, z, buffer);
		}
	}

	buffer.resize(image.get_size().z); 
	for (size_t y = 0; y < image.get_size().y; ++y) {
		for (size_t x = 0; x < image.get_size().x; ++x) {
			result.get_data_line_z(x, y, buffer);
			distance_transform_inplace(buffer); 
			result.put_data_line_z(x, y, buffer);
		}
	}
	return result; 
}

C3DFImage EXPORT_3D distance_transform(const C3DImage& f)
{
	F3DDistanceTransform dtf; 
	return mia::filter(dtf, f); 
}



struct C3DDistanceImpl{
	C3DDistanceImpl(const C2DImage& slice); 
	
	void push_slice(int z, const C2DImage& slice); 
	
	float get_distance_at(const C3DFVector& p) const; 

	C2DFImage get_distance_slice(int z) const; 
	
	struct SParabola {
		int q;
		int v;
		float z; 
		float fv;
	};

	C2DBounds m_size; 
	vector<int> m_k; 
	vector< vector<SParabola> > m_zdt;

	struct FSlicePusher: public TFilter<void> {
		FSlicePusher(vector<int>& k, vector< vector<SParabola> >& zdt):
			m_k(k), m_zdt(zdt), m_q(-1){
		}

		template <typename T> 
		void operator ()(const T2DImage<T>& f); 

		void set_slice(int z); 

	private:
		vector<int>& m_k; 
		vector< vector<SParabola> >& m_zdt;
		int m_q; 
	}; 

	struct FPushFirstSlice: public TFilter<void> {
		FPushFirstSlice(vector< vector<SParabola> >& zdt):
			m_zdt(zdt){
		}

		template <typename T> 
		void operator ()(const T2DImage<T>& f); 
		vector< vector<SParabola> >& m_zdt;
	}; 

	FSlicePusher m_pusher; 
}; 

inline float d(float fp, float p, float fq, float q)
{
	return  ( fp  - fq + p * p - q * q) / (p - q) * 0.5; 
}

void C3DDistanceImpl::FSlicePusher::set_slice(int z)
{
	m_q = z; 
}

template <typename T> 
void C3DDistanceImpl::FSlicePusher::operator ()(const T2DImage<T>& func)
{
	assert(m_q > 0); 
	
	auto si = func.begin(); 
	auto ei = func.end(); 
	auto k = m_k.begin(); 
	auto p = m_zdt.begin(); 


	while (si != ei) {
		const float fq = *si;
		
		SParabola& parabola = (*p)[*k];
		
		float s  = d (fq, m_q, parabola.fv, parabola.v);
		while (s <= parabola.z) {
			--(*k);
			parabola = (*p)[*k]; 
			s  = d (fq, m_q, parabola.fv, parabola.v);
		}
		++(*k);
		
		if (*k > (int)p->size()) {
			cverr() << "k = " << *k << " but column p->size() = " << p->size() <<"\n"; 
			assert(0 && "can't do");
		}
		
		SParabola new_p = {*k, m_q, s, fq};
		if ( *k == (int)p->size() ) {
			p->push_back(new_p);
		}else {
			(*p)[*k] = new_p;
			if (*k < (int)p->size() - 1) {
				cvinfo() << "C3DDistance::FSlicePusher::operator: reducing column size should not happen\n"; 
				p->resize(*k + 1); 
			}
		}
		++si; 
		++k; 
		++p; 
	}
	m_q = -1; 
}

template <typename T> 
void C3DDistanceImpl::FPushFirstSlice::operator ()(const T2DImage<T>& func)
{
	auto si = func.begin(); 
	auto ei = func.end(); 
	auto p = m_zdt.begin(); 
	while (si != ei) {
		SParabola parabola{ 0, 0, -numeric_limits<float>::max(), static_cast<float>(*si)}; 
		p->push_back(parabola); 
		++si; 
		++p; 
	}
	
}



C3DDistance::C3DDistance():
	impl(NULL)
{
}

C3DDistance::~C3DDistance()
{
	delete impl; 
}


void C3DDistance::push_slice(int z, const C2DImage& slice)
{
	if (z == 0)
		impl = new C3DDistanceImpl(slice); 
	else {
		assert(impl); 
		impl->push_slice(z, slice); 
	}
}

float C3DDistance::get_distance_at(const C3DFVector& p) const
{
	assert(impl); 
	return impl->get_distance_at(p); 
}


C3DDistanceImpl::C3DDistanceImpl(const C2DImage& slice):
	m_size(slice.get_size()), 
	m_k(m_size.product(), 0), 
	m_zdt(m_size.product()), 
	m_pusher(m_k, m_zdt)
{
	FPushFirstSlice pfs(m_zdt);
	mia::accumulate(pfs, slice); 
}
	
	
void C3DDistanceImpl::push_slice(int z, const C2DImage& slice)
{
	FUNCTION_NOT_TESTED; 
	m_pusher.set_slice(z); 
	mia::accumulate(m_pusher, slice); 
}
	
float C3DDistanceImpl::get_distance_at(const C3DFVector& p) const
{
	FUNCTION_NOT_TESTED; 
	float distance = numeric_limits<float>::max(); 
	float search_radius = 0; 
	
	int center_x = (int)(p.x + 0.5); 
	int center_y = (int)(p.y + 0.5);
	int center_z = (int)(p.z + 0.5);

	// limit search to support range 
	if (center_x >= (int)m_size.x - 1)
		center_x = m_size.x -1;

	if (center_x < 0)
		center_x = 0;

	if (center_y >= (int)m_size.y -1)
		center_y = m_size.y -1;

	if (center_y < 0)
		center_y = 0;

	int delta_x = p.x + 0.5 - center_x; 
	int delta_y = p.y + 0.5 - center_y;

	// search closest z-value 
	int center_idx = center_y * m_size.x + center_x; 

	auto& zdt = m_zdt[center_idx];
	if (zdt.size() > 1) {
		size_t k = 0; 
		while ( k < zdt.size() - 1 && zdt[ k + 1 ].z  < center_z) {
			++k; 
		}
		float delta = p.z - zdt[k].q; 
		distance = delta_x * delta_x + delta_y * delta_y + delta * delta + zdt[k].fv;
	}
	
	while (distance > search_radius) {
		
		
		

	}
	return distance; 
}

C2DFImage C3DDistanceImpl::get_distance_slice(int z) const
{
	FUNCTION_NOT_TESTED; 
	C2DFImage result(m_size); 
	auto i = result.begin(); 
	auto e = result.end(); 
	auto p = m_zdt.begin(); 

	while (i != e) {
		size_t k = 0; 
		
		while ( k < p->size() - 1 && (*p)[ k + 1 ].z  < z) {
			++k; 
		}
		
		float delta = float(z) - (*p)[k].q; 
		*i = delta * delta + (*p)[k].fv;
		++i; 
		++p; 
	}

	return distance_transform(result);
}

C2DFImage C3DDistance::get_distance_slice(int z) const
{
	assert(impl); 
	return impl->get_distance_slice(z); 
}
NS_MIA_END
