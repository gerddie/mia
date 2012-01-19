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

/* 
   LatexBeginPluginDescription{2D image filters}
   
   \subsection{Distance transform}
   \label{filter2d:distance}
   
   \begin{description}
   
   \item [Plugin:] distance
   \item [Description:] Apply a distance transform to the input image 
   \item [Input:] Binary image 
   \item [Output:] A double float valued image that gives the Euklidian distance 
                   of each pixel to the shape defined in the input image. 
		   
   This plug-in doesn't take further parameters. 
		   
   \end{description}

   LatexEnd  
 */


#include <mia/2d/2dfilter.hh>
#include <mia/2d/filter/distance.hh>

NS_BEGIN(distance_2d_filter) 

NS_MIA_USE;
using namespace std; 


template <class T>
typename C2DDistance::result_type 
C2DDistance::operator () (const T2DImage<T>& image) const
{
	C2DDImage *result = new C2DDImage(image.get_size()); 
	fill(result->begin(),   result->end(), numeric_limits<C2DDImage::value_type>::max()); 
	
	// brute force approach, there is a better way ...
	auto i = image.begin(); 
	for (size_t yi = 0; yi < image.get_size().y; ++yi) {
		for (size_t xi = 0; xi < image.get_size().x; ++xi, ++i) {
			if (!*i) 
				continue;
			auto r = result->begin(); 
			for (size_t yr = 0; yr < image.get_size().y; ++yr) {
				double dy2 = double(yr) - yi; 
				dy2 *= dy2; 
				for (int xr = 0; xr < (int)image.get_size().x; ++xr, ++r) {
					const double dx = double(xi) - xr; 
					const double d = dx * dx + dy2; 
					if (*r > d) 
						*r = d;
					
				}
			}
		}
	}
	transform(result->begin(), result->end(), result->begin(), 
		  [](double x) {return sqrt(x);}); 
	return P2DImage(result); 
}


P2DImage C2DDistance::do_filter(const C2DImage& image) const
{
	return ::mia::filter(*this, image); 
}


class C2DDistanceImageFilterFactory: public C2DFilterPlugin {
public:
	C2DDistanceImageFilterFactory();
private:
	virtual C2DFilter *do_create()const;
	virtual const std::string do_get_descr() const;
};

C2DDistanceImageFilterFactory::C2DDistanceImageFilterFactory():
	C2DFilterPlugin("distance")
{
}

C2DFilter *C2DDistanceImageFilterFactory::do_create() const
{
	return new C2DDistance(); 
}

const string C2DDistanceImageFilterFactory::do_get_descr()const
{
	return "2D image distance filter, evaluates the distance map for a binary mask."; 
}
	
extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DDistanceImageFilterFactory(); 
}

NS_END
