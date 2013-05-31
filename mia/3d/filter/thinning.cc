/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/3d/filter/thinning.hh>
#include <mia/core/errormacro.hh>

using namespace thinning_3dimage_filter; 
using namespace mia; 

using std::vector; 
using std::invalid_argument; 

C3DThinning::C3DThinning()
{
}

class CNeighborhood {
public: 
	CNeighborhood(const C3DBitImage& input, const C3DBounds& pos); 
	
	bool is_candidate(int dir)const; 

	bool is_simple_pixel()const; 
	
private: 
	bool is_boundary(int dir) const; 
	
	bool is_Euler_invariant() const; 

	static void ot_label1(char label, vector<char>& cube); 
	static void ot_label2(char label, vector<char>& cube); 
	static void ot_label3(char label, vector<char>& cube); 
	static void ot_label4(char label, vector<char>& cube); 
	static void ot_label5(char label, vector<char>& cube); 
	static void ot_label6(char label, vector<char>& cube); 
	static void ot_label7(char label, vector<char>& cube); 
	static void ot_label8(char label, vector<char>& cube); 

	typedef void (*FLabelFunction)(char label, vector<char>& cube); 
	static const FLabelFunction label_callback[26];
	
	vector<char> m_pixels;
	int m_nneigbors; 
	static const int m_direction_index[6]; 

	static const signed char eulerLUT_26n[128]; 
}; 

const CNeighborhood::FLabelFunction CNeighborhood::label_callback[26]  = {
	ot_label1, ot_label1, ot_label2, // 2 
	ot_label1, ot_label1, ot_label2, // 5 
	ot_label3, ot_label3, ot_label4, // 8 
	
	ot_label1, ot_label1, ot_label2, // 11 
	ot_label1,/*center,*/ ot_label2, // 13
	ot_label3, ot_label3, ot_label4, // 16
	
	ot_label5, ot_label5, ot_label6, // 19
	ot_label5, ot_label5, ot_label6, // 22
	ot_label7, ot_label7, ot_label8  // 25
}; 

const int CNeighborhood::m_direction_index[6] = {10, 15, 13, 12, 21, 4 }; 

static void thinning_run(C3DBitImage& input) 
{
	int pixels_changed; 

	do {
		pixels_changed = 0; 
		for (int border = 0; border < 6; ++border) {

			vector<C3DBitImage::range_iterator> candidate_pixels; 
			for (auto i = input.begin_range(C3DBounds::_0,input.get_size()); 
			     i != input.end_range(C3DBounds::_0,input.get_size()); ++i) {

				// background? 
				if (!*i) 
					continue; 
				
				CNeighborhood neighbours(input, i.pos()); 
				if (!neighbours.is_candidate(border))
					continue; 
				
				candidate_pixels.push_back(i);
			}
			// this two-step procedure is only needed if the candidate collection is done 
			// in parallel 
			for (auto c = candidate_pixels.begin(); c != candidate_pixels.end(); ++c) {
				CNeighborhood n(input, c->pos()); 
				if (n.is_simple_pixel()) {
					**c = false;
					++pixels_changed; 
				}
			}
		}
		cvinfo() << "Pixels changed:" << pixels_changed << "\n"; 
		
	} while (pixels_changed > 0); 

}


mia::C3DFilter::result_type C3DThinning::do_filter(const mia::C3DImage& image) const
{
	if (image.get_pixel_type() != it_bit)  {
		throw create_exception<invalid_argument>("C3DThinning: only binary images are supported, "
							 "but pixel format is '", 
							 CPixelTypeDict.get_name(image.get_pixel_type()), "'"); 

	}
	auto result = image.clone(); 
	auto bit_image = dynamic_cast<C3DBitImage&>(*result); 
	thinning_run(bit_image); 
	return bit_image.clone(); 
}



C3DThinningFilterPlugin::C3DThinningFilterPlugin():
	C3DFilterPlugin("thinning")
{
}


mia::C3DFilter *C3DThinningFilterPlugin::do_create()const
{
	return new C3DThinning(); 
}

const std::string C3DThinningFilterPlugin::do_get_descr()const
{
	return "3D morphological thinning,  "
		"based on: Lee and Kashyap, 'Building Skeleton Models "
		"via 3-D Medial Surface/Axis Thinning Algorithms', "
		"Graphical Models and Image Processing, 56(6):462-478, 1994. "
		"This implementation only supports the 26 neighbourhood.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DThinningFilterPlugin(); 
}


CNeighborhood::CNeighborhood(const C3DBitImage& input, const C3DBounds& pos):
	m_pixels(26), 
	m_nneigbors(-1)
{
	vector<char> help(27); 
	auto i = help.begin(); 
	for(int z = -1; z < 2; ++z) {
		unsigned int iz = pos.z + static_cast<unsigned int>(z);
		for(int y = -1; y < 2; ++y) {
			unsigned int iy = pos.y + static_cast<unsigned int>(y);
			for(int x = -1; x < 2; ++x) {
				unsigned int ix = pos.x + static_cast<unsigned int>(x);
				if (input(ix,iy,iz)) {
					*i = 1; 
					++m_nneigbors; 
				}
				++i; 
			}
		}
	}

	copy(help.begin(), help.begin()+13, m_pixels.begin()); 
	copy(help.begin()+14, help.end(), m_pixels.begin()+13); 
}

bool CNeighborhood::is_candidate(int dir)const
{
	return is_boundary(dir) && is_Euler_invariant() && is_simple_pixel(); 
}

bool CNeighborhood::is_boundary(int dir) const
{
	return !m_pixels[m_direction_index[dir]]; 
}

bool CNeighborhood::is_Euler_invariant() const
{
	signed char EulerChar = 0;
	unsigned int n; 
	
	// octant south-west-upper
	n = m_pixels[23] * 64 +  m_pixels[24] * 32 +  m_pixels[14] * 16 +
		m_pixels[15]  * 8 + m_pixels[20] * 4 + m_pixels[21] * 2 + m_pixels[12];
	EulerChar += eulerLUT_26n[n];

	// octant south-east-upper
	n = m_pixels[25] * 64 +  m_pixels[22] * 32 + m_pixels[16]  * 16 + 
		m_pixels[13] * 8 + m_pixels[24] * 4 + m_pixels[21] * 2 + m_pixels[15]; 
	EulerChar += eulerLUT_26n[n];
	
	// octant north-west-upper
	n = m_pixels[17] * 64 +  m_pixels[20] * 32 + m_pixels[ 9]  * 16 + 
		m_pixels[12] * 8 + m_pixels[18] * 4 + m_pixels[21] * 2 + m_pixels[10]; 
	EulerChar += eulerLUT_26n[n];

       	// octant north-east-upper
	n = m_pixels[19] * 64 +  m_pixels[22] * 32 + m_pixels[18]  * 16 + 
		m_pixels[21] * 8 + m_pixels[11] * 4 + m_pixels[13] * 2 + m_pixels[10]; 
	EulerChar += eulerLUT_26n[n];

        // octant south-west-below
	n = m_pixels[ 6] * 64 +  m_pixels[14] * 32 + m_pixels[7]  * 16 + 
		m_pixels[15] * 8 + m_pixels[ 3] * 4 + m_pixels[12] * 2 + m_pixels[4]; 
	EulerChar += eulerLUT_26n[n];

        // octant south-east-below
	n = m_pixels[ 8] * 64 +  m_pixels[ 7] * 32 + m_pixels[16]  * 16 + 
		m_pixels[15] * 8 + m_pixels[ 5] * 4 + m_pixels[ 4] * 2 + m_pixels[13]; 
	EulerChar += eulerLUT_26n[n];

	// octant north-west-below
	n = m_pixels[ 0] * 64 +  m_pixels[ 9] * 32 + m_pixels[ 3]  * 16 + 
		m_pixels[12] * 8 + m_pixels[ 1] * 4 + m_pixels[10] * 2 + m_pixels[4]; 
	EulerChar += eulerLUT_26n[n];

        // octant north-east-below
	n = m_pixels[ 2] * 64 +  m_pixels[ 1] * 32 + m_pixels[11]  * 16 + 
		m_pixels[10] * 8 + m_pixels[ 5] * 4 + m_pixels[4] * 2 + m_pixels[13]; 
	EulerChar += eulerLUT_26n[n];

	return EulerChar == 0; 
}

const signed char CNeighborhood::eulerLUT_26n[128] = {
	  1,  -1,  -1,   1, 
	 -3,  -1,  -1,   1, 
	 -1,   1,   1,  -1, 
	  3,   1,   1,  -1, 
	 -3,  -1,   3,   1, 
	  1,  -1,   3,   1, 
	 -1,   1,   1,  -1, 
	  3,   1,   1,  -1, 

	 -3,   3,  -1,   1, 
	  1,   3,  -1,   1, 
	 -1,   1,   1,  -1, 
	  3,   1,   1,  -1, 
	  1,   3,   3,   1, 
	  5,   3,   3,   1, 
	 -1,   1,   1,  -1, 
	  3,   1,   1,  -1, 

	 -7,  -1,  -1,   1, 
	 -3,  -1,  -1,   1, 
	 -1,   1,   1,  -1, 
	  3,   1,   1,  -1, 
	 -3,  -1,   3,   1, 
	  1,  -1,   3,   1, 
	 -1,   1,   1,  -1, 
	  3,   1,   1,  -1, 

	 -3,   3,  -1,   1, 
	  1,   3,  -1,   1, 
	 -1,   1,   1,  -1, 
	  3,   1,   1,  -1, 
	  1,   3,   3,   1, 
	  5,   3,   3,   1, 
	 -1,   1,   1,  -1, 
	  3,   1,   1,  -1
}; 

bool CNeighborhood::is_simple_pixel()const
{
	vector<char> cube(m_pixels); 
	char label = 2;
	
	for(int i = 0; i < 26; i++ ) {
		if (cube[i] != 1)
			continue; 
		
		label_callback[i](label, cube); 
		++label; 
		if( label-2 >= 2 )
			return false;

	}
	return true; 
}

void CNeighborhood::ot_label1(char label, vector<char>& cube)
{
	if( cube[0] == 1 )
		cube[0] = label;

	if( cube[1] == 1 ) {
		cube[1] = label;        
		ot_label2(label, cube);
	}

	if( cube[3] == 1 )  {
		cube[3] = label;        
		ot_label3(label, cube);
	}

	if( cube[4] == 1 )  {
		cube[4] = label;        
		ot_label2(label, cube);
		ot_label3(label, cube);
		ot_label4(label, cube);
	}

	if( cube[9] == 1 )   {
		cube[9] = label;        
		ot_label5(label, cube);
	}

	if( cube[10] == 1 ) {
		cube[10] = label;        
		ot_label2(label, cube);
		ot_label5(label, cube);
		ot_label6(label, cube);
	}

	if( cube[12] == 1 )  {
		cube[12] = label;        
		ot_label3(label, cube);
		ot_label5(label, cube);
		ot_label7(label, cube);
	}
}
 
void CNeighborhood::ot_label2(char label, vector<char>& cube)
{
	if( cube[1] == 1 ) {
		cube[1] = label;
		ot_label1(label, cube);
	}

	if( cube[4] == 1 ) {
		cube[4] = label;        
		ot_label1(label, cube);
		ot_label3(label, cube);
		ot_label4(label, cube);
	}
	if( cube[10] == 1 ) {
		cube[10] = label;        
		ot_label1(label, cube);
		ot_label5(label, cube);
		ot_label6(label, cube);
	}

	if( cube[2] == 1 )
		cube[2] = label;        

	if( cube[5] == 1 )  {
		cube[5] = label;        
		ot_label4(label, cube);
	}

	if( cube[11] == 1 ) {
		cube[11] = label;        
		ot_label6(label, cube);
	}

	if( cube[13] == 1 )  {
		cube[13] = label;        
		ot_label4(label, cube);
		ot_label6(label, cube);
		ot_label8(label, cube);
	}

}

void CNeighborhood::ot_label3(char label, vector<char>& cube)
{
		
	if( cube[3] == 1 ) {
		cube[3] = label;        
		ot_label1(label, cube);
	}

	if( cube[4] == 1 ) {
		cube[4] = label;        
		ot_label1(label, cube);
		ot_label2(label, cube);
		ot_label4(label, cube);
	}

	if( cube[12] == 1 ) {
		cube[12] = label;        
		ot_label1(label, cube);
		ot_label5(label, cube);
		ot_label7(label, cube);
	}

	if( cube[6] == 1 )
		cube[6] = label;        

	if( cube[7] == 1 )  {
		cube[7] = label;        
		ot_label4(label, cube);
	}

	if( cube[14] == 1 ){
		cube[14] = label;        
		ot_label7(label, cube);
	}

	if( cube[15] == 1 )
	{
		cube[15] = label;        
		ot_label4(label, cube);
		ot_label7(label, cube);
		ot_label8(label, cube);
	}

}

void CNeighborhood::ot_label4(char label, vector<char>& cube)
{
	if( cube[4] == 1 ) {
		cube[4] = label;        
		ot_label1(label, cube);
		ot_label2(label, cube);
		ot_label3(label, cube);
	}

	if( cube[5] == 1 ) {
		cube[5] = label;        
		ot_label2(label, cube);
	}

	if( cube[13] == 1 ) {
		cube[13] = label;        
		ot_label2(label, cube);
		ot_label6(label, cube);
		ot_label8(label, cube);
	}

	if( cube[7] == 1 ) {
		cube[7] = label;        
		ot_label3(label, cube);
	}

	if( cube[15] == 1 ) {
		cube[15] = label;        
		ot_label3(label, cube);
		ot_label7(label, cube);
		ot_label8(label, cube);
	}

	if( cube[8] == 1 )
		cube[8] = label;        

	if( cube[16] == 1 ) {
		cube[16] = label;        
		ot_label8(label, cube);
	}

}

void CNeighborhood::ot_label5(char label, vector<char>& cube)
{
	if( cube[9] == 1 ) {
		cube[9] = label;        
		ot_label1(label, cube);
	}

	if( cube[10] == 1 ) {
		cube[10] = label;        
		ot_label1(label, cube);
		ot_label2(label, cube);
		ot_label6(label, cube);
	}

	if( cube[12] == 1 ) {
		cube[12] = label;        
		ot_label1(label, cube);
		ot_label3(label, cube);
		ot_label7(label, cube);
	}

	if( cube[17] == 1 )
		cube[17] = label;        

	if( cube[18] == 1 ) {
		cube[18] = label;        
		ot_label6(label, cube);
	}

	if( cube[20] == 1 ) {
		cube[20] = label;        
		ot_label7(label, cube);
	}

	if( cube[21] == 1 ){
		cube[21] = label;        
		ot_label6(label, cube);
		ot_label7(label, cube);
		ot_label8(label, cube);
	}

}

void CNeighborhood::ot_label6(char label, vector<char>& cube)
{
	if( cube[10] == 1 ) {
		cube[10] = label;        
		ot_label1(label, cube);
		ot_label2(label, cube);
		ot_label5(label, cube);
	}

	if( cube[11] == 1 ) {
		cube[11] = label;        
		ot_label2(label, cube);
	}

	if( cube[13] == 1 ) {
		cube[13] = label;        
		ot_label2(label, cube);
		ot_label4(label, cube);
		ot_label8(label, cube);
	}

	if( cube[18] == 1 ) {
		cube[18] = label;        
		ot_label5(label, cube);
	}

	if( cube[21] == 1 ) {
		cube[21] = label;        
		ot_label5(label, cube);
		ot_label7(label, cube);
		ot_label8(label, cube);
	}

	if( cube[19] == 1 )
		cube[19] = label;        

	if( cube[22] == 1 ) {
		cube[22] = label;        
		ot_label8(label, cube);
	}

}

void CNeighborhood::ot_label7(char label, vector<char>& cube)
{
	if( cube[12] == 1 ) {
		cube[12] = label;        
		ot_label1(label, cube);
		ot_label3(label, cube);
		ot_label5(label, cube);
	}

	if( cube[14] == 1 ) {
		cube[14] = label;        
		ot_label3(label, cube);
	}

	if( cube[15] == 1 ) {
		cube[15] = label;        
		ot_label3(label, cube);
		ot_label4(label, cube);
		ot_label8(label, cube);
	}

	if( cube[20] == 1 ) {
		cube[20] = label;        
		ot_label5(label, cube);
	}

	if( cube[21] == 1 ) {
		cube[21] = label;        
		ot_label5(label, cube);
		ot_label6(label, cube);
		ot_label8(label, cube);
	}

	if( cube[23] == 1 )
		cube[23] = label;        

	if( cube[24] == 1 ) {
		cube[24] = label;        
		ot_label8(label, cube);
	}

}

void CNeighborhood::ot_label8(char label, vector<char>& cube)
{
	if( cube[13] == 1 ) {
		cube[13] = label;        
		ot_label2(label, cube);
		ot_label4(label, cube);
		ot_label6(label, cube);
	}

	if( cube[15] == 1 ) {
		cube[15] = label;        
		ot_label3(label, cube);
		ot_label4(label, cube);
		ot_label7(label, cube);
	}
 
	if( cube[16] == 1 ) {
		cube[16] = label;        
		ot_label4(label, cube);
	}

	if( cube[21] == 1 ) {
		cube[21] = label;        
		ot_label5(label, cube);
		ot_label6(label, cube);
		ot_label7(label, cube);
	}

	if( cube[22] == 1 ) {
		cube[22] = label;        
		ot_label6(label, cube);
	}

	if( cube[24] == 1 ){
		cube[24] = label;        
		ot_label7(label, cube);
	}

	if( cube[25] == 1 )
		cube[25] = label;        

}

