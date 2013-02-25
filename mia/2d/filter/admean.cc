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

#include <mia/2d/filter/admean.hh>

NS_BEGIN ( admean_2dimage_filter )
NS_MIA_USE;
using namespace std;

static char const * plugin_name = "admean";

template <class Data2D>
typename C2DAdmean::result_type C2DAdmean::operator () ( const Data2D& data ) const
{
	typedef typename Data2D::value_type out_type; 
	Data2D *result = new Data2D ( data.get_size(), data );


	auto i = result->begin();
	auto t = data.begin();

	cvdebug() << "filter with width = " << m_width << endl;

	double sum = 0.0;
	double sum2 = 0.0;
	double n = data.size();

	if ( n == 1 ){
		*i = *t;
		return P2DImage ( result );
	}

	for ( auto i = data.begin(); i != data.end(); ++i ) {
		sum += *i;
		sum2 += *i * *i;
	}

	double mean = sum / n;
	double var2 = ( sum2 - mean * sum ) / ( n -1 );

	for ( int y = 0; y < ( int ) data.get_size().y; ++y )
		for ( int x = 0; x < ( int ) data.get_size().x; ++x, ++i, ++t )	{
			
			double lsum = 0.0;
			double lsum2 = 0.0;
			int num = 0;

			for ( int iy = max ( 0, y - m_width );
			      iy < min ( y + m_width + 1, ( int ) data.get_size().y );  ++iy )
				for ( int ix = max ( 0, x - m_width );
				      ix < min ( x + m_width + 1, ( int ) data.get_size().x );  ++ix )	{
					double v = data ( ix,iy );
					lsum += v;
					lsum2 += v*v;
					++num;
				}
			if ( num > 1 )	{
				float lmean = lsum/num;
				float lvar2 = ( lsum2 - lsum * lmean ) / ( num - 1 );
				*i = static_cast<out_type>( lvar2 > var2 ? 
							    ( *t -  var2 / lvar2 * ( *t - lmean ) ) : lmean );
			}
			else
				*i = static_cast<out_type>( lsum );
		}
	return P2DImage ( result );
}

P2DImage C2DAdmean::do_filter ( const C2DImage& image ) const
{
	return mia::filter ( *this,image );
}

CAdmean2DImageFilterFactory::CAdmean2DImageFilterFactory() :
		C2DFilterPlugin ( plugin_name ),
		m_hwidth ( 1 )
{
	add_parameter("w", new CIntParameter(m_hwidth, 0, numeric_limits<int>::max(),
					     false, "half filter width"));
}

C2DFilter *CAdmean2DImageFilterFactory::do_create()const
{
	return new C2DAdmean ( m_hwidth );
}

const string CAdmean2DImageFilterFactory::do_get_descr() const
{
	return "An adaptive mean filter that works like a normal mean filter, if the intensity "
		"variation within the filter mask is lower then the intensity variation in the "
		"whole image, that the uses a special formula if the local variation is higher "
		"then the image intensity variation.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CAdmean2DImageFilterFactory();
}

NS_END
