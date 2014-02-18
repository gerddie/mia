/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <mia/3d/filter.hh>

NS_BEGIN(labelxmap_3dimage_filter)


typedef std::pair<size_t, size_t> XLabel;

struct LessXLabel {
	bool operator () (const XLabel& a, const XLabel& b)const
	{
		return a.first < b.first || (a.first == b.first && a.second < b.second);
	}
};

class CXLabelResult: public mia::CCombinerResult {
	friend bool operator == (const CXLabelResult& a, const CXLabelResult& b);
public:
	void add(size_t a , size_t b);
private:
	virtual void do_save(const std::string& fname) const;
        virtual boost::any do_get() const;  
	typedef std::map<XLabel, size_t, LessXLabel> CLabelMap;
	CLabelMap m_map;
};

class CLabelXMap: public mia::C3DImageCombiner {
public:
	template <typename T, typename S>
	mia::C3DImageCombiner::result_type operator () ( const mia::T3DImage<T>& a, const mia::T3DImage<S>& b) const;
private:
	virtual mia::PCombinerResult do_combine( const mia::C3DImage& a, const mia::C3DImage& b) const;



};
class CLabelXMapPlugin: public mia::C3DImageCombinerPlugin {
public:
	CLabelXMapPlugin();
private:
	virtual mia::C3DImageCombiner *do_create()const;
	virtual const std::string do_get_descr() const;

};

NS_END

