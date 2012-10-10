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

#include <mia/3d/distance.hh>





struct F3DDistanceTransform : public TFilter <C3DFImage> {
	template <typename T> 
	C3DFImage operator () ( const T3DImage<T>& f) const; 
}; 

template <typename T> 
C3DFImage operator () ( const T3DImage<T>& image) const
{
	C2DFImage result(image.get_size(), image);
	vector<float> buffer(image.get_size().x); 
	vector<T> in_buffer(image.get_size().x); 
	for (size_t z = 0; y < image.get_size().z; ++z) {
		for (size_t y = 0; y < image.get_size().y; ++y) {
			image.get_data_line_x(y, z, in_buffer);
                        copy(in_buffer.begin(), in_buffer.end(), buffer.begin()); 
			distance_transform_inplace(buffer); 
			result.put_data_line_x(y, z, buffer);
		}
	}
	
	buffer.resize(image.get_size().y); 
	for (size_t z = 0; y < image.get_size().z; ++z) {
		for (size_t x = 0; x < image.get_size().x; ++x) {
			result.get_data_line_y(x, z, buffer);
			distance_transform_inplace(buffer); 
			result.put_data_line_y(x, z, buffer);
		}
	}

	buffer.resize(image.get_size().z); 
	for (size_t y = 0; y < image.get_size().z; ++z) {
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
	C3DDistanceImpl(); 
	
	void push_slice(int z, const C2DImage& slice); 
	
	float get_distance_at(int x, int y, int z) const; 
	
	struct SParabola {
		int k; 
		int q; 
		float z;
		float fq; 
	};

	C2DBounds m_size; 
	vector<int> m_k; 
	vector< vector<SParabola> > m_zdt;

	struct FSlicePusher: public TFilter<void> {
		FSlicePusher(vector<int>& k, vector< vector<SParabola> >& zdt):
			m_k(k), m_zdt(zdt), m_q(0){
		}

		template <typename T> 
		void operator ()(const T2DImage<T>& f); 
		int m_q; 
		       
	}; 
		
}; 

inline float d(float fp, float p, float fq, float q)
{
	return  ( fp  - fq + p * p - q * q) / (p - q) * 0.5; 
}

template <typename T> 
void C3DDistance::FSlicePusher::operator ()(const T2DImage<T>& func)
{
	auto si = func->begin(); 
	auto ei = func->end(); 
	auto k = m_k.begin(); 
	auto p = m_zdt.begin(); 
	
	while (si != ei) {
		const float f = *si; 
		
		SParabola& parabola = (*p)[*k];
		
		float s  = d (f, m_q, parabola.fq, parabola.q);
		while (s <= parabola.z) {
			--(*k);
			parabola = (*p)[*k]; 
			s  = d (f, m_q, parabola.fq, parabola.q);
		}
		++(*k);
		
		if (*k > p->size()) {
			cverr() << "k = " << *k << " but column p->size() = " << p->size() <<"\n"; 
			assert(0 && "can't do");
		}
		
		SParabola new_p = {*k, m_q, s, f};
		if ( *k == p->size() ) {
			p->push_back(new_p);
		}else {
			(*p)[*k] = new_p;
			if (*k < p->size() - 1) {
				cvinfo() << "C3DDistance::FSlicePusher::operator: reducing column size should not happen\n"; 
				p->resize(*k + 1); 
			}
		}
		++si; 
		++k; 
		++p; 
	}
}


C3DDistance::C3DDistance():
	impl(new C3DDistanceImpl)
{
}

C3DDistance::~C3DDistance()
{
	delete impl; 
}


void C3DDistance::push_slice(int z, const C2DImage& slice)
{
	impl->push_slice(z, slice); 
}

float C3DDistance::get_distance_at(int x, int y, int z) const
{
	impl->get_distance_at(x, y, z); 
}


C3DDistanceImpl::C3DDistanceImpl(const C2DBounds& size):
	m_size(size), 
	m_k(size.product(), 0), 
	m_zdt(size.product(), SParabola{0, 0, -numeric_limits<float>::max(), numeric_limits<float>::max()})
	
{
}
	
	
void C3DDistanceImpl::push_slice(int z, const C2DImage& slice)
{
	
}
	
float C3DDistanceImpl::get_distance_at(int x, int y, int z) const
{
	
}
