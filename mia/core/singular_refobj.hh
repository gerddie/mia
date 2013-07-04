/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_singular_refobj_hh
#define mia_singular_refobj_hh


#include <mia/core/defines.hh>
#include <cassert>

NS_MIA_BEGIN


template <typename T> 
struct empty_desctructor {
	static void apply(T& MIA_PARAM_UNUSED(data)) {
	}; 
}; 

/**
   \brief a singulater reference counted object that gets destroyed when the refount goes to zero 

   This template handles singular objects that must no be copied (e.g. a resource)  
   but that do also not fit the sigleton pattern. An example use are the HDF5 file, group, and 
   dataset handles. 

*/


template <typename T, typename D = empty_desctructor<T>> 
class TSingleReferencedObject {
public: 
        TSingleReferencedObject(T data);
        
        TSingleReferencedObject(const TSingleReferencedObject<T,D>& other);

        TSingleReferencedObject& operator = (const TSingleReferencedObject<T,D>& other);

        ~TSingleReferencedObject(); 

        operator T()const; 

        unsigned get_refcount()const; 
        
private: 
        class TheObject {
        public: 
                TheObject(T data); 
                ~TheObject();
                void add_ref(); 
                bool del_ref();
                T get() const; 

                unsigned get_refcount()const; 
        private: 
                TheObject(const TheObject& data) = delete; 
                TheObject& operator = (const TheObject& data) = delete; 
                T m_data; 
                unsigned m_refcount; 
        }; 
        TheObject *m_object; 
}; 


template <typename T, typename D> 
TSingleReferencedObject<T,D>::TSingleReferencedObject(T data)
{
        m_object = new TheObject(data); 
}

template <typename T, typename D> 
TSingleReferencedObject<T,D>::TSingleReferencedObject(const TSingleReferencedObject<T,D>& other)
{
        m_object = other.m_object; 
        if (m_object)
                m_object->add_ref(); 
}

template <typename T, typename D> 
TSingleReferencedObject<T,D>& TSingleReferencedObject<T,D>::operator = (const TSingleReferencedObject<T,D>& other)
{
        if (m_object)
                m_object->del_ref(); 
        m_object = other.m_object; 
        if (m_object)
                m_object->add_ref(); 
	return *this; 
}

template <typename T, typename D> 
TSingleReferencedObject<T,D>::~TSingleReferencedObject()
{
        if (m_object)
                if (m_object->del_ref()) 
                        delete m_object; 
}

template <typename T, typename D> 
TSingleReferencedObject<T,D>::operator T() const 
{
        return m_object->get();
}

template <typename T, typename D> 
unsigned TSingleReferencedObject<T,D>::get_refcount()const
{
        if (m_object) 
                return m_object->get_refcount(); 
        else 
                return 0; 
}

template <typename T, typename D> 
TSingleReferencedObject<T,D>::TheObject::TheObject(T data):
        m_data(data), 
        m_refcount(1)
{
}

template <typename T, typename D> 
TSingleReferencedObject<T,D>::TheObject::~TheObject()
{
	assert(m_refcount == 0); 
	D::apply(m_data); 
}
                
template <typename T, typename D> 
void TSingleReferencedObject<T,D>::TheObject::add_ref()
{
        ++m_refcount; 
}

template <typename T, typename D> 
bool TSingleReferencedObject<T,D>::TheObject::del_ref()
{
        --m_refcount; 
        return (m_refcount <= 0); 
}

template <typename T, typename D> 
unsigned TSingleReferencedObject<T,D>::TheObject::get_refcount()const
{
	return m_refcount; 
}

template <typename T, typename D> 
T TSingleReferencedObject<T,D>::TheObject::get() const 
{
        return m_data; 
}

NS_MIA_END
#endif 
