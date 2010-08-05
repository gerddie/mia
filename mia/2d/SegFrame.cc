
#include <stdexcept>
#include <string>
#include <mia/2d/SegFrame.hh>
#include <mia/core/msgstream.hh>

#include <libxml++/libxml++.h>
#include <boost/filesystem.hpp> 



NS_MIA_BEGIN
using namespace std; 
using namespace xmlpp; 

namespace bfs=boost::filesystem; 

CSegFrame::CSegFrame():
	m_has_star(false)
{
}

CSegFrame::CSegFrame(const string& image, const CSegStar& star, const Sections& sections):
	m_has_star(true),
	m_star(star), 
	m_sections(sections),  
	m_filename(image) 
{
}

CSegFrame::CSegFrame(const Node& node):
	m_has_star(false)
{
	TRACE("CSegFrame::CSegFrame"); 
	const Element& elm = dynamic_cast<const Element&>(node); 
		
	if (elm.get_name() != "frame")
		throw invalid_argument(string("CSegFrame: unexpected node type: ") + elm.get_name()); 
	
	const Attribute *attr = elm.get_attribute("image"); 
	if (!attr) {
		throw invalid_argument("CSegFrame: image attribute not found"); 
	}
	m_filename = attr->get_value(); 
	
	Node::NodeList nodes = elm.get_children(); 
	
	for (auto i = nodes.begin(); 
	     i != nodes.end(); ++i) {

		if ((*i)->get_name() == "star") {
			m_star = CSegStar(**i); 
			m_has_star = true; 
		}
		else if ((*i)->get_name() == "section") {
			m_sections.push_back(CSegSection(**i)); 
		}else {
			cvinfo() << "ignoring unsuported element '" << (*i)->get_name() << "'\n"; 
		}
	}
}

const std::string& CSegFrame::get_imagename() const
{
	return m_filename; 
}

void CSegFrame::set_imagename(const std::string& name)
{
	m_filename = name; 
}

void CSegFrame::rename_base(const std::string& new_base)
{
	bfs::path filename(m_filename); 
	string suffix = filename.extension(); 
	string name = filename.stem(); 
	auto i = name.rbegin();
	int k = 0; 
	while (i != name.rend() && isdigit(*i) ) {
		++i; 
		++k; 
	}
	string number(name.end() - k, name.end()); 
	m_filename = new_base + number + suffix; 
}

const CSegFrame::Sections& CSegFrame::get_sections()const
{
	return m_sections; 
}


const CSegStar& CSegFrame::get_star() const
{
	if (!m_has_star) 
		cvwarn() << "CSegFrame::get_star(): returing fake star"; 
	return m_star; 
}


void CSegFrame::write(Node& node) const
{
	Element* self = node.add_child("frame"); 
	self->set_attribute("image", m_filename); 	
	
	if (m_has_star) 
		m_star.write(*self);
	for (auto i = m_sections.begin(); 
	     i != m_sections.end(); ++i) {
		i->write(*self); 
	}
}

const C2DBoundingBox CSegFrame::get_boundingbox() const
{
	C2DBoundingBox result; 
	for (auto i = m_sections.begin(); 
	     i != m_sections.end(); ++i) {
		result.unite(i->get_boundingbox()); 
	}
	return result; 
}

void CSegFrame::shift(const C2DFVector& delta, const std::string& cropped_file)
{
	if (m_has_star) 
		m_star.shift(delta); 
	for (auto i = m_sections.begin(); 
	     i != m_sections.end(); ++i)
		i->shift(delta); 
	m_filename = cropped_file; 
}

void CSegFrame::transform(const C2DTransformation& t)
{
	if (m_has_star) 
		m_star.transform(t); 
	for (auto i = m_sections.begin(); 
	     i != m_sections.end(); ++i)
		i->transform(t); 
}

void CSegFrame::inv_transform(const C2DTransformation& t)
{
	if (m_has_star) 
		m_star.inv_transform(t); 
	for (auto i = m_sections.begin(); i != m_sections.end(); ++i)
		i->inv_transform(t); 
}


float CSegFrame::get_hausdorff_distance(const CSegFrame& other) const
{
	C2DPolygon p1; 
	for(auto s = m_sections.begin(); 
	    s != m_sections.end(); ++s) 
		s->append_to(p1); 


	C2DPolygon p2; 
	for(auto s = other.m_sections.begin(); 
	    s != other.m_sections.end(); ++s) 
		s->append_to(p2); 

	return p1.get_hausdorff_distance(p2); 
}


NS_MIA_END
