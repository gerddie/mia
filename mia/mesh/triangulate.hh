/* -*- mona-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

	
#include <mia/mesh/clist.hh>
#include <mia/core/msgstream.hh>
#include <iostream>

NS_MIA_BEGIN

/**
   \brief class to make a triangle mesh from a closed polygon

   This class is used to convert a closed polygon into a mesh defined 
   by triangles. 

   \tparam VertexVector type of the vector that contains the vertices
   \tparam Polygon The type of the Polygon   

*/
template <class VertexVector, class Polygon>
class TPolyTriangulator {
public:
	/**
	   Constructor to be initializes with a vector of vertices that used when defining the polygon 
	   \param vv the vertices
	 */
	TPolyTriangulator(const VertexVector& vv);
	
	/**
	   Evaluate the triangulation of the polygon that is defined by the vertices given above and 
	   their ordering by indices as given in poly. 
	   \param [out] output the list of triangles as list of index triples into the vertex arrey given in the 
	   constructor 
	   \param [in] poly the index array into the vertex array describing the polygon 
	   \returns true of the triangulation was successfull and false if the polygon only consisted only 
	   of two corners.
	 */
	template <class TriangleList>
	bool triangulate(TriangleList& output, const Polygon& poly) const;

private:
	typedef clist<typename Polygon::value_type> CPoly;
	typedef typename VertexVector::value_type Vector;

	Vector eval_orientation(const Polygon& poly) const;
	bool is_convex(const typename CPoly::const_iterator& i, bool debug = false) const;
	bool is_ear(const typename CPoly::const_iterator& p, const CPoly& cpoly, bool debug = false) const;

	bool is_inside(
		const typename VertexVector::value_type& a,
		const typename VertexVector::value_type& b,
		const typename VertexVector::value_type& c,
		const typename VertexVector::value_type& p,
		bool debug = false ) const;

	
	const VertexVector& m_vv;
	mutable Vector m_orientation;
};

template <class VertexVector, class Polygon>
TPolyTriangulator<VertexVector,Polygon>::TPolyTriangulator(const VertexVector& vv):
	m_vv(vv)
{
}

template <class VertexVector, class Polygon>
template <class TriangleList>
bool 
TPolyTriangulator<VertexVector,Polygon>::triangulate(TriangleList& output, const Polygon& poly) const
{
	size_t poly_size = poly.size();
	
	if ( poly_size < 3) // no triangles at all
		return false;
	
	typedef clist<typename Polygon::value_type> CPoly; 
	
	CPoly cpoly; 

	m_orientation = eval_orientation(poly);

	typename Polygon::const_iterator pi = poly.begin();
	typename Polygon::const_iterator pe = poly.end();

	while (pi != pe)
		cpoly.push_back(*pi++);

	typename CPoly::iterator  p_i = cpoly.begin();
	typename CPoly::iterator  p_e = cpoly.end();

	p_i = p_i->succ; 
	
	while ( (p_i != p_e) && (poly_size > 3)) {
		if (is_ear(p_i, cpoly, false)) {
			// we have a valid triangle, store it 
				output.push_back(typename TriangleList::value_type(**p_i->succ, **p_i, **p_i->prev));
			
			// set the current middle node 
			typename CPoly::iterator  p_r = p_i;
			p_i = (p_i->prev != cpoly.begin()) ? p_i->prev : p_i->succ; 
			
			cpoly.remove(p_r);
			--poly_size;
		}else
			p_i = p_i->succ; 
	}
	
	if ((p_i == p_e) && (poly_size > 3)) {
		cvdebug()  <<"gotcha\n";
		
		p_i = cpoly.begin();
		
		p_i = p_i->succ; 
		
		while ( (p_i != p_e) && (poly_size > 3)) {
			
			if (is_ear(p_i, cpoly, true)) {
				// we have a valid triangle, store it 
				output.push_back(typename TriangleList::value_type(**p_i->succ, **p_i, **p_i->prev));
				
				// set the current middle node 
				typename CPoly::iterator  p_r = p_i;
			p_i = (p_i->prev != cpoly.begin()) ? p_i->prev : p_i->succ; 
			
			cpoly.remove(p_r);
			--poly_size;
			}else
				p_i = p_i->succ; 
		}
	}
	
	output.push_back(typename TriangleList::value_type(**p_i->succ, **p_i, **p_i->prev));
	return true; 
	

}



template <class VertexVector, class Polygon>
typename TPolyTriangulator<VertexVector,Polygon>::Vector
TPolyTriangulator<VertexVector,Polygon>::eval_orientation(const Polygon& poly) const
{
	typename VertexVector::value_type result(0,0,0); 
	
	typename Polygon::const_iterator pb = poly.begin();
	typename Polygon::const_iterator be = poly.end();

	typename Polygon::const_iterator c1 = pb; 
	++c1; 

	typename Polygon::const_iterator c2 = c1; 
	++c2; 
	
	typename VertexVector::value_type a = m_vv[*pb];
	
	while (c1 != be && c2 != be) {
		result += (m_vv[*c1++] - a) ^ (m_vv[*c2++] - a);
	}
	return result;

}

template <class VertexVector, class Polygon>
bool TPolyTriangulator<VertexVector,Polygon>::is_convex(const typename CPoly::const_iterator& i, bool /*debug*/) const
{
	typename VertexVector::value_type a = m_vv[**i->prev];
	typename VertexVector::value_type b = m_vv[**i];
	typename VertexVector::value_type c = m_vv[**i->succ];
	
	typename VertexVector::value_type ab = a - b;
	typename VertexVector::value_type cb = c - b; 
	
	typename VertexVector::value_type cross = ab ^ cb;

	const bool result = dot(cross, m_orientation) > 0; 
	
	return result; 

}

template <class VertexVector, class Polygon>
bool TPolyTriangulator<VertexVector,Polygon>::is_ear(const typename CPoly::const_iterator& p, const CPoly& cpoly, bool debug) const
{
	if (!is_convex(p,debug)) {
		cvdebug() << "corner is concave\n"; 
		return false; 
	}
	
	typename VertexVector::value_type a = m_vv[**p->prev];
	typename VertexVector::value_type b = m_vv[**p];
	typename VertexVector::value_type c = m_vv[**p->succ];
	
	cvdebug() << "check triangle" << a << b << c << " = (" << **p->prev << "," << **p << "," << **p->succ << "\n"; 
	
	
	typename CPoly::const_iterator i = cpoly.begin();
	i = i->succ;
	while (i != cpoly.end()) {
		if (i != p && i != p->prev && i != p->succ)
			if (!is_convex(i, debug) && is_inside(a,b,c,m_vv[**i], debug)) {
				cvdebug() << "point " << **i << ":" << m_vv[**i] << " is concave and inside\n"; 
		
				return false; 
			}
		
		i = i->succ;
	}
	return true; 
}

template <class VertexVector, class Polygon>
bool TPolyTriangulator<VertexVector,Polygon>::is_inside(
	const typename VertexVector::value_type& a,
	const typename VertexVector::value_type& b,
	const typename VertexVector::value_type& c,
	const typename VertexVector::value_type& p,
	bool /*debug*/) const
{
	double abc = ((a-b)^(c-b)).norm() * 0.5;
	double abp = ((a-p)^(b-p)).norm() * 0.5;
	double acp = ((a-p)^(c-p)).norm() * 0.5;
	double bcp = ((b-p)^(c-p)).norm() * 0.5;

	const bool result = (abc >= abp + acp + bcp); 
	
	return result; 
}

NS_MIA_END
