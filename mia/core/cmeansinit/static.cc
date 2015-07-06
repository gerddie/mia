/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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


#include <mia/core/cmeansinit/cmeansinit.hh>


NS_MIA_BEGIN

CEqualInitializer::CEqualInitializer(size_t nclasses):m_nclasses(nclasses)
{
}

CMeans::DVector CEqualInitializer::run(const NormalizedHistogram& nh) const
{
        CMeans::DVector result(m_nclasses);

        for (int i = 0; i < m_nclasses; ++i)
                result[i] = static_cast<double>(i) / ( m_nclasses - 1); 
        
        return result; 
}

CPredefinedInitializer::CPredefinedInitializer(const CMeans::DVector& init):m_init(init)
{
        // should check that this is in normalized range ? 
}

CMeans::DVector CPredefinedInitializer::run(const NormalizedHistogram& nh) const
{
        
        
        return m_init; 
}



// the class that has only the size as a paramater
CMeansInitializerSizedPlugin::CMeansInitializerSizedPlugin(const char *name):
        CMeansInitializerPlugin(name)
{
        add_parameter("nc", make_lo_param(m_size, 2, true, "Number of classes to use for the fuzzy-cmeans classification")); 
}

size_t CMeansInitializerSizedPlugin::get_size_param() const
{
        return m_size; 
}


CPredefinedInitializerPlugin::CPredefinedInitializerPlugin():
        CMeansInitializerPlugin("predefined")
        
{
        add_parameter("cc", make_lo_param(m_init, true,
                                          "Initial class centers fuzzy-cmeans classification (normalized to range [0,1])")); 
}

CMeansInitializerPlugin::Product * CPredefinedInitializerPlugin::do_create() const
{
        return new CPredefinedInitializer(m_init); 
}

const std::string CPredefinedInitializerPlugin::do_get_descr() const
{
        return "C-Means initializer that sets pre-defined values for the initial class centers"; 
}


CEqualInitializerPlugin::CEqualInitializerPlugin():
        CMeansInitializerPlugin("even")
{
}


CMeansInitializerPlugin::Product * CEqualInitializerPlugin::do_create() const
{
        return new CEqualInitializer(get_size_param());
}

const std::string CEqualInitializerPlugin::do_get_descr() const
{
        return "C-Means initializer that sets the initial class centers as evenly distributed over [0,1]"; 
}



NS_MIA_END
