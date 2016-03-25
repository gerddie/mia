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

#include <iostream>
#include <sstream> 
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <mia/3d/rot.hh>
#include <mia/core/attributes.cxx>

NS_MIA_BEGIN

using std::string; 
using std::istringstream; 
using std::ostringstream; 
using std::vector; 
using std::invalid_argument; 

static const char * c_rot_identity = "rot-identity"; 
static const char * c_rot_mat = "rot-matrix"; 
static const char * c_rot_quat = "rot-quaternion"; 



class C3DRotationImpl {
        
public: 
	virtual ~C3DRotationImpl(); 
	virtual C3DRotationImpl *clone() const  __attribute__((warn_unused_result))= 0 ; 
        virtual C3DDMatrix as_matrix_3x3() const = 0;
        virtual Quaternion as_quaternion() const = 0; 
        virtual std::string as_string() const = 0; 
        
        static C3DRotationImpl* from_string(const std::string& s) __attribute__((warn_unused_result));
}; 

class C3DQuaternionRotation: public C3DRotationImpl {

public: 
        C3DQuaternionRotation(const std::string& s); 
        C3DQuaternionRotation(const Quaternion& q); 

	
	virtual C3DRotationImpl *clone() const  __attribute__((warn_unused_result)); 
        virtual C3DDMatrix as_matrix_3x3() const;
        virtual Quaternion as_quaternion() const; 
        virtual std::string as_string() const; 

private: 
        Quaternion m_q;
        
}; 

class C3DMatrix3x3Rotation: public C3DRotationImpl {
public: 
        C3DMatrix3x3Rotation(const std::string& s);
        C3DMatrix3x3Rotation(const C3DDMatrix& q); 
        
	virtual C3DRotationImpl *clone() const  __attribute__((warn_unused_result)); 
        virtual C3DDMatrix as_matrix_3x3() const;
        virtual Quaternion as_quaternion() const; 
        virtual std::string as_string() const; 

private: 

        C3DDMatrix m_matrix;
}; 

C3DRotation::C3DRotation():impl(nullptr)
{
}

C3DRotation::~C3DRotation()
{
	delete impl; 
}

C3DRotation::C3DRotation(const C3DRotation& other)
{
	if (other.impl) 
		impl = other.impl->clone(); 
	else 
		impl = nullptr; 
}

C3DRotation& C3DRotation::operator = (const C3DRotation& other)
{
	if (this != &other) {
		auto old = impl; 
		if (other.impl) 
			impl = other.impl->clone(); 
		else 
			impl = nullptr;
		delete old; 
	}
	return *this; 
}

C3DRotation::C3DRotation(const C3DDMatrix& m)
{
	impl = new C3DMatrix3x3Rotation(m); 
}

C3DRotation::C3DRotation(const Quaternion& q)
{
	impl = new C3DQuaternionRotation(q); 
}
	
C3DRotation::C3DRotation(const std::string& s)
{
	impl = C3DRotationImpl::from_string(s); 
}

C3DDMatrix C3DRotation::as_matrix_3x3() const
{
	return impl ? impl->as_matrix_3x3() : C3DDMatrix::_1; 
}

Quaternion C3DRotation::as_quaternion() const
{
	return impl ? impl->as_quaternion() : Quaternion::_1; 
}

string C3DRotation::as_string() const
{
	return impl ? impl->as_string() : string(c_rot_identity); 
}

const C3DRotation C3DRotation::_1; 

C3DRotationImpl::~C3DRotationImpl()
{
}

C3DRotationImpl* C3DRotationImpl::from_string(const std::string& s) 
{
        vector<string> tockens; 
        boost::split(tockens, s ,boost::is_any_of("="));

	if ((tockens.size() == 1) && !strcmp(c_rot_identity, tockens[0].c_str()))
		return nullptr;
	
        if (tockens.size() != 2) {
                throw create_exception<invalid_argument>("Unable to read C3DRotation from '", 
                                                         s, "'"); 
        }
        
        const char *test_tocken =  tockens[0].c_str(); 
        if (!strcmp(c_rot_mat, test_tocken))
                return new C3DMatrix3x3Rotation(tockens[1]); 
        else if (!strcmp(c_rot_quat, test_tocken))
                return new C3DQuaternionRotation(tockens[1]);
	else
                throw create_exception<invalid_argument>("Unknown C3DRotation type '", tockens[0], "'"); 
}

C3DRotationImpl *C3DQuaternionRotation::clone() const 
{
	return new C3DQuaternionRotation(*this); 
}

C3DQuaternionRotation::C3DQuaternionRotation(const std::string& s)
{
        vector<string> tockens; 
        boost::split(tockens, s ,boost::is_any_of(","));
        
        if (tockens.size() != 4) {
                throw create_exception<invalid_argument>("Expect four values to create quaternions but got ", tockens.size()); 
        }
        
        double v[4]; 
        for (int i = 0; i < 4; ++i) {
                istringstream iss(tockens[i]); 
                iss >> v[i]; 
                if (iss.bad()) 
                        throw create_exception<invalid_argument>("Unable to read quaternions from '", s, "'"); 
        }
        m_q = Quaternion(v[0], v[1], v[2], v[3]); 
}

C3DQuaternionRotation::C3DQuaternionRotation(const Quaternion& q):
      m_q(q)
{
        
}

C3DDMatrix C3DQuaternionRotation::as_matrix_3x3() const
{
        return m_q.get_rotation_matrix(); 
}

Quaternion C3DQuaternionRotation::as_quaternion() const
{
        return m_q; 
}

std::string C3DQuaternionRotation::as_string() const
{
        ostringstream s; 
        s << c_rot_quat << "=" <<m_q; 
        return s.str(); 
}

C3DRotationImpl *C3DMatrix3x3Rotation::clone()const 
{
	return new C3DMatrix3x3Rotation(*this); 
}

C3DMatrix3x3Rotation::C3DMatrix3x3Rotation(const std::string& s)
{
        vector<string> tockens; 
        boost::split(tockens, s ,boost::is_any_of(";"));
	
        if (tockens.size() != 3) {
                throw create_exception<invalid_argument>("Unable to read C3DMatrix3x3Rotation from '", 
                                                         s, "'"); 
        }

        istringstream iss_x(tockens[0]); 
        iss_x >> m_matrix.x; 
        if (iss_x.bad()) {
                throw create_exception<invalid_argument>("Unable to rotation matrix x row from '", tockens[0], "'");
        }

        istringstream iss_y(tockens[1]); 
        iss_y >> m_matrix.y; 
        if (iss_y.bad()) {
                throw create_exception<invalid_argument>("Unable to rotation matrix y row from '", tockens[1], "'");
        }

        istringstream iss_z(tockens[2]); 
        iss_z >> m_matrix.z; 
        if (iss_z.bad()) {
                throw create_exception<invalid_argument>("Unable to rotation matrix z row from '", tockens[2], "'");
        }

}

C3DMatrix3x3Rotation::C3DMatrix3x3Rotation(const C3DDMatrix& m):
m_matrix(m)
{
        
}


C3DDMatrix C3DMatrix3x3Rotation::as_matrix_3x3() const
{
        return m_matrix; 
}

Quaternion C3DMatrix3x3Rotation::as_quaternion() const
{
        return Quaternion(m_matrix); 
}

std::string C3DMatrix3x3Rotation::as_string() const
{
        ostringstream s; 
        s << c_rot_mat << "="<< m_matrix; 
        return s.str(); 
}

EXPORT_3D bool operator == (const C3DRotation& lhs, const C3DRotation& rhs)
{
	// this needs to be done floating point value friendly
	auto q = lhs.as_quaternion(); 
	q -= rhs.as_quaternion(); 
	
	return (q.norm() < 1e-10); 
}

EXPORT_3D bool operator < (const C3DRotation& lhs, const C3DRotation& rhs) 
{
	auto qlhs = lhs.as_quaternion(); 
	auto qrhs = rhs.as_quaternion(); 

	return qlhs.w() < qrhs.w() || 
		(qlhs.w() == qrhs.w() && 
		 (qlhs.z() < qrhs.z() || 
		  (qlhs.z() == qrhs.z() && 
		   ( qlhs.y() < qrhs.y() || 
		     ( qlhs.y() == qrhs.y() && qlhs.x() < qrhs.x()))))); 
		  
}

template class EXPORT_3D TAttribute<C3DRotation>; 
template class EXPORT_3D TTranslator<C3DRotation>;

NS_MIA_END
