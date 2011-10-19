/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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
   LatexBeginPluginDescription{3D image combiners}
   
   \subsection{Label cross-reference combiner}
   \label{combiners3d:labelxmap}
   
   \begin{description}
   
   \item [Plugin:] labelxmap
   \item [Description:] Takes two images of integral pixel values of the same size and evaluates 
                      teh overlap count of the combinations of intensities. 
   \item [Input:] Two integral valued gray scale images. 
   \item [Output:] A map that gives the number of overlapping voxels for each pair of non-zero 
                intensity values. 
   \item [Remark:] I really don't remember what this is used for.
   This plug-in doesn't take any additional parameters. 

   \end{description}

   LatexEnd  
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
#include <mia/3d/3dfilter.hh>


NS_BEGIN(labelxmap_3dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;

typedef pair<size_t, size_t> XLabel;


struct LessXLabel {
	bool operator () (const XLabel& a, const XLabel& b)const
	{
		return a.first < b.first || (a.first == b.first && a.second < b.second);
	}
};

class CXLabelResult: public CCombinerResult {
	friend bool operator == (const CXLabelResult& a, const CXLabelResult& b);
public:
	void add(size_t a , size_t b);
private:
	virtual void do_save(const std::string& fname) const;
	typedef map<XLabel, size_t, LessXLabel> CLabelMap;
	CLabelMap m_map;
};

bool operator == (const CXLabelResult& a, const CXLabelResult& b)
{
	return a.m_map == b.m_map;
}


class CLabelXMap: public C3DImageCombiner {
public:
	template <typename T, typename S>
	C3DImageCombiner::result_type operator () ( const T3DImage<T>& a, const T3DImage<S>& b) const;
private:
	virtual PCombinerResult do_combine( const C3DImage& a, const C3DImage& b) const;



};

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

class CLabelXMapPlugin: public C3DImageCombinerPlugin {
public:
	CLabelXMapPlugin();
private:
	virtual C3DImageCombiner *do_create()const;
	virtual const string do_get_descr() const;
	virtual bool do_test() const;

};

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

bool CLabelXMapPlugin::do_test() const
{
	CLabelXMap xmap;

	const int init_data1[10]   = {0, 0, 1, 2, 3, 4, 2, 1, 2, 0};
	const short init_data2[10] = {1, 0, 2, 1, 2, 3, 4, 2, 1, 1};

	CXLabelResult right_answer;

	right_answer.add(1,2);
	right_answer.add(1,2);
	right_answer.add(2,1);
	right_answer.add(2,1);
	right_answer.add(3,2);
	right_answer.add(4,3);
	right_answer.add(2,4);



	P3DImage image(new C3DUSImage(C3DBounds(1,1,1)));
	P3DImage fimage(new C3DFImage(C3DBounds(10,1,1)));

	P3DImage int_image(new C3DSIImage(C3DBounds(10,1,1), init_data1));
	P3DImage short_image(new C3DSSImage(C3DBounds(10,1,1), init_data2));

	bool success = true;
	try {
		xmap.combine( *image, *short_image);
		cvfail() << "combining image of different size should throw";
		success = false;
	}
	catch (invalid_argument& x) {
		cvdebug() << "Caught " << x.what() << "\n";
	}

	try {
		xmap.combine( *fimage, *short_image);
		cvfail() << "combining images of different type should throw\n";
		success = false;
	}
	catch (invalid_argument& x) {
		cvdebug() << "Caught " << x.what() << "\n";
	}

	C3DImageCombiner::result_type result = xmap.combine( *int_image, *short_image);

	CXLabelResult *xlr = dynamic_cast<CXLabelResult *>(result.get());
	if (!xlr) {
		cvfail() << "return type\n";
		return false;
	}

	return success && (*xlr == right_answer);

}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CLabelXMapPlugin();
}

NS_END




