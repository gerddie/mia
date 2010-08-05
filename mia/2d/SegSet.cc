/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/2d/SegSet.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/filetools.hh>
#include <libxml++/libxml++.h>


NS_MIA_BEGIN
using namespace std;
using namespace xmlpp;


CSegSet::CSegSet()
{
}

CSegSet::CSegSet(const std::string& src_filename)
{
	DomParser parser;
	parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	parser.parse_file(src_filename);

	if (!parser)
		throw runtime_error(string("CSegSet: Unable to parse input file:") + src_filename);

	read(*parser.get_document());

}

CSegSet::CSegSet(const xmlpp::Document& doc)
{
	TRACE("CSegSet::CSegSet");
	read(doc);
}

void CSegSet::add_frame(const CSegFrame& frame)
{
	m_frames.push_back(frame);
}

const CSegSet::Frames& CSegSet::get_frames() const
{
	return m_frames;
}

CSegSet::Frames& CSegSet::get_frames()
{
	return m_frames;
}

void CSegSet::rename_base(const std::string& new_base)
{
	for (auto i = m_frames.begin(); i != m_frames.end(); ++i) 
		i->rename_base(new_base);
}

const C2DBoundingBox CSegSet::get_boundingbox() const
{
	C2DBoundingBox result;

	for(Frames::const_iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		result.unite(i->get_boundingbox());

	return result;
}

xmlpp::Document *CSegSet::write() const
{
	xmlpp::Document *doc = new xmlpp::Document;
	xmlpp::Element* nodeRoot = doc->create_root_node("workset");

	for(Frames::const_iterator i = m_frames.begin(); i != m_frames.end(); ++i) {
		i->write(*nodeRoot);
	}
	return doc;
}

void CSegSet::read(const xmlpp::Document& node)
{
	const xmlpp::Element *root = node.get_root_node ();
	if (root->get_name() != "workset") {
		throw invalid_argument(string("CSegSet: Document root node: expected 'workset', but got ") +
				       root->get_name());
	}

	xmlpp::Node::NodeList frames = root->get_children("frame");
	xmlpp::Node::NodeList::const_iterator i = frames.begin();
	xmlpp::Node::NodeList::const_iterator e = frames.end();

	while (i != e) {
		m_frames.push_back(CSegFrame(**i));
		++i;
	}
}

CSegSet  CSegSet::shift_and_rename(size_t skip, const C2DFVector&  shift, const std::string& new_filename_base) const
{
	CSegSet result;
	auto iframe = get_frames().begin();
	auto eframe = get_frames().end();

	while (skip-- && iframe != eframe)
		++iframe;

	while (iframe != eframe ) {
		string base, suffix, number;
		split_filename_number_pattern(iframe->get_imagename(), base, suffix, number);
		stringstream fname;
		fname << new_filename_base << number << suffix;
		CSegFrame rframe = *iframe;
		rframe.shift(shift, fname.str());

		result.add_frame(rframe);
		++iframe;
	}
	return result;
}

void CSegSet::transform(const C2DTransformation& t)
{
	for (auto i = get_frames().begin(); i != get_frames().end(); ++i) 
		i->transform(t); 
}

NS_MIA_END
