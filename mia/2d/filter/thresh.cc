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

#include <mia/2d/2dfilter.hh>
#include <libmona/monaException.hh>
#include <limits>
#include <sstream>

namespace thresh_2dimage_filter {
NS_MIA_USE;
using namespace std; 

static char const * plugin_name = "thresh";
static const CFloatOption param_min("min", "min", 0, -numeric_limits<float>::max(), numeric_limits<float>::max()); 
static const CIntOption param_neighbourhood("n", "pixel neighbourhood to consider (0,4,8)", 8, 0, 8);

class CThreshN : public C2DFilter {
public:
	CThreshN(float min, int neighbourhood); 

	template <class Data2D>
	typename CThreshN::result_type operator () (const Data2D& data) const ;
private: 
	
	template <class Data2D>
	void filter_0(Data2D& data)const; 
	
	template <class Data2D>
	void filter_4(Data2D& data)const; 
	
	template <class Data2D>
	void filter_8(Data2D& data)const; 

	float m_min; 
	int m_neighbourhood; 
};



class C2DThreshNImageFilter: public C2DImageFilterBase {
	CThreshN m_filter; 
public:
	C2DThreshNImageFilter(float min, int neighbourhood);

	virtual P2DImage do_filter(const C2DImage& image) const;
};

class C2DThreshNImageFilterFactory: public C2DFilterPlugin {
public: 
	C2DThreshNImageFilterFactory();
	virtual C2DFilterPlugin::ProductPtr create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
private: 
	virtual int do_test() const; 
};

CThreshN::CThreshN(float min, int neighbourhood):
	m_min(min), 
	m_neighbourhood(neighbourhood)
{
}


template <class Data2D>
void CThreshN::filter_0(Data2D& data)const
{
	typename Data2D::iterator ib = data.begin(); 
	typename Data2D::iterator ie = data.end(); 
	
	while (ib != ie) {
		if (*ib < m_min)
			*ib = 0; 
		++ib; 
	};
}

template <class Data2D>
void CThreshN::filter_4(Data2D& data)const
{
	typename Data2D::iterator i = data.begin(); 
	
	// upper left corner
	if (*i < m_min && i[1] < m_min && i[data.get_size().x] < m_min)
		*i = 0; 
	++i; 
	// upper row
	for (size_t x = 1; x < data.get_size().x - 1; ++x, ++i) {
		if (*i < m_min && i[1] < m_min && i[data.get_size().x] < m_min
		    && i[-1] < m_min)
			*i = 0; 
	}

	// upper right corner  
	if (*i < m_min && i[-1] < m_min && i[data.get_size().x] < m_min)
		*i = 0; 
	++i; 

	// the main part 
	for (size_t y = 1; y < data.get_size().y -1; ++y) {
		// left column 
		if (*i < m_min && 
		    i[1] < m_min && 
		    i[-data.get_size().x] < m_min && 
		    i[data.get_size().x] < m_min)
			*i = 0; 
		++i; 
		
		// inner image area
		for (size_t x = 1; x < data.get_size().x - 1; ++x, ++i) {
			if (*i < m_min && 
			    i[1] < m_min && i[-1] < m_min && 
			    i[-data.get_size().x] < m_min && i[data.get_size().x] < m_min)
				*i = 0; 
		}
		
		// right column
		if (*i < m_min && 
		    i[-1] < m_min && 
		    i[-data.get_size().x] < m_min && 
		    i[data.get_size().x] < m_min)
			*i = 0; 
		++i; 
	}


	// lower left corner
	if (*i < m_min && i[1] < m_min && i[-data.get_size().x] < m_min)
		*i = 0; 
	++i; 
	// upper row
	for (size_t x = 1; x < data.get_size().x - 1; ++x, ++i) {
		if (*i < m_min && i[1] < m_min && i[-data.get_size().x] < m_min
		    && i[-1] < m_min)
			*i = 0; 
	}

	// lower right corner  
	if (*i < m_min && i[-1] < m_min && i[-data.get_size().x] < m_min)
		*i = 0; 
	++i; 

}

template <class Data2D>
void CThreshN::filter_8(Data2D& data)const
{
	typename Data2D::iterator i = data.begin(); 
	
	// upper left corner
	if (*i < m_min && 
	    i[1] < m_min && 
	    i[data.get_size().x] < m_min &&
	    i[data.get_size().x + 1] < m_min)
		*i = 0; 
	++i; 
	
	// upper row
	for (size_t x = 1; x < data.get_size().x - 1; ++x, ++i) {
		if (*i < m_min && 
		    i[1] < m_min && 
		    i[-1] < m_min && 
		    i[data.get_size().x - 1] < m_min && 
		    i[data.get_size().x] < m_min && 
		    i[data.get_size().x + 1] < m_min 
		    )
			*i = 0; 
	}

	// upper right corner  
	if (*i < m_min && 
	    i[-1] < m_min && 
	    i[data.get_size().x] < m_min &&
	    i[data.get_size().x - 1] < m_min)
		*i = 0; 
	++i; 


	// the main part 
	for (size_t y = 1; y < data.get_size().y -1; ++y) {
		
		// left column
		if (*i < m_min && 
		    i[1] < m_min && 
		    i[-data.get_size().x] < m_min &&
		    i[-data.get_size().x + 1] < m_min &&
		    i[data.get_size().x] < m_min &&
		    i[data.get_size().x + 1] < m_min)
			*i = 0; 
		++i; 
		
		// upper row
		for (size_t x = 1; x < data.get_size().x - 1; ++x, ++i) {
			if (*i < m_min && 
			    i[1] < m_min && 
			    i[-1] < m_min && 
			    i[-data.get_size().x - 1] < m_min && 
			    i[-data.get_size().x] < m_min && 
			    i[-data.get_size().x + 1] < m_min &&
			    i[data.get_size().x - 1] < m_min && 
			    i[data.get_size().x] < m_min && 
			    i[data.get_size().x + 1] < m_min 
			    )
				*i = 0; 
		}

		// upper right corner  
		if (*i < m_min && 
		    i[-1] < m_min && 
		    i[-data.get_size().x -1] < m_min &&
		    i[-data.get_size().x] < m_min &&
		    i[data.get_size().x] < m_min &&
		    i[data.get_size().x - 1] < m_min)
			*i = 0; 
		++i; 
	}

	// upper left corner
	if (*i < m_min && 
	    i[1] < m_min && 
	    i[-data.get_size().x] < m_min &&
	    i[-data.get_size().x + 1] < m_min)
		*i = 0; 
	++i; 
	
	// upper row
	for (size_t x = 1; x < data.get_size().x - 1; ++x, ++i) {
		if (*i < m_min && 
		    i[1] < m_min && 
		    i[-1] < m_min && 
		    i[-data.get_size().x - 1] < m_min && 
		    i[-data.get_size().x] < m_min && 
		    i[-data.get_size().x + 1] < m_min 
		    )
			*i = 0; 
	}

	// upper right corner  
	if (*i < m_min && 
	    i[-1] < m_min && 
	    i[-data.get_size().x] < m_min &&
	    i[-data.get_size().x - 1] < m_min)
		*i = 0; 
	++i; 

}


template <class Data2D>
typename CThreshN::result_type CThreshN::operator () (const Data2D& data) const
{
	Data2D *result = new Data2D(data); 
	result->make_single_ref(); 
	
	cvdebug() << "CThreshN::operator () begin\n";
	switch (m_neighbourhood) {
	case 0:filter_0(*result); 
		break;
	case 4:filter_4(*result); 
		break;
	case 8:
		filter_8(*result); 
		break;
	default:
		stringstream errmsg; 
		errmsg << "undefined neighborhood " << m_neighbourhood << " in CThreshN::operator()"; 
		throw mona_fatal_error(errmsg.str()); 
	}
	cvdebug() << "CThreshN::operator () end\n";
	return P2DImage(result); 
}

C2DThreshNImageFilter::C2DThreshNImageFilter(float min, int neighbourhood):
		m_filter(min,neighbourhood)
{
}

P2DImage C2DThreshNImageFilter::do_filter(const C2DImage& image) const
{
	return wrap_filter(m_filter, image); 
	
}


C2DThreshNImageFilterFactory::C2DThreshNImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_min); 
	add_help(param_neighbourhood); 
}

C2DFilterPlugin::ProductPtr C2DThreshNImageFilterFactory::create(const CParsedOptions& options) const
{
	float min = param_min.get_value(options); 
	int n = param_neighbourhood.get_value(options); 

	return C2DFilterPlugin::ProductPtr(new C2DThreshNImageFilter(min, n)); 
}

const string C2DThreshNImageFilterFactory::do_get_descr()const
{
	return "2D image highpass filter"; 
}


int C2DThreshNImageFilterFactory::do_test() const
{
	const size_t size_x = 7; 
	const size_t size_y = 5;
	const int thresh = 5;
	
	const int src[size_y][size_x] =
		{{ 0, 1, 2, 3, 2, 3, 5}, 
		 { 2, 5, 2, 3, 5, 3, 2},
		 { 1, 2, 7, 6, 4, 2, 1},
		 { 3, 4, 4, 3, 4, 3, 2},
		 { 1, 3, 2, 4, 5, 6, 2}}; 
	
	// "hand filtered" w = 1 -> 3x3
	const int ref_4[size_y][size_x] =
		{{ 0, 1, 0, 0, 2, 3, 5}, 
		 { 2, 5, 2, 3, 5, 3, 2},
		 { 0, 2, 7, 6, 4, 0, 0},
		 { 0, 0, 4, 3, 4, 3, 0},
		 { 0, 0, 0, 4, 5, 6, 2}}; 

	const int ref_8[size_y][size_x] =
		{{ 0, 1, 2, 3, 2, 3, 5}, 
		 { 2, 5, 2, 3, 5, 3, 2},
		 { 1, 2, 7, 6, 4, 2, 0},
		 { 0, 4, 4, 3, 4, 3, 2},
		 { 0, 0, 0, 4, 5, 6, 2}}; 

	
	
	C2DBounds size(size_x, size_y);
	
	C2DSIImage *src_img = new C2DSIImage(size); 
	for (size_t y = 0; y < size_y; ++y)
		for (size_t x = 0; x < size_x; ++x) 
			(*src_img)(x,y) = src[y][x]; 
	
	C2DThreshNImageFilter thresh_4(thresh, 4); 
	C2DThreshNImageFilter thresh_8(thresh, 8); 
	
	P2DImage src_wrap(src_img); 
	
	P2DImage res_wrap_4 = thresh_4.filter(src_wrap); 
	P2DImage res_wrap_8 = thresh_8.filter(src_wrap); 
	C2DSIImage *res_img_4 = res_wrap_4.getC2DSIImage(); 
	C2DSIImage *res_img_8 = res_wrap_8.getC2DSIImage(); 
	
	if (!res_img_4 || !res_img_8 ) {
		cvfail() << get_name() << "wrong return type\n"; 
		return -1; 
	}
	
	if (res_img_4->get_size() != src_img->get_size() ||
	    res_img_8->get_size() != src_img->get_size()) {
		cvfail() << get_name() << "wrong return size\n"; 
		return -1; 
	}
	
	for (size_t y = 0; y < size_y; ++y)
		for (size_t x = 0; x < size_x; ++x) {
			if ((*res_img_4)(x,y) != ref_4[y][x]) {
				cvfail() << get_name() << "_4: images different at(" << x << "," << y << "): got "<< (*res_img_4)(x,y) <<
					" expected " << ref_4[y][x] << "\n"; 
				return -1; 
			}
			if ((*res_img_8)(x,y) != ref_8[y][x]) {
				cvfail() << get_name() << "_4: images different at(" << x << "," << y << "): got "<< (*res_img_8)(x,y) <<
					" expected " << ref_8[y][x] << "\n"; 
				return -1; 
			}
						
		}
	return 1; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DThreshNImageFilterFactory(); 
}

} // end namespace thresh_2dimage_filter
