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

#include <mia/core/export_handler.hh>
#include <mia/3d/vfregularizer.hh>

#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>

NS_MIA_BEGIN

const char *C3DFVectorfieldRegularizer::type_descr = "regularizer";

C3DFVectorfieldRegularizer::~C3DFVectorfieldRegularizer()
{
}

double C3DFVectorfieldRegularizer::run(C3DFVectorfield& velocity, C3DFVectorfield& force, const C3DFVectorfield& deform) const
{
       assert(velocity.get_size() == m_size);
       assert(force.get_size() == m_size);
       assert(deform.get_size() == m_size);
       return do_run(velocity, force, deform);
}

void C3DFVectorfieldRegularizer::set_size(const C3DBounds& size)
{
       if (size != m_size) {
              m_size = size;
              on_size_changed();
       }
}

const C3DBounds& C3DFVectorfieldRegularizer::get_size() const
{
       return m_size;
}

void C3DFVectorfieldRegularizer::on_size_changed()
{
}

template <> const char   *const
TPluginHandler<C3DFVectorfieldRegularizerPlugin>::m_help =
       "This class of plug-ins implement various regularizations for "
       "3D vector fields that are typically used in image registration "
       "to translate the force driving the registration to a smooth "
       "velocity field.";

EXPLICIT_INSTANCE_HANDLER(C3DFVectorfieldRegularizer);

NS_MIA_END

