
#include <stdexcept>
#include <string>
#include <mia/2d/SegFrame.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/errormacro.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/angle.hh>

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

void CSegFrame::set_image(P2DImage image)
{
	m_image = image; 
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


C2DUBImage CSegFrame::get_section_masks(const C2DBounds& size) const 
{
	C2DUBImage result(size); 
	unsigned char idx = 1; 
	for (auto i = m_sections.begin(); 
	     i != m_sections.end(); ++i, ++idx)
		i->draw(result, idx); 
	return result; 
}

void CSegFrame::load_image() const
{
	m_image = load_image2d(m_filename); 
	if (!m_image) 
		THROW(runtime_error, "unable to find image file '" << m_filename << "'");
}

C2DUBImage CSegFrame::get_section_masks() const
{
	if (!m_image)
		load_image(); 
	return get_section_masks(m_image->get_size()); 
}

C2DUBImage CSegFrame::get_section_masks(size_t n_sections) const
{
	if (!m_image) 		
		load_image(); 
	C2DUBImage result = get_section_masks(m_image->get_size()); 
	if (n_sections != m_sections.size()) {
		const C2DFVector ray_a = m_star.m_directions[0]; 
		
		const double scale = n_sections / (2 * M_PI); 
		
		// make a mask and re-run 
		auto i = result.begin(); 
		for (size_t y = 0; y < result.get_size().y; ++y)  {
			for (size_t x = 0; x < result.get_size().x; ++x, ++i)  {
				if (*i) {
					const C2DFVector ray_b(x - m_star.m_center.x, y - m_star.m_center.y); 
					const double a = angle(ray_a, ray_b);
					*i = (unsigned char) (a * scale + 1.0); 
				}
			}
		}
	}
	return result; 
}

struct EvalMaskStat: public TFilter<CSegFrame::SectionsStats> {

	EvalMaskStat(const C2DUBImage& mask):
		m_mask(mask) {
		max_idx = *max_element(mask.begin(), mask.end());
	}; 
	
	template <typename T> 
	CSegFrame::SectionsStats operator ()(const T2DImage<T>& image) const {
		CSegFrame::SectionsStats result(max_idx); 
		vector<size_t> size(max_idx, 0); 
		for(auto m=m_mask.begin(), i = image.begin(); m != m_mask.end(); ++i, ++m) {
			const double v = *i;
			if (*m) {
				result[*m-1].first  += v; 
				result[*m-1].second += v * v; 
				++size[*m-1]; 
			}
		}
		for (auto i = result.begin(), n=size.begin(); i != result.end(); ++i, ++n) {
			if (*n) 
				i->first /= *n; 
			if (*n > 1) 
				i->second = sqrt( (i->second - i->first * i->first * *n)/ (*n - 1)); 
		}
		return result; 
	}
private: 
	const C2DUBImage& m_mask; 
	unsigned char max_idx; 
}; 

CSegFrame::SectionsStats CSegFrame::get_stats(const C2DUBImage& mask) const
{
	if (!m_image)
		load_image(); 
	if (mask.get_size() != m_image->get_size()) 
		THROW(invalid_argument, "Mask image and data image are of different size");
	
	return ::mia::filter(EvalMaskStat(mask), *m_image);  


}

CSegFrame::SectionsStats CSegFrame::get_stats() const
{
	if (!m_image) {
		m_image = load_image2d(m_filename); 
		if (!m_image) 
			THROW(runtime_error, "unable to find image file '" << m_filename << "'");
	}
	C2DUBImage mask = get_section_masks(m_image->get_size()); 
	return get_stats(mask); 
}

NS_MIA_END
