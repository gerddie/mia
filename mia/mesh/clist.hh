/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

	
#ifndef CLIST_HH
#define CLIST_HH

#include <cassert>

NS_MIA_BEGIN

template <class T> 
class clist {
	
public:
	typedef T value_type; 
	

	struct node {
		T value;
		node *prev; 
		node *succ; 
		node(T v, node *p, node *s):
			value(v), prev(p), succ(s)
		{
		}
		
		T& operator *() {
			return value; 
		}
		const T& operator *() const {
			return value; 
		}

		
	};

	typedef node *iterator; 
	typedef const node *const_iterator; 

	
	clist(): m_head(NULL){
		
	}
	~clist() {
		if (m_head != NULL) {
			node *head = m_head;
			while (head != head->succ) 
				remove(head->succ);
			delete m_head; 
		}
	}
	
	iterator begin() {
		return m_head; 
	}
	
	iterator end() {
		return m_head;
	}

	const_iterator begin() const {
		return m_head; 
	}
	
	const_iterator end() const  {
		return m_head;
	}

	
	void remove(node *n){
		if (n->prev != n) {
			n->succ->prev = n->prev; 
			n->prev->succ = n->succ; 
			if (n == m_head) {
				m_head = n->prev; 
			}
			delete n;
		}else { // only head left
			assert(n == m_head);
			delete n; 
			m_head = NULL; 
		}
	}
	void push_back(T val)
	{
		if (m_head) {
			node *nn = new node(val, m_head, m_head->succ);
			nn->prev->succ = nn; 
			nn->succ->prev = nn; 
		}else {
			assert (m_head == NULL);
			m_head = new node(val,NULL,NULL);
			m_head->prev = m_head->succ = m_head;  
		}
	}
	int size() {
		int s = 0; 
		if (m_head) {
			node *n = m_head; 
			while (n->succ != m_head) {
				n = n->succ; 
				++s; 
			}
		}
		return s; 
	}
private:
	node *m_head;
};

NS_MIA_END

#endif

/*
  $Log$
  Revision 1.3  2005/06/29 13:22:20  wollny
  switch to version 0.7

  Revision 1.1.1.1  2005/03/17 13:44:20  gerddie
  initial import 

  Revision 1.2  2004/10/15 14:05:37  wollny
  log entrys

*/
