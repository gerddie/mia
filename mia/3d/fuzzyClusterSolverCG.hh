/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
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

// $Id: fuzzyClusterSolverCG.hh 672 2005-06-29 13:32:02Z wollny $

/*! \brief solverCG -- a class providing a CG solver

	This contains basic solver functions based on CG schemes

\file solverCG.hh

\author Stefan Burckhardt and Carsten Wolters, wolters@mis.mpg.de, 2004
\remark adapted for libmona by Heike Jaenicke and Marc Tittgemeyer, tittge@cbs.mpg.de, 2004
*/


#ifndef __SOLVERCG_HH
#define __SOLVERCG_HH

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mia/3d.hh>
#include <cstdio>
#include <stdexcept>
#include <string>



NS_MIA_BEGIN

using namespace std;



/*! \brief function defining field borders

    \param index  index running from 0 to NoOfPixels
    \param nx     no of pixels in x
    \param ny     no of pixels in y
    \param nz     no of pixels in z

*/
extern bool fborder (long index, long nx, long ny, long nz);

/** A class to solve sCG */

class solve_sCG {

  private:

	C3DFImage& __gain;

	double __lambda1;
	double __lambda2;

	// Dimension of images
	long __iter;
	unsigned int  __nx, __ny, __nz;
	unsigned long __count;

	// Pointer to Elements of w
	float *__weight_imagePtr;
	float *__fptr;
	float *__gain_image_ptr;



	// b and x for solution of system
	double *__b;
	double *__v;

	// counts iterations


	// help pointers for one iteration cycle
	double *__r;	   // r^(k)
	double *__rho;     // p^(k)
	double *__g;
	double *__Ag;	   // speichert A * p
	// Field of scaling factors
	double *__scale;
	double *__scale2;

	// field for border voxels
	bool *__border;


	double __r1rho1;   // speichert r1 * rho1
	double __r2rho2;   // speichert r2 * rho2
	double __normr, __normr0;
	double __q, __e, __sprod;

	// minimal residuum
	double __min_res, __relres;

	/** function for initialising
         */
        void init();

  public:
	/** constructor
	    \param w1
	    \param f1
	    \param g1
	    \param l1
	    \param l2
	    \param r_res
	    \param m_res
	 */
	solve_sCG (C3DFImage& w1, C3DFImage&  f1, C3DFImage& g1, double l1, double l2, double r_res, double m_res);

	~solve_sCG();

	/** Function to solve ...
	    \param max_iterations Maximum number of iterations
	    \param firstnormr0
	    \returns
	 */
	int solve(long max_iterations, double *firstnormr0);

	/** Function to get preset number of iterations
	    \returns Number of iterations
	 */
	inline long get_iterations() {return __iter;}

	/** Multiplication of vector and matrix

	TODO mit standard classe Austauschen

	    \param x
	    \param result
	    \param start
	    \param ende
	 */
	void multA(double *x, double *result, long start, long ende);

	/** Multiplication

	TODO mit standard classe Austauschen

	    \param x Pointer at
	    \param result Pointer at
	 */
	void multA_float(float *x, float *result);

	/**
	    \param gain Image with gain-field
	 */
	void get_solution(C3DFImage& gain);

	/**
	    \param e
	 */
	void add_to_solution(C3DFImage *e);

	// function for parallel solver
	void solvepar(long *max_iteration, double *normr, double *firstnormr0);

};

NS_MIA_END

#endif
