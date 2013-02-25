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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <iomanip>
#include <fstream>

#include <string>
#include <stdexcept>
#include <dlfcn.h>

#include <mia/core.hh>
#include <mia/2d/imageio.hh>
#include <mia/3d/2dimagefifofilter.hh>
#include <mia/3d/vector.hh>


using namespace std;
NS_MIA_USE;

const SProgramDescription g_description = {
	{pdi_group, "Processing of series of 2D images in a 3D fashion (out-of-core)"}, 
	{pdi_short, "Evaluate the distance between two binary shapes."}, 
	
	{pdi_description, "This program takes two binary masks as input and evaluates the distance of one "
	"mask with respect to the other in voxel space. The output is given as text file "
	"with the coordinates of the source voxels and their distance to the reference "
	 "mask. Correction for voxel size must be done after processing."},

	{pdi_example_descr, "Evaluate the distance of the mask given in the images srcXXXX.png to the mask "
	 "given in refXXXX.png and save the result to distances.txt"}, 
	
	{pdi_example_code, "-i srcXXXX.png -r refXXXX.png -o distances.txt"}
}; 


const float g_far = numeric_limits<float>::max(); 

class C3DDT {
public: 
	C3DDT(); 
	void initialise(const C2DImage& wimage);
	
	void read( const C2DImage& wimage, short q); 
	
	void get_slice(size_t s, const C2DImage& image, vector<pair<C3DBounds, float> >& result) const; 
	
	bool test(); 
	
	bool test_initialise(const C2DBounds& size); 
private: 	

	float d(float fq, float q, float fv, float v)const; 
	
	void dt1d(vector<float>& f)const; 
	void dt2d(C2DFImage& image)const; 
	
	struct SParabola {
		short k; 
		short v; 
		float z;
		float fv; 
	};
	
	C2DBounds _M_size; 
	vector<short> _M_k; 
	vector< vector<SParabola> > _M_zdt;
};

C3DDT::C3DDT()
{
}

float C3DDT::d(float fq, float q, float fv, float v) const
{
	const float d = ( fq  - fv + q * q - v * v) / (q - v) * 0.5; 
	return d; 
}


void C3DDT::dt1d(vector<float>& r)const
{
	TRACE_FUNCTION; 
	vector<float> f(r); 
	vector<int> v(f.size()); 
	vector<float> z(f.size() + 1); 

	int k = 0;
	
	v[0] = 0;
	z[0] = -numeric_limits<float>::max();
	z[1] = +numeric_limits<float>::max();
	
	for (size_t q = 1; q < f.size(); q++) {
		float s  = d(f[q], q, f[v[k]], v[k]);
		while (s <= z[k]) {
			--k;
			s  = d(f[q], q, f[v[k]], v[k]);
		}
		++k;
		v[k] = q;
		z[k] = s;
		z[k+1] = numeric_limits<float>::max();
	}

	k = 0;
	for (size_t q = 0; q < f.size(); ++q) {
		while (z[k+1] < q)
			++k;
		float delta = float(q) - v[k]; 
		r[q] = delta * delta + f[v[k]];
	}
}

	

void C3DDT::initialise(const C2DImage& wimage)
{
	TRACE_FUNCTION; 
	
	_M_size = wimage.get_size(); 
	_M_k.resize(_M_size.x * _M_size.y); 
	fill(_M_k.begin(), _M_k.end(), 0.0f); 
	_M_zdt.resize(_M_size.x * _M_size.y); 
	
	auto src = dynamic_cast<const C2DBitImage&>(wimage); 
	auto pixel = src.begin(); 
	auto k = _M_zdt.begin(); 
	
	SParabola p = {0, 0, -numeric_limits<float>::max(), 0.0f}; 
	
	for (size_t i = 0; i < _M_zdt.size(); ++i, ++pixel, ++k) {
		p.fv = *pixel ? 0.0f : g_far; 
		k->push_back(p); 
	}
}


void C3DDT::read(const C2DImage& wimage, short q)
{
	TRACE_FUNCTION; 

	const C2DBitImage *src = dynamic_cast<const C2DBitImage *>(&wimage); 
	if (!src) 
		throw runtime_error("input image is not a bit image!");
	
	if (src->size() != _M_zdt.size()) 
		throw runtime_error("input image has different size!");
	
	auto si = src->begin(); 
	auto ei = src->end(); 
	auto k = _M_k.begin(); 
	auto p = _M_zdt.begin(); 
	
	while (si != ei) {
		float f = *si ? 0.0f : g_far; 
		
		SParabola& parabola = (*p)[*k]; 
		
		float s  = d (f, q, parabola.fv, parabola.v);
		while (s <= parabola.z) {
			--(*k);
			parabola = (*p)[*k]; 
			s  = d (f, q, parabola.fv, parabola.v);
		}
		++(*k);
		
		if (*k > (int)p->size()) {
			cverr() << "k = " << *k << " but p->size() = " << p->size() <<"\n"; 
			assert(0 && "can't do");
		}
		

		SParabola new_p = {*k, q, s, f};
		if ( *k == (int)p->size() ) {
			p->push_back(new_p);
		}else {
			(*p)[*k] = new_p; 
			if (*k < (int)p->size() - 1)
				p->resize(*k + 1); 
		}
		++si; 
		++k; 
		++p; 
	}
}

void C3DDT::dt2d(C2DFImage& image)const
{
	TRACE_FUNCTION; 
	vector<float> buffer(image.get_size().x); 
	for (size_t y = 0; y < image.get_size().y; ++y) {
		image.get_data_line_x(y, buffer);
		dt1d(buffer); 
		image.put_data_line_x(y, buffer);
	}
	buffer.resize(image.get_size().y); 
	for (size_t x = 0; x < image.get_size().x; ++x) {
		image.get_data_line_y(x, buffer);
		dt1d(buffer); 
		image.put_data_line_y(x, buffer);
	}
}

void C3DDT::get_slice(size_t s, const C2DImage& image, vector<pair<C3DBounds, float> >& result) const
{
	TRACE_FUNCTION; 
	
	const C2DBitImage *src = dynamic_cast<const C2DBitImage*>(&image);
	if (!src) {
		throw create_exception<invalid_argument>( "input image ", s, "not of type bit"); 
	}
	
	if (src->get_size() != _M_size) {
		throw create_exception<invalid_argument>( "input image ", s, "has a dffernt size then reference"); 
	}
	
	C2DFImage slice_tmp(_M_size); 
	auto i = slice_tmp.begin(); 
	auto e = slice_tmp.end(); 
	auto p = _M_zdt.begin(); 

	
	while (i != e) {
		size_t k = 0; 
		
		while ( k < p->size() - 1 && (*p)[ k + 1 ].z  < s) {
			++k; 
		}
		
		float delta = float(s) - (*p)[k].v; 
		*i = delta * delta + (*p)[k].fv;
		++i; 
		++p; 
	}
	
	dt2d(slice_tmp); 

	auto isrc = src->begin(); 
	auto iref = slice_tmp.begin(); 
	
	for (size_t y = 0; y < _M_size.y; ++y)
		for (size_t x = 0; x < _M_size.x; ++x, ++iref, ++isrc)
			if (*isrc)
				result.push_back(pair<C3DBounds, float>(C3DBounds(x,y,s), sqrt(*iref))); 
}

bool C3DDT::test_initialise(const C2DBounds& size)
{
	if (_M_zdt.size() != size.x * size.y) {
		cverr() << " buffer size should be " << size.x * size.y 
			<< " but is " << _M_zdt.size() <<"\n"; 
		return false; 
	}
	
	auto ki = _M_zdt.begin(); 
	auto ke = _M_zdt.end(); 
	
	bool result = true; 
	while (ki != ke) {
		if (ki->size() != 1) {
			cverr() << "not all slots are properly initialised in _M_zdt\n"; 
			return false; 
		}
		
		const SParabola& p = (*ki)[0]; 
		if (p.k != 0) {
			cverr() << "got a wrong k-initializer\n"; 
			result = false; 
		}
		
		if (p.v != 0) {
			cverr() << "got a wrong v-initializer\n"; 
			result = false; 
		}
		
		if (p.z != -numeric_limits<float>::max()) {
			cverr() << "got a wrong z-initializer\n"; 
			result = false; 
		}
		
		if ( ki == _M_zdt.begin() ) {
			if (p.fv != 0.0 ) {
				cverr() << "got a wrong (0,0,0) fv-initializer\n"; 
				result = false; 
			}
		} else {
			if (p.fv != g_far) {
				cverr() << "got a wrong general fv-initializer: " << p.fv <<" != " << g_far<<"\n"; 
				result = false; 
			}
		}
		++ki; 
	}
	
	return result; 
}

/*
static void print_diff(const C2DUSImage& image, unsigned short *ref) 
{
	size_t k = 0; 
	for (C2DUSImage::const_iterator i = image.begin(); 
	     i != image.end(); ++i, ++k)
		cverr() << *i << " vs. ref:" << ref[k] << "\n"; 
}
*/
							  
bool C3DDT::test()
{
	float in_1d[16]  = { 1, 1, 1, 0, 
			     1, 1, 1, 1, 
			     1, 1, 0, 1, 
			     1, 1, 1, 1 }; 
	float out_1d[16] = { 9, 4, 1, 0, 
			     1, 4, 9, 9, 
			     4, 1, 0, 1, 
			     4, 9, 16, 25 };
	
	vector<float> src(16); 
	transform(&in_1d[0], &in_1d[16],src.begin(), 
		  [](float x){return numeric_limits<float>::max() * x;}); 

	dt1d(src); 

	if (!equal(src.begin(), src.end(), &out_1d[0])) {
		cverr() << "1D distance transform failed\n"; 
		for (size_t i = 0; i < 16; ++i) {
			cverr() << out_1d[i] << " vs " << src[i] << "\n"; 
		}
		return false; 
	}
	
	float in_2d[16] =  { 1, 1, 1, 0, 
			     1, 1, 1, 1, 
			     1, 1, 0, 1, 
			     1, 1, 1, 1 }; 
	float out_2d[16] = { 8, 4, 1, 0, 
			     5, 2, 1, 1, 
			     4, 1, 0, 1, 
			     5, 2, 1, 2 }; 
	

	C2DFImage src_img(C2DBounds(4,4)); 
	
	transform(&in_2d[0], &in_2d[16],src_img.begin(), [](float x){return numeric_limits<float>::max() * x;}); 
	dt2d(src_img); 
	
	if (!equal(src_img.begin(), src_img.end(), &out_2d[0])) {
		cverr() << "2D distance transform failed\n"; 
		return false; 
	}
	
	bool in_3d_1[9] = { 1, 0, 0, 0, 0, 0, 0, 0, 0};
	bool in_3d_2[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0}; 

	/*
	float  out_3d_1[9] = { 0,          1,           2, 
			       1, sqrt(2.0f),  sqrt(5.0f), 
			       2, sqrt(5.0f), sqrt(8.0f)};
	
	float  out_3d_2[9] = {          1, sqrt(2.0f), sqrt(5.0f),
			       sqrt(2.0f), sqrt(3.0f), sqrt(6.0f), 
			             0.0f, sqrt(6.0f),         3};
	*/
	vector<pair<C3DBounds, float> > ref_result; 
	
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(0,0,0), 0)); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(1,0,0), 1)); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(2,0,0), 2)); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(0,1,0), 1)); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(1,1,0), sqrt(2.0f))); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(2,1,0), sqrt(5.0f))); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(0,2,0), 2)); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(1,2,0), sqrt(5.0f))); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(2,2,0), sqrt(8.0f))); 
	
	//ref_result.push_back(pair<C3DBounds, float>(C3DBounds(0,0,1), 0)); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(1,0,1), sqrt(2.0f))); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(2,0,1), sqrt(5.0f))); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(0,1,1), sqrt(2.0f))); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(1,1,1), sqrt(3.0f))); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(2,1,1), sqrt(6.0f))); 
	//ref_result.push_back(pair<C3DBounds, float>(C3DBounds(0,2,1), 0)); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(1,2,1), sqrt(6.0f))); 
	ref_result.push_back(pair<C3DBounds, float>(C3DBounds(2,2,1), 3)); 
	
	
	bool ref_3d_1[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1};
	bool ref_3d_2[9] = { 0, 1, 1, 1, 1, 1, 0, 1, 1}; 

	C2DBounds Size2D(3,3); 
	
	C2DBitImage *src1 = new C2DBitImage(Size2D); 
	C2DBitImage *src2 = new C2DBitImage(Size2D); 
	
	C2DBitImage *ref1 = new C2DBitImage(Size2D); 
	C2DBitImage *ref2 = new C2DBitImage(Size2D); 

	copy(&in_3d_1[0], &in_3d_1[9], src1->begin()); 
	copy(&in_3d_2[0], &in_3d_2[9], src2->begin()); 

	copy(&ref_3d_1[0], &ref_3d_1[9], ref1->begin()); 
	copy(&ref_3d_2[0], &ref_3d_2[9], ref2->begin()); 

	
	P2DImage s1(src1); 
	P2DImage s2(src2); 

	P2DImage wref1(ref1); 
	P2DImage wref2(ref2); 
	
	
	initialise(*s1); 
	
	if (!test_initialise(Size2D)) {
		cverr() << "initialisation failed!\n"; 
		return false; 
	}
	
	read(*s2, 1); 
	read(*s1, 2); 
	read(*s2, 3); 
	read(*s1, 4); 

	// should implement another test
	vector<pair<C3DBounds, float> > result; 
	get_slice(0, *wref1, result); 
	get_slice(1, *wref2, result);
	
	if (!equal(result.begin(), result.end(), ref_result.begin())) {
		cverr() << "part 3 failed\n"; 
		return false; 
	}

	return true; 
}


int do_main( int argc, char *argv[] )
{

	string src_filename;
	string ref_filename;	
	string out_filename;
	string out_type; 
	bool self_test = false; 
	
	
	CCmdOptionList options(g_description);
	options.set_group("File IO"); 
	options.add(make_opt( src_filename, "in-file", 'i', 
					"input image(s) that contain the source pixel mask", CCmdOption::required)); 
	options.add(make_opt( ref_filename, "ref-file", 'r', 
					"reference mask to evaluate the distance from", CCmdOption::required)); 
	options.add(make_opt( out_filename, "out-file", 'o', "output file name", CCmdOption::required)); 
	
	options.set_group("Test"); 
	options.add(make_opt( self_test, "self-test", 0,"run a self test" ));

	
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	if (self_test) {
		C3DDT dt; 
		return  dt.test() ? EXIT_SUCCESS : EXIT_FAILURE; 
	}

	
	size_t start_filenum = 0; 
	size_t end_filenum  = 0; 
	size_t format_width = 0; 		
	
	string ref_basename = get_filename_pattern_and_range(ref_filename, start_filenum,
							     end_filenum, format_width);
	if (start_filenum >= end_filenum)
		throw invalid_argument(string("no files match pattern ") + ref_basename);
	

	//char new_line = cverb.show_debug() ? '\n' : '\r'; 
	bool first = true; 
	
	C3DDT dt; 

	cvdebug() << "Read reference images\n"; 
	for (size_t i = start_filenum, k=0; i < end_filenum; ++i, ++k) {

		string src_name = create_filename(ref_basename.c_str(), i);

		cvmsg() << "\rRead " << src_name; 
		auto in_image = load_image2d(src_name);
		
		if (!in_image) {
			cverr() << "expected " << end_filenum - start_filenum <<
				" images, got only" << i - start_filenum <<"\n";
			break;
		}
		if (first) {
			dt.initialise(*in_image); 
			first = false; 
		}else
			dt.read(*in_image, k); 
	}
	cvmsg() << "\n"; 

	size_t src_start_filenum = 0; 
	size_t src_end_filenum  = 0; 
	string src_basename = get_filename_pattern_and_range(src_filename, src_start_filenum, src_end_filenum, format_width);
	if (src_start_filenum >= src_end_filenum)	
		throw invalid_argument(string("no files match pattern ") + src_basename);
	
	if (src_start_filenum != start_filenum || src_end_filenum != end_filenum) 
		throw invalid_argument(string("reference range and in-range are not equal")); 
	
	vector<pair<C3DBounds, float> > result; 
		
	for (size_t i = start_filenum, k=0; i < end_filenum; ++i, ++k) {
		string src_name = create_filename(src_basename.c_str(), i);
		cvmsg() << "\rRead " << src_name; 

		auto in_image = load_image2d(src_name);
		
		if (in_image) {
			dt.get_slice(k, *in_image, result);
		}
	}
	
	cvmsg() << "\n"; 
		
	ofstream os( out_filename.c_str(), ios::out ); 
		
	for (vector<pair<C3DBounds, float> >::const_iterator i = result.begin(); 
	     i != result.end(); ++i) 
		os << i->first.x << " " << i->first.y << " " << i->first.z << " " << i->second << '\n'; 
	
	return EXIT_SUCCESS; 
	
}
#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
