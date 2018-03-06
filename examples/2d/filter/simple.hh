/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/2d/filter.hh>


/*
   Each plug-in should provide its own namespace to avoid possible name clashes
   when more then one plug-ins are loaded.

*/
NS_BEGIN(simple_2dimage_filter)


/*
   The actual 2D filter is a derivative of the C2DFilter class which is a model of TImageFilter.
   Note, that these class names are not visible outside of this plug-in code, so there is really no need
   to give very expressive names.
*/

class C2DSimple: public mia::C2DFilter
{
public:
       /*
          The constructor takes all the necessary parameters. Usually this means that after construction, the
          filter does not change anymore. As an exception to this rule, a data key may be provided that makes
          it possible to load data from the internal memory that was created after construction of the filter.
          This may be the case when specifying a long pipeline of filters.
        */

       C2DSimple(double a);


       /*
          This templated operator does the actual work of the filter. The operator must support all pixel types
          that are currently provided by MIA. If some input pixel type is nor supported, e.g. because the filter
          just doesn't define an operation on the pixel type, then you should use (partial) template specialization
          to dispatch the work based on the pixel type, and if an image with an unsupported type is provided as input then
          an exception should be thrown.
          Thr result type is a P2Dimage.
        */
       template <typename T>
       C2DSimple::result_type operator () (const mia::T2DImage<T>& data) const;
private:
       /*
          This method implements always the same code - redirect to dispatching the call
          based on input pixel type.
        */
       virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;

       /*
          This is the actual parameter of the 'simple' filter
        */
       double m_a;
};


/*
  This class provides the actual plug-in interface that is used to create the filter. It also provides some
  information about the filter to be used in the help system.
*/
class C2DSimpleImageFilterPlugin: public mia::C2DFilterPlugin
{
public:
       C2DSimpleImageFilterPlugin();
       virtual mia::C2DFilter *do_create()const;
       virtual const std::string do_get_descr()const;
private:
       /*
         This is a duplicate of the  parameter of the 'simple' filter used when parsing the plug-in
         description and creaing the actual filter - the product of the plug-in.
       */
       double m_a;
};

NS_END
