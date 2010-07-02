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

#ifndef SegSet_h
#define SegSet_h

#include <mia/2d/SegFrame.hh>
#include <mia/2d/BoundingBox.hh>

namespace xmlpp {
	class Document;
};

NS_MIA_BEGIN

class EXPORT_2D CSegSet {
public:
	typedef std::vector<CSegFrame> Frames;

	CSegSet();

	CSegSet(const std::string& src_filename);
	CSegSet(const xmlpp::Document& node);

	void add_frame(const CSegFrame& frame);
	xmlpp::Document *write() const;

	const Frames& get_frames()const;
	Frames& get_frames();
	const C2DBoundingBox get_boundingbox() const;

	CSegSet  shift_and_rename(size_t skip, const C2DFVector&  shift, const std::string& new_filename_base)const;
private:
	void read(const xmlpp::Document& node);
	Frames m_frames;
};


NS_MIA_END

#endif
