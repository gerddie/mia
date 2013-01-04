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

#include <mia/3d/creator.hh>


NS_BEGIN(creator_lattic_3d);

class C3DLatticCreator	: public mia::C3DImageCreator {
public:
	C3DLatticCreator(const mia::C3DFVector& freq);
	virtual mia::P3DImage operator () (const mia::C3DBounds& size, mia::EPixelType type) const;
private:
 	mia::C3DFImage do_create(const mia::C3DBounds& size) const;
	mia::C3DFVector m_freq; 
};

class C3DLatticCreatorPlugin : public  mia::C3DImageCreatorPlugin {
public:
	C3DLatticCreatorPlugin();
private:
	virtual mia::C3DImageCreator *do_create()const;
	virtual const std::string do_get_descr()const;
	mia::C3DFVector m_freq;
};


NS_END
