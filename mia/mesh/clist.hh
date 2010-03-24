/* -*- mona-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004 - 2009
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

	
	clist(): _M_head(NULL){
		
	}
	~clist() {
		if (_M_head != NULL) {
			node *head = _M_head;
			while (head != head->succ) 
				remove(head->succ);
			delete _M_head; 
		}
	}
	
	iterator begin() {
		return _M_head; 
	}
	
	iterator end() {
		return _M_head;
	}

	const_iterator begin() const {
		return _M_head; 
	}
	
	const_iterator end() const  {
		return _M_head;
	}

	
	void remove(node *n){
		if (n->prev != n) {
			n->succ->prev = n->prev; 
			n->prev->succ = n->succ; 
			if (n == _M_head) {
				_M_head = n->prev; 
			}
			delete n;
		}else { // only head left
			assert(n == _M_head);
			delete n; 
			_M_head = NULL; 
		}
	}
	void push_back(T val)
	{
		if (_M_head) {
			node *nn = new node(val, _M_head, _M_head->succ);
			nn->prev->succ = nn; 
			nn->succ->prev = nn; 
		}else {
			assert (_M_head == NULL);
			_M_head = new node(val,NULL,NULL);
			_M_head->prev = _M_head->succ = _M_head;  
		}
	}
	int size() {
		int s = 0; 
		if (_M_head) {
			node *n = _M_head; 
			while (n->succ != _M_head) {
				n = n->succ; 
				++s; 
			}
		}
		return s; 
	}
private:
	node *_M_head;
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
