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

#include <mia/2d/filterchain.hh>

NS_MIA_BEGIN

using namespace std;

C2DFilterChain::C2DFilterChain()
{
}

C2DFilterChain::C2DFilterChain(const vector<const char *>& descr)
{
	const C2DFilterPluginHandler::Instance& filter_plugins = C2DFilterPluginHandler::instance();
	vector<const char *>::const_iterator i = descr.begin();
	vector<const char *>::const_iterator e = descr.end();

	for (; i != e; ++i) {
		P2DFilter f = filter_plugins.produce(*i);
		if (f)
			m_chain.push_back(f);
		else
			throw invalid_argument(*i);
	}
}

void C2DFilterChain::push_front(P2DFilter filter)
{
	assert(filter);
	m_chain.push_front(filter);
}

void C2DFilterChain::push_back(P2DFilter filter)
{
	assert(filter);
	m_chain.push_back(filter);
}

P2DImage C2DFilterChain::filter(const C2DImage& image) const
{
	P2DImage tmp(image.clone());
	list<P2DFilter>::const_iterator i = m_chain.begin();
	list<P2DFilter>::const_iterator e = m_chain.end();

	while (i != e) {
		tmp = (*i)->filter(*tmp);
		++i;
	}
	return tmp;
}

bool C2DFilterChain::empty() const
{
	return m_chain.empty();
}

NS_MIA_END
