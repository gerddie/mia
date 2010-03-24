/*
** Copyright (C) 1999 Max-Planck-Institute of Cognitive Neurosience
**                    Gert Wollny <wollny@cns.mpg.de>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  As an exception to this license, "NEC C&C Research Labs" may use
  this software under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation.
   

*/

#ifndef __sor_solver_h
#define __sor_solver_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mia/core/parallel.hh>

#include "eqn_solver.hh"


class TSORSolver: public TFluidHomogenSolver {
protected:
	mia::C3DBounds size; 
	int d_xy; 
public:	
	TSORSolver(int _max_steps, float _rel_res, float _abs_res,
		   float mu, float lambda);
	virtual int solve(const mia::C3DFVectorfield& right_side,mia::C3DFVectorfield *solution);
protected: 
	float solve_at_very_old(const mia::C3DFVectorfield& B,mia::C3DFVectorfield *V,size_t  x, size_t  y, size_t  z);
	float solve_at_old(mia::C3DFVector *Data,const mia::C3DFVector& bv);
	float solve_at(mia::C3DFVector *Data,const mia::C3DFVector& b);
};



class TSORASolver: public TSORSolver {
public:	
	TSORASolver(int _max_steps, float _rel_res, float _abs_res,
		    float mu, float lambda);
	
	virtual int solve(const mia::C3DFVectorfield& b,mia::C3DFVectorfield *x);
protected: 
	class  TUpdateInfo : public mia::C3DUBDatafield {
	public:
		TUpdateInfo(const mia::C3DBounds& size): mia::C3DUBDatafield(size){
		};
		void set_update(int hardcode) {
			const unsigned char val = 1; 
			unsigned char *ptr = &(*this)[hardcode];
			unsigned char *p2 = ptr - get_plane_size_xy();
			const int size_x = get_size().x; 
			
			p2[-size_x] = val;
			p2[-1] = val; 
			p2[ 0] = val; 
			p2[ 1] = val; 
			p2[size_x] = val;
			
			p2 = ptr - get_size().x;

			p2[-1] = val; 
			p2[ 0] = val; 
			p2[ 1] = val; 
			
			ptr[-1] = val;
			ptr[+1] = val;
			
			p2 = ptr + get_size().x;
			p2[-1] = val; 
			p2[ 0] = val; 
			p2[ 1] = val; 

			p2 = ptr + get_plane_size_xy();
			p2[-size_x] = val;
			p2[-1] = val; 
			p2[0  ] = val; 
			p2[1] = val; 
			p2[size_x] = val;
		}
	};
};


class TSORAParallelSolver: public TSORASolver, private boost::noncopyable{
	int max_threads;
	
	boost::mutex global_z_mutex; 
	int  global_z; 
	boost::thread_group threads; 
	boost::barrier iter_barrier;
	boost::barrier after_barrier;
	
	
	bool i_do_it; 
	size_t  block_size; 
	TUpdateInfo  *update_needed;
	TUpdateInfo  *need_update;
	mia::C3DFDatafield *residua;
	float  global_res; 

	int threads_ready; 
	float  doorstep,lastres,firstres; 
	bool done; 
	int gSize; 
	
	const mia::C3DFVectorfield *pb; 
	mia::C3DFVectorfield *px; 
	
public:
    	TSORAParallelSolver(int _max_steps, float _rel_res, float _abs_res,
		    float mu, float lambda,int _max_threads);
	~TSORAParallelSolver();
	virtual int solve(const mia::C3DFVectorfield& b,mia::C3DFVectorfield *x);
	void operator () (); 
private:
	void solve_p(const mia::C3DFVectorfield& b,mia::C3DFVectorfield *x);

};

#endif

/* Changes to this file 
   
  $Log$
  Revision 1.5  2005/06/29 13:43:35  wollny
  cg removed and libmona-0.7

  Revision 1.1.1.1  2005/06/17 10:31:09  gerddie
  initial import at sourceforge

  Revision 1.4  2005/02/22 10:06:25  wollny
  enable parallel processing

  Revision 1.3  2005/02/22 09:49:16  wollny
  removed vistaio dependecy

  Revision 1.1.1.1  2005/02/21 15:00:37  wollny
  initial import 

  Revision 1.14  2004/04/05 15:24:33  gerddie
  change filter allocation

  Revision 1.13  2003/08/27 10:14:36  gerddie
  adapt to new debian install location and libtool versioning

  Revision 1.12  2002/08/01 20:38:54  gerddie
  added new mutex and condition type

  Revision 1.11  2002/06/20 09:59:49  gerddie
  added cvs-log entry
 
  
*/

