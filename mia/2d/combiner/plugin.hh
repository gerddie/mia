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

#ifndef mia_2d_combiner_plugin_hh
#define mia_2d_combiner_plugin_hh

NS_MIA_BEGIN

template <typename Combiner, const char *const name>
class T2DImageCombinerPlugin: public C2DImageCombinerPlugin
{
public:
       T2DImageCombinerPlugin();
private:
       virtual Combiner *do_create()const;
       virtual const std::string do_get_descr() const;
};


template <typename Combiner, const char *const name>
T2DImageCombinerPlugin<Combiner, name>::T2DImageCombinerPlugin():
       C2DImageCombinerPlugin(name)
{
}

template <typename Combiner, const char *const name>
Combiner *T2DImageCombinerPlugin<Combiner, name>::do_create()const
{
       return new Combiner;
}

template <typename Combiner, const char name[]>
const std::string T2DImageCombinerPlugin<Combiner, name>::do_get_descr() const
{
       return std::string("Image combiner '") +  std::string(name) + std::string("'");
}


NS_MIA_END

#endif
