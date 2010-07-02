/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

/*! \brief basic type of a plugin handler

A 3D weighted median filter plugin

\file gauss_image3d_filter.hh
\author Gert Wollny <wollny at eva.mpg.de>

*/

#include <mia/2d/filter/admean.hh>

NS_BEGIN ( admean_2dimage_filter )
NS_MIA_USE;
using namespace std;

static char const * plugin_name = "admean";

template <class Data2D>
typename C2DAdmean::result_type C2DAdmean::operator () ( const Data2D& data ) const
{
	Data2D *result = new Data2D ( data.get_size() );


	typename Data2D::iterator i = result->begin();
	typename Data2D::const_iterator t = data.begin();

	cvdebug() << "filter with width = " << _M_width << endl;

	double sum = 0.0;
	double sum2 = 0.0;
	double n = data.size();

	if ( n == 1 )
	{
		*i = *t;
		return P2DImage ( result );
	}

	for ( typename Data2D::const_iterator i = data.begin();
	        i != data.end(); ++i )
	{
		sum += *i;
		sum2 += *i * *i;
	}

	double mean = sum / n;
	double var2 = ( sum2 - mean * sum ) / ( n -1 );

	for ( int y = 0; y < ( int ) data.get_size().y; ++y )
		for ( int x = 0; x < ( int ) data.get_size().x; ++x, ++i, ++t )
		{

			double lsum = 0.0;
			double lsum2 = 0.0;
			int num = 0;

			for ( int iy = max ( 0, y - _M_width );
			        iy < min ( y + _M_width + 1, ( int ) data.get_size().y );  ++iy )

				for ( int ix = max ( 0, x - _M_width );
				        ix < min ( x + _M_width + 1, ( int ) data.get_size().x );  ++ix )
				{
					double v = data ( ix,iy );
					lsum += v;
					lsum2 += v*v;
					++num;
				}
			if ( num > 1 )
			{
				float lmean = lsum/num;
				float lvar2 = ( lsum2 - lsum * lmean ) / ( num - 1 );
				*i = ( typename Data2D::value_type ) ( lvar2 > var2 ? ( *t -  var2 / lvar2 * ( *t - lmean ) ) : lmean );
			}
			else
				*i = ( typename Data2D::value_type ) ( lsum );
		}

	return P2DImage ( result );
}

P2DImage C2DAdmean::do_filter ( const C2DImage& image ) const
{
	return mia::filter ( *this,image );
}

CAdmean2DImageFilterFactory::CAdmean2DImageFilterFactory() :
		C2DFilterPlugin ( plugin_name ),
		_M_hwidth ( 1 )
{
	add_parameter("w", new CIntParameter(_M_hwidth, 0, numeric_limits<int>::max(),
					     false, "half filter width"));
}

C2DFilterPlugin::ProductPtr CAdmean2DImageFilterFactory::do_create()const
{
	return C2DFilterPlugin::ProductPtr ( new C2DAdmean ( _M_hwidth ) );
}

const string CAdmean2DImageFilterFactory::do_get_descr() const
{
	return "2D adaptive mean filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CAdmean2DImageFilterFactory();
}

NS_END
