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



/**
   \brief a singulater reference counted object that gets destroyed when the refount goes to zero 

   This template handles singular objects that must no be copied (e.g. a resource)  
   but that do also not fit the sigleton pattern. An example use are the HDF5 file, group, and 
   dataset handles. 

   \tparam T type of the data to be hold
   \tparam D type of a functor that frees the resource. It must provide a static method 
         static void apply(T& data) that frees the resource. 

*/


template <typename T> 
class TSingleReferencedObject {
public: 
	struct Destructor {
		virtual void operator ()(T& MIA_PARAM_UNUSED(data)) const = 0;
	}; 

	struct EmptyDestructor : public Destructor{
		virtual void operator ()(T& MIA_PARAM_UNUSED(data))const {}
	};

	static const EmptyDestructor empty_destructor; 

	TSingleReferencedObject(); 

        TSingleReferencedObject(T data, const Destructor& d = empty_destructor);
        
        TSingleReferencedObject(const TSingleReferencedObject<T>& other);

        TSingleReferencedObject& operator = (const TSingleReferencedObject<T>& other);

        ~TSingleReferencedObject(); 

        operator T()const; 

        unsigned get_refcount()const; 
        
private: 
        class TheObject {
        public: 
                TheObject(T data, const Destructor& d = empty_destructor); 
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
		const Destructor& m_destructor; 
        }; 
        TheObject *m_object; 
}; 

template <typename T> 
const typename TSingleReferencedObject<T>::EmptyDestructor TSingleReferencedObject<T>::empty_destructor; 


template <typename T> 
TSingleReferencedObject<T>::TSingleReferencedObject():
	m_object(nullptr)
{
}

template <typename T> 
TSingleReferencedObject<T>::TSingleReferencedObject(T data, const Destructor& d)
{
        m_object = new TheObject(data, d); 
}

template <typename T> 
TSingleReferencedObject<T>::TSingleReferencedObject(const TSingleReferencedObject<T>& other)
{
        m_object = other.m_object; 
        if (m_object)
                m_object->add_ref(); 
}

template <typename T> 
TSingleReferencedObject<T>& TSingleReferencedObject<T>::operator = (const TSingleReferencedObject<T>& other)
{
        if (m_object)
                m_object->del_ref(); 
        m_object = other.m_object; 
        if (m_object)
                m_object->add_ref(); 
	return *this; 
}

template <typename T> 
TSingleReferencedObject<T>::~TSingleReferencedObject()
{
        if (m_object)
                if (m_object->del_ref()) 
                        delete m_object; 
}

template <typename T> 
TSingleReferencedObject<T>::operator T() const 
{
        return m_object->get();
}

template <typename T> 
unsigned TSingleReferencedObject<T>::get_refcount()const
{
        if (m_object) 
                return m_object->get_refcount(); 
        else 
                return 0; 
}

template <typename T> 
TSingleReferencedObject<T>::TheObject::TheObject(T data, const Destructor& d):
        m_data(data), 
        m_refcount(1), 
	m_destructor(d)
{
}

template <typename T> 
TSingleReferencedObject<T>::TheObject::~TheObject()
{
	assert(m_refcount == 0); 
	m_destructor(m_data); 
}
                
template <typename T> 
void TSingleReferencedObject<T>::TheObject::add_ref()
{
        ++m_refcount; 
}

template <typename T> 
bool TSingleReferencedObject<T>::TheObject::del_ref()
{
        --m_refcount; 
        return (m_refcount <= 0); 
}

template <typename T> 
unsigned TSingleReferencedObject<T>::TheObject::get_refcount()const
{
	return m_refcount; 
}

template <typename T> 
T TSingleReferencedObject<T>::TheObject::get() const 
{
        return m_data; 
}

NS_MIA_END
#endif 
