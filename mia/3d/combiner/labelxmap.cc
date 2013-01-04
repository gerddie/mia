/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <stdexcept>
#include <boost/type_traits.hpp>

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/file.hh>
#include <mia/3d/combiner/labelxmap.hh>


NS_BEGIN(labelxmap_3dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;

bool operator == (const CXLabelResult& a, const CXLabelResult& b)
{
	return a.m_map == b.m_map;
}

void CXLabelResult::add(size_t a , size_t b)
{
	if (a != 0 && b != 0) {
		XLabel l(a,b);
		CLabelMap::iterator i = m_map.find(l);
		if (i == m_map.end())
			m_map[l] = 1;
		else
			++i->second;
	}
}

void CXLabelResult::do_save(const std::string& fname) const
{
	ofstream f(fname.c_str(),ios_base::out);

	f << "#labelxmap\n";
	f << "size=" << m_map.size() << '\n';

	for (CLabelMap::const_iterator i = m_map.begin(); i != m_map.end(); ++i)
		f << i->first.first << " " <<  i->first.second <<  " " << i->second << '\n';



	if (!f.good()) {
		stringstream errmsg;
		errmsg << "CXLabelResult::do_save: error saving '" << fname << "'";
		throw runtime_error(errmsg.str());
	}
}


PCombinerResult CLabelXMap::do_combine( const C3DImage& a, const C3DImage& b) const
{
	if (a.get_size() != b.get_size())
		throw invalid_argument("CLabelXMap::combine: images are of different size");

	return ::mia::filter(*this, a, b);

}

template <typename U, typename V, bool integral>
struct xmap {
	static C3DImageCombiner::result_type apply(U /*ab*/, U /*ae*/, V /*bb*/) {
		throw invalid_argument("labelxmap: support only integral pixel types");
	}
};

template <typename U, typename V>
struct xmap<U, V, true> {
	static C3DImageCombiner::result_type apply(U ab, U ae, V bb) {
		CXLabelResult *r  = new CXLabelResult;
		C3DImageCombiner::result_type result(r);

		while (ab != ae) {
			r->add(*ab, *bb);
			++ab; ++bb;
		}
		return result;
	}
};

template <typename T, typename S>
C3DImageCombiner::result_type CLabelXMap::operator () ( const T3DImage<T>& a, const T3DImage<S>& b) const
{
	const bool is_integral = ::boost::is_integral<T>::value && ::boost::is_integral<S>::value;
	typedef typename T3DImage<T>::const_iterator U;
	typedef typename T3DImage<S>::const_iterator V;

	return xmap<U,V,is_integral>::apply(a.begin(), a.end(), b.begin());
}

CLabelXMapPlugin::CLabelXMapPlugin():
	C3DImageCombinerPlugin("labelxmap")
{
}

C3DImageCombiner *CLabelXMapPlugin::do_create()const
{
	return new CLabelXMap;
}

const string CLabelXMapPlugin::do_get_descr() const
{
	return "generate a label reference mapping";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CLabelXMapPlugin();
}

NS_END




