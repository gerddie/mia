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
   LatexBeginPluginDescription{2D image stack filters}
   
   \subsection{By Slice filter}
   \label{fifof:byslice}
   
   \begin{description}
   
   \item [Plugin:] byslice
   \item [Description:] 
   \item [Input:] Gray scale or binary image(s) 
   \item [Output:] The filtered image(s) 
   
   \plugtabstart
   filter & string & a 2D filter (see section \ref{sec:filter2d}) & - \\ 
   \plugtabend
   
   \end{description}

   LatexEnd  
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iomanip>
#include <limits>
#include <mia/3d/fifof/byslice.hh>

NS_BEGIN(byslice_2dstack_filter)

NS_MIA_USE
using namespace std;


C2DBysliceFifoFilter::C2DBysliceFifoFilter(string filter):
	C2DImageFifoFilter(1, 1, 0)
{
	m_filter = C2DFilterPluginHandler::instance().produce(filter); 
}

void C2DBysliceFifoFilter::do_push(::boost::call_traits<P2DImage>::param_type x)
{
	TRACE("C2DBysliceFifoFilter::do_push");
	m_last_image = x;  
}

P2DImage C2DBysliceFifoFilter::do_filter()
{
	TRACE("C2DBysliceFifoFilter::do_filter");

	return m_filter->filter(*m_last_image); 
}


class C2DBysliceFifoFilterPlugin : public C2DFifoFilterPlugin {
public:
	C2DBysliceFifoFilterPlugin();
private:

	virtual const string do_get_descr() const;
	virtual bool do_test() const;
	virtual C2DImageFifoFilter *do_create()const;

	string m_filter;
};

C2DBysliceFifoFilterPlugin::C2DBysliceFifoFilterPlugin():
	C2DFifoFilterPlugin("byslice")
{
	add_parameter("filter", new CStringParameter(m_filter, true , "2D filter to be run"));
}

const string C2DBysliceFifoFilterPlugin::do_get_descr() const
{
	return "Runs a filter on a per slice basis. In essence, this is a wrapper that "
		"makes it possible to add pure 2D filters to the stack filter pipeline "
		"without ducplicating the implementation.";
}

typedef TFifoFilterSink<C2DImage> C2DImageFifoFilterSink;
bool C2DBysliceFifoFilterPlugin::do_test() const
{
	return true;
}

C2DImageFifoFilter *C2DBysliceFifoFilterPlugin::do_create()const
{
	
	return new C2DBysliceFifoFilter(m_filter);
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{

	return new C2DBysliceFifoFilterPlugin();
}
NS_END

