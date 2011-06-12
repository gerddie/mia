/* -*- mia-c++  -*-
 *
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
/* 
   atexBeginPluginDescription{2D image stack filters}
   
   \subsection{Label}
   \label{fifof:label}
   
   \begin{description}
   
   \item [Plugin:] label
   \item [Description:] Labels connected components in a series of binary images. 
      Since lables may join after preceeding label images are already saved and 
      discarded from the pipeline stage, a map for labels that were joined is 
      created. 
   \item [Input:] Binary images, all of the same size
   \item [Output:] The labeled image(s) 
   
   \plugtabstart
   \plugtabend
   shape & string & shape to define neighbourhood of voxels, only simple 2D neighborhoods are supported & 4n \\ 
   map & string & file name where the label join map will be saved &  - \ \
   \end{description}

   atexEnd  
 */

#include <limits>
#include <queue>
#include <ostream>
#include <fstream>
#include <set>
#include <map>
#include <ctime>
#include <mia/3d/fifof/label.hh>


NS_BEGIN(label_2dstack_filter)

NS_MIA_USE; 
using namespace std; 

C2DLabelStackFilter::C2DLabelStackFilter(const string& mapfile, P2DShape n):
	C2DImageFifoFilter(1, 1, 0), 
	m_neigbourhood(n), 
	m_last_label(1), 
	m_map_file(mapfile), 
	m_first_pass(true)
{
}
			   
C2DLabelStackFilter::~C2DLabelStackFilter()
{
}

static int slice = 0; 
void C2DLabelStackFilter::do_initialize(::boost::call_traits<mia::P2DImage>::param_type x)
{
	m_first_pass = true; 
	m_joints.clear(); 
	m_out_buffer = C2DUSImage(x->get_size()); 
	m_last_label = 1; 
}

void C2DLabelStackFilter::grow( int x, int y, C2DBitImage& input, unsigned short l)
{
	vector<C2DBounds> seed; 
	seed.push_back(C2DBounds(x,y)); 
	
	while (!seed.empty()) {
		C2DBounds loc = seed.back(); 
		seed.pop_back(); 
		for (auto si = m_neigbourhood->begin(); 
		     si != m_neigbourhood->end(); ++si) {
			size_t px = si->x + loc.x; 
			size_t py = si->y + loc.y; 
			if (px >= input.get_size().x ||
			    py >= input.get_size().y) 
				continue; 
			
			unsigned short lold = m_out_buffer(px, py); 
			if (lold) {
				if (l != lold) {
					if (lold > l) 
						m_joints.insert(T2DVector<unsigned short>(l, lold)); 
					else
						m_joints.insert(T2DVector<unsigned short>(lold, l)); 
					// overwrite old label to reduce numbers of lables. 
					cvdebug () << "overwrite " << lold << " with " << l << "\n"; 
					m_out_buffer(px, py) = l; 
					cvdebug () << "Now at " << m_joints.size() << " joints\n"; 
				}
			}else if (input(px, py)) {
				m_out_buffer(px, py) = l; 
				input(px, py) = false; 
				seed.push_back(C2DBounds(px,py)); 
			}
		}
	}
}

void C2DLabelStackFilter::label_new_regions(C2DBitImage& input)
{
	C3DBitImage::iterator ii = input.begin();
	C3DUSImage::iterator usi = m_out_buffer.begin(); 
	
	for (size_t y = 0; y < input.get_size().y; ++y) 
		for (size_t x = 0; x < input.get_size().x; ++x, ++usi, ++ii) {
			if (*ii) {
				cvdebug() << "("<< x << ", " << y <<"," << slice <<  "):" 
					  << m_last_label << "\n"; 
				*usi = m_last_label++;
				*ii = false; 
				grow(x,y,input,*usi); 
			}
		}
}

void C2DLabelStackFilter::label(C2DBitImage& input)
{
	// first grow all regions that are already labeled from the last slice
	C3DUSImage::iterator usi = m_out_buffer.begin(); 
	for (size_t y = 0; y < input.get_size().y; ++y) 
		for (size_t x = 0; x < input.get_size().x; ++x, ++usi) {
			if ( *usi )
				grow(x,y, input, *usi);
		}
	
	// then label all not yet connected regions 
	label_new_regions(input); 
}

void C2DLabelStackFilter::new_label(C2DBitImage& input)
{
	m_out_buffer = C2DUSImage(input.get_size(), input); 
	label_new_regions(input); 
}

void  C2DLabelStackFilter::re_label(C2DBitImage& input)
{
	
	C3DUSImage::iterator usi = m_out_buffer.begin(); 
	C3DUSImage::iterator use = m_out_buffer.end(); 
	C2DBitImage::iterator ii = input.begin(); 

	// maintain old labeling for new slice, and clean the input at 
	// labeled positions
	while (usi != use)  {
		if (!*ii) 
			*usi = 0; 
		else if (*usi) 
			*ii = 0; 
		++ii; 
		++usi; 
	}

	// now continue normal labeling 
	label(input); 
}

void C2DLabelStackFilter::do_push(::boost::call_traits<mia::P2DImage>::param_type x)
{
	
	const C2DBitImage *inp = dynamic_cast<const C2DBitImage*>(x.get()); 
	if (!inp) 
		throw invalid_argument("C2DLabelStackFilter: input images must be binary"); 
	
	C2DBitImage input(*inp); 
	if (m_first_pass) {
		new_label(input); 
		m_first_pass = false; 
	}else {
		re_label(input);  
	}
	++slice; 

}

mia::P2DImage C2DLabelStackFilter::do_filter()
{
	return P2DImage(new C2DUSImage(m_out_buffer)); 
}
void C2DLabelStackFilter::post_finalize()
{
	priority_queue<T2DVector<unsigned short> > sorted; 
	cvdebug()<< "got " << m_joints.size() << " joints\n"; 

	for (auto i = m_joints.begin();  i != m_joints.end();  ++i)
		sorted.push(*i); 
	
	while (!sorted.empty()) {
		auto v = sorted.top(); 
		sorted.pop();
		
		auto m = m_target.find(v.y);
		if (m != m_target.end())  
			m_target[v.x] = m->second; 
		else 
			m_target[v.y] = v.x; 
	}
	if (!m_map_file.empty()) {
		ofstream outfile(m_map_file.c_str(), ios_base::out );
		m_target.save(outfile); 
		if (!outfile.good()) {
			THROW(runtime_error, "C2DLabelStackFilter: failed to save labale join map to '"
			      << m_map_file << "'"); 
		}
	}
}

const CLabelMap& C2DLabelStackFilter::get_joints() const
{
	return m_target; 
}


class C2DLabelFifoFilterPlugin : public C2DFifoFilterPlugin {
public:
	C2DLabelFifoFilterPlugin();
private:

	virtual const string do_get_descr() const;
	virtual bool do_test() const;
	virtual C2DFifoFilterPlugin::ProductPtr do_create()const;

	string m_neighborhood;
	string m_mapfile;
};

C2DLabelFifoFilterPlugin::C2DLabelFifoFilterPlugin():
	C2DFifoFilterPlugin("label"),
	m_neighborhood("4n")
{
	add_parameter("n", new CStringParameter(m_neighborhood, false, 
						"2D neighborhood shape to define connectedness"));
	add_parameter("map", new CStringParameter(m_mapfile, true, 
						  "Mapfile to save label numbers that are joined"));
}

const string C2DLabelFifoFilterPlugin::do_get_descr() const
{
	return "Stack Label filter";
}

bool C2DLabelFifoFilterPlugin::do_test() const
{
	return true;
}

C2DFifoFilterPlugin::ProductPtr C2DLabelFifoFilterPlugin::do_create()const
{
	auto shape = C2DShapePluginHandler::instance().produce(m_neighborhood); 
	return C2DFifoFilterPlugin::ProductPtr(new C2DLabelStackFilter(m_mapfile, shape));
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{

	return new C2DLabelFifoFilterPlugin();
}


NS_END
