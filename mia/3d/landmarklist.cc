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

#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/landmarklist.hh>

NS_MIA_BEGIN

using std::invalid_argument;

const char *C3DLandmarklist::data_descr = "3dlandmarklist";

void C3DLandmarklist::add(P3DLandmark lm)
{
       assert(lm);
       auto p = m_list.find(lm->get_name());

       if (p == m_list.end()) {
              cvdebug() << "C3DLandmarklist: add '" << lm->get_name() << "'\n";
              m_list[lm->get_name()] = lm;
       } else {
              cvdebug() << "C3DLandmarklist: overwrite '" << lm->get_name() << "'\n";
              p->second = lm;
       }
}

C3DLandmarklist *C3DLandmarklist::clone() const
{
       return new C3DLandmarklist(*this);
}

P3DLandmark C3DLandmarklist::get(const std::string& name) const
{
       auto p = m_list.find(name);

       if (p != m_list.end())
              return p->second;
       else
              return P3DLandmark();
}

C3DLandmarklist::const_iterator C3DLandmarklist::begin() const
{
       return m_list.begin();
}

C3DLandmarklist::const_iterator C3DLandmarklist::end() const
{
       return m_list.end();
}

C3DLandmarklist::iterator C3DLandmarklist::begin()
{
       return m_list.begin();
}

C3DLandmarklist::iterator C3DLandmarklist::end()
{
       return m_list.end();
}

void C3DLandmarklist::set_path(const std::string& path)
{
       m_path = path;
}

void C3DLandmarklist::set_name(const std::string& name)
{
       m_name = name;
}

size_t C3DLandmarklist::size() const
{
       return m_list.size();
}

const std::string& C3DLandmarklist::get_name() const
{
       return m_name;
}

NS_MIA_END
