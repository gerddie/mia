/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <mia/2d/segset.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/filetools.hh>
#include <mia/core/xmlinterface.hh>
#include <mia/core/tools.hh>

#include <iterator>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if LIBXMLPP_VERSION < 3
#define add_child_element add_child
#endif


NS_MIA_BEGIN
using namespace std;


CSegSet::CSegSet():
	m_RV_peak(-1),
	m_LV_peak(-1), 
	m_preferred_reference(-1), 
	m_version(2)
{
}

CSegSet::CSegSet(int version):
	m_RV_peak(-1),
	m_LV_peak(-1), 
	m_preferred_reference(-1), 
	m_version(version)
{
}

CSegSet::CSegSet(const std::string& src_filename):
	m_RV_peak(-1),
	m_LV_peak(-1),
	m_preferred_reference(-1), 
	m_version(2)
{
	ifstream infile(src_filename);
	string xml_init(std::istreambuf_iterator<char>{infile}, {});
	
	if (!infile.good())
		throw create_exception<runtime_error>("CSegSet: Unable to read input file: '", src_filename, "'");

	CXMLDocument doc(xml_init.c_str()); 
	
	read(doc);
}

CSegSet::CSegSet(const CXMLDocument& doc):
	m_RV_peak(-1),
	m_LV_peak(-1), 
	m_preferred_reference(-1), 
	m_version(1)
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

CXMLDocument CSegSet::write() const
{
	CXMLDocument result;
	
	auto nodeRoot = result.create_root_node("workset");

	if (m_version > 1) {
		nodeRoot->set_attribute("version", to_string<int>(m_version));
	}

	auto description = nodeRoot->add_child("description"); 
	auto  RVPeak = description->add_child("RVpeak"); 
	RVPeak->set_attribute("value", to_string<int>(m_RV_peak));
	auto  LVPeak = description->add_child("LVpeak"); 
	LVPeak->set_attribute("value", to_string<int>(m_LV_peak));
	auto  PreferedRef = description->add_child("PreferedRef"); 
	PreferedRef->set_attribute("value", to_string<int>(m_preferred_reference));


	for(auto i: m_frames)
		i.write(*nodeRoot, m_version);

	return result;
}

void CSegSet::read(const CXMLDocument& doc)
{
	auto root = doc.get_root_node();
	assert(root); 
	
	if (root->get_name() != "workset") {
		throw invalid_argument(string("CSegSet: Document root node: expected 'workset', but got ") +
				       root->get_name());
	}

	// without attribute its version 1, otherwise read the version. 
	auto attr = root->get_attribute("version"); 
	if (!attr.empty()) {
		if (!from_string(attr, m_version)) 
			throw create_exception<invalid_argument>("bogus version '", 
								 attr, 
								 "' in segmentation set"); 
	}
	cvdebug() << "Read version " << m_version << "\n"; 
	
	auto frames = root->get_children("frame");

	for (auto i: frames) {
		try {
			m_frames.push_back(CSegFrame(*i, m_version));
		}
		catch (invalid_argument& x) {
			throw create_exception<invalid_argument>("Segset: Error reading frames:", x.what());  
		}
	}

	auto descr = root->get_children("description");
	if (!descr.empty()) 
		descr = descr[0]->get_all_children();
	
	for(auto i : descr) {
		cvdebug() << "description element '" << i->get_name() << "'\n"; 
		if (i->get_name() == "RVpeak") {
			auto attr = i->get_attribute("value"); 
			if (attr.empty())
				cvwarn() << "CSegFrame: LVpeak without attribute"; 
			else 
				if (!from_string(attr, m_RV_peak)) {
					cvwarn() << "Could't convert RV_peak attribute '" << attr
						 <<"' to an integer; ignoring\n";
					m_RV_peak = -1; 

				}
		} else if (i->get_name() == "LVpeak") {
			auto attr = i->get_attribute("value"); 
			if (attr.empty())
				cvwarn() << "CSegFrame: LVpeak without attribute"; 
			else 	
				if (!from_string(attr, m_LV_peak)) {
					cvwarn() << "Could't convert LV_peak attribute '" << attr
						 <<"' to an integer; ignoring\n"; 
					m_LV_peak = -1; 
				}
		} else if (i->get_name() == "PreferedRef") {
			auto attr = i->get_attribute("value"); 
			if (attr.empty())
				cvwarn() << "CSegFrame: PreferedRef without attribute"; 
			else 	
				if (!from_string(attr, m_preferred_reference)) {
					cvwarn() << "Could't convert PreferedRef attribute '" << attr
						 <<"' to an integer; ignoring\n"; 
					m_preferred_reference = -1; 
				}
		}
		else {
			cvinfo() << "Ignoring unknown element '" << i->get_name() << "'\n"; 
		}
	}
}

CSegSet  CSegSet::shift_and_rename(size_t skip, const C2DFVector&  shift, const std::string& new_filename_base) const
{
	CSegSet result(*this);
	auto iframe = result.get_frames().begin();
	auto eframe = result.get_frames().end();

	while (skip-- && iframe != eframe)
		++iframe;

	while (iframe != eframe ) {
		string base, suffix, number;
		split_filename_number_pattern(iframe->get_imagename(), base, suffix, number);
		stringstream fname;
		fname << new_filename_base << number << suffix;
		CSegFrame& rframe = *iframe;
		rframe.shift(shift, fname.str());
		++iframe;
	}
	return result;
}

void CSegSet::transform(const C2DTransformation& t)
{
	for (auto i = get_frames().begin(); i != get_frames().end(); ++i) 
		i->transform(t); 
}

void CSegSet::set_RV_peak(int peak)
{
	m_RV_peak = peak; 
}

int CSegSet::get_RV_peak() const
{
	return m_RV_peak; 
}


void CSegSet::set_LV_peak(int peak)
{
	m_LV_peak = peak; 
}

int CSegSet::get_LV_peak() const
{
	return m_LV_peak; 
}

int CSegSet::get_preferred_reference() const
{
	return m_preferred_reference; 
}

void  CSegSet::set_preferred_reference(int value)
{
	m_preferred_reference = value; 
}

NS_MIA_END
