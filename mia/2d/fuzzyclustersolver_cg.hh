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

#ifndef __SOLVERCG_HH
#define __SOLVERCG_HH

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mia/2d.hh>
#include <cstdio>
#include <stdexcept>
#include <string>



NS_MIA_BEGIN

using namespace std;



/*! \brief function defining field borders

    \param index  index running from 0 to NoOfPixels
    \param nx     no of pixels in x
    \param ny     no of pixels in y

*/
extern bool fborder (long index, long nx, long ny);

/*! 
  @ingroup filtering
  \brief solve_sCG -- a class providing a CG solver

	This contains basic solver functions based on CG schemes

\author Stefan Burckhardt and Carsten Wolters, wolters@mis.mpg.de, 2004
\remark adapted for libmona by Heike Jaenicke and Marc Tittgemeyer, tittge@cbs.mpg.de, 2004
\remark adapted for mia2 by Gert Wollny, gw.fossdev@gmail.com 2011 
*/


class C2DSolveCG {

  private:
	double m_lambda1;
	double m_lambda2;

	// Dimension of images
	long m_iter;
	int  m_nx, m_ny;
	unsigned long m_count;

	// Pointer to Elements of w
	float *m_weight_imagePtr;
	float *m_fptr;
	float *m_gain_image_ptr;



	// b and x for solution of system
	std::vector<double> m_b;
	std::vector<double> m_v;

	// counts iterations


	// help pointers for one iteration cycle
	std::vector<double> m_r;	   // r^(k)
	std::vector<double> m_rho;     // p^(k)
	std::vector<double> m_g;
	std::vector<double> m_Ag;	   // speichert A * p
	// Field of scaling factors
	std::vector<double> m_scale;
	std::vector<double> m_scale2;

	// field for border voxels
	std::vector<bool> m_border;


	double m_r1rho1;   // speichert r1 * rho1
	double m_r2rho2;   // speichert r2 * rho2
	double m_normr0;
	double m_q, m_e, m_sprod;

	// minimal residuum
	double m_min_res, m_relres;

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
	C2DSolveCG (C2DFImage& w1, C2DFImage&  f1, C2DFImage& g1, double l1, double l2, double r_res, double m_res);

	~C2DSolveCG();

	/** Function to solve ...
	    \param max_iterations Maximum number of iterations
	    \param firstnormr0
	    \returns
	 */
	int solve(long max_iterations, double *firstnormr0);

	/** Function to get preset number of iterations
	    \returns Number of iterations
	 */
	inline long get_iterations() {return m_iter;}

	/** Multiplication of vector and matrix

	TODO mit standard classe Austauschen

	    \param x
	    \param result
	    \param start
	    \param ende
	 */
	void multA(std::vector<double>& x, std::vector<double>& result, long start, long ende);

	/** Multiplication

	TODO mit standard classe Austauschen

	    \param x Pointer at
	    \param result Pointer at
	 */
	void multA_float(float *x, float *result);

	/**
	    \param gain Image with gain-field
	 */
	void get_solution(C2DFImage& gain);

	/**
	    \param e
	 */
	void add_to_solution(C2DFImage *e);

	/** function for parallel solver
	    @param max_iteration
	    @param normr
	    @param firstnormr0
	 */
	void solvepar(long *max_iteration, double *normr, double *firstnormr0);

};

NS_MIA_END

#endif
