/* -*- mona-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
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

A gradient based region growing filter seeded based on a fuzzy c-means classification. 

\author Gert Wollny <wollny at eva.mpg.de>

*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cassert>

#include <libmona/defines.hh>

#include <libmona/filter2dstack.hh>
#include <libmona/probmapio.hh>

#include "rgg_tools.hh"

NAMESPACE_BEGIN(rgg2pass_2dstack_filter)

USING_NAMESPACE_MONA;

using namespace std; 

static char const * plugin_name = "rgg2pass";
static const CStringOption param_map("map", "seed class map", "");
static const CFloatOption  param_seed("seed", "threshold for seed probability", 0.9f, 0.0f, 1.0f); 
static const CIntOption    param_depth("depth", "number of slices to keep during processing", 5, 1, 
				       numeric_limits<int>::max()); 
static const CFloatOption  param_gt("gt", "gradient thresh", 4.0, 0.0, numeric_limits<float>::max()); 
static const CIntOption    param_pass("pass", "segmentation pass", 0, 0, 2); 


class C2DRGG2PassStackFilterFactory: public C2DStackFilterFactory {
public: 	
	C2DRGG2PassStackFilterFactory();
	C2DStackFilterFactory::ProductPtr create(const CParsedOptions& options) const;
	const string short_descr()const; 
private: 
	int do_test() const; 
};


C2DRGG2PassStackFilterFactory::C2DRGG2PassStackFilterFactory()
{
	
}
	
C2DStackFilterFactory::ProductPtr 
C2DRGG2PassStackFilterFactory::create(const CParsedOptions& options) const
{
	return C2DStackFilterFactory::ProductPtr(NULL);
}

const string C2DRGG2PassStackFilterFactory::short_descr()const
{
	return "A 2-pass image stack seeded region growing filter";  
}

int C2DRGG2PassStackFilterFactory::do_test() const
{
	CParsedOptions options; 

	C2DStackFilterFactory::ProductPtr filter = create(options); 

	assert(filter.get()); 

	return 1; 
}





NAMESPACE_END // end namespace rgg2pass_2dstack_filter
