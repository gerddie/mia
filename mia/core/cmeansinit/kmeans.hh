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


#include <mia/core/cmeans.hh>


NS_MIA_BEGIN

class CKMeansInitializer : public CMeans::Initializer {
public: 
        CKMeansInitializer(size_t nclasses);
        
        CMeans::DVector run(const CMeans::NormalizedHistogram& nh) const;
private: 
        size_t m_nclasses; 
        
}; 

class CKMeansInitializerPlugin : public CMeansInitializerSizedPlugin {
public: 
        CKMeansInitializerPlugin();
private:
        CMeansInitializerPlugin::Product * do_create() const;
        virtual const std::string do_get_descr() const;
}; 


NS_MIA_END
