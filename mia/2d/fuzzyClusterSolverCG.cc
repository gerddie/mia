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

#include <cstring>

#include <mia/core/msgstream.hh>
#include <mia/2d/fuzzyClusterSolverCG.hh>


NS_MIA_BEGIN
#warning this needs to be tested
bool fborder (long index, long nx, long ny)
{
	const long x = index % nx;

	if ( x < 2 || x > nx-3 )
	     return true;

	index /= nx;

	const long y = index % ny;
	if ( y < 2 || y > ny-3 )
	     return true;

	return false;

}



C2DSolveCG::C2DSolveCG (C2DFImage& w1, C2DFImage& f1, C2DFImage& gain_image, double l1, double l2, double r_res, double m_res):
	__gain(gain_image),
	__lambda1(l1),
	__lambda2(l2),
	__iter(0),
	__nx(w1.get_size().x),
	__ny(w1.get_size().y),
	__count(__nx*__ny),
	__weight_imagePtr(&w1(0, 0)),
	__fptr(&f1(0, 0)),
	__gain_image_ptr(&gain_image(0, 0)),

	__b(__count),
	__v(__count),
	__r(__count),

	__rho(__count),   // p^(k)
	__g(__count),
	__Ag(__count),   // speichert A * p

	__scale(__count),
	__scale2(__count),
	__border(__count),

	__min_res(m_res),
	__relres(r_res)

{
	TRACE_FUNCTION; 
	init ();

}


C2DSolveCG::~C2DSolveCG()
{
}



// Umweg, da keine Methoden parallel aufgerufen werden koennen
void solver(C2DSolveCG *s, long *maxit, double *normr, double *firstnormr0)
{
    s->solvepar(maxit, normr, firstnormr0);
}


int C2DSolveCG::solve(long max_iterations, double *firstnormr0) {

        double normr;

#ifdef PARALLEL
    	m_set_procs(NPROCS);
    	m_fork((void (*)())solver, this, &max_iterations, &normr,firstnormr0);
    	m_kill_procs();
#else
    	solver(this, &max_iterations, &normr,firstnormr0);
#endif

	return normr > 1;
}


// loest das Gleichungssystem mittels CG-Algorithmus
void C2DSolveCG::solvepar(long *maxit, double *normr, double *firstnormr0) 
{
	TRACE_FUNCTION; 
    // Prozessornummer
    int me;

    // Start und Ende fuer Vektoroperationen
    long start;
    long ende;

    // Setze me, start und ende
    me = 0;
    start = 0;
    ende = __count;

    // Initialisierung
    cvmsg() << "mult = " << start << ", " << ende << "\n"; 
    multA(__v, __r, start, ende);

    // Berechnung normr0;
    double tmp_normr0 = 0;
    for(long i = start; i < ende; i++) {
      __r[i] += __b[i];
      double square = __r[i]/__scale[i];
      tmp_normr0 += square*square;
    }

    __normr0 = tmp_normr0;

    if (me == 0)
	    *normr = __normr0 = sqrt(__normr0);


    if (*firstnormr0 == 1.0 && __normr0 > 1.0)
	    *firstnormr0 = __normr0;


    // Iterationen
    while (*normr >= __min_res && __iter < *maxit && *normr / *firstnormr0 > __relres){

    	if (me == 0) {
	  		__iter++;
			copy(__r.begin(), __r.end(), __rho.begin()); 
	  		__r2rho2 = __r1rho1;
        }


	// r1rho1 = r * rho
	if (me == 0) __r1rho1 = 0;

	double tmp_r1rho1 = 0;
	for(long i = start; i < ende; i++) {
	  tmp_r1rho1 += __r[i] * __rho[i];
	}

	__r1rho1 = tmp_r1rho1;

	if (__iter >= 2 && me == 0) __e = __r1rho1 / __r2rho2; else __e = 0;

	for(long i = start; i < ende; i++) __g[i] = -__rho[i] + __e * __g[i];

	multA(__g, __Ag, start, ende);

	// q = r1rho1 / (g * Ag)
	if (me == 0) __sprod = 0;
	double tmp_sprod = 0;

	for(long i = start; i < ende; i++) tmp_sprod += __g[i] * __Ag[i];

	__sprod = tmp_sprod;

	if (me == 0) __q = __r1rho1 / __sprod;

	// v = v1 + q * g, r = r1 + q * Ag
	if (me == 0) *normr = 0;
	double tmp_normr = 0;

	for(long i = start; i < ende; i++) {

	  __v[i] += __q * __g[i];
	  __r[i] += __q * __Ag[i];
	  tmp_normr += pow(__r[i]/__scale[i], 2);

	}

	*normr = tmp_normr;

	if (me == 0)
		*normr = sqrt(*normr);

	//*\todo implement some progression alert here */

    }

    return;
 }


// Vektor-Matrix-Multiplikation
#define VALUE(o) (sc2[o] * x2[o])

void C2DSolveCG::multA(vector<double>& x, vector<double>& result, long start, long ende) 
{
	assert(x.size() == result.size()); 
	assert(x.size() == __border.size()); 
	assert(x.size() == __scale2.size()); 
	assert(x.size() == __count); 

	auto bord2 = __border.begin(); 
	auto res2 = result.begin(); 
	auto x2 = x.begin(); 
	auto sc2 = __scale2.begin(); 
	vector<double> help(x.size()); 
	
	transform(x.begin(), x.end(), __scale2.begin(), help.begin(), 
		  [](double x, double s) {return x * s; }); 

	auto value = help.begin(); 
	
	for(int i = start; i < ende; i++, ++bord2, ++res2, ++x2, ++value, ++sc2) {
		
		if (*bord2) {
			*res2 = *x2;
			continue;
		}

		double s1= value[-__nx    ] + value[-1] + value[+1] + value[+__nx]; 
		double s2= value[-__nx  -1] + value[-__nx + 1]	+ value[+ __nx -1]  + value[+__nx + 1]; 
		double s3= value[ - 2*__nx] + value[ - 2     ]  + value[+ 2]        + value[ + 2*__nx];

		*res2 = (*x2 + ((s3 + 2*s2 - 12*s1) * __lambda2 - __lambda1 * s1) * *sc2);
	}

}


#define VALUE2(o) ((bord2[o])?0:x2[o])

void C2DSolveCG::multA_float(float *x, float *result) {

	auto bord2 = __border.begin(); 
	float *x2, *res2;
	
	for(unsigned long i = 0; i < __count; i++, ++bord2) {
		
		double s1, s2, s3;
		
		res2  = result + i;
		x2    = x + i;
		
		if (*bord2) {
			*res2 = (__weight_imagePtr[i] + 6*__lambda1 + 42*__lambda2) * *x2;
			continue;
		}
		
		s1 = VALUE2(-__nx)    + VALUE2(-1) + VALUE2(+1) + VALUE2(+__nx);
		
		s2= VALUE2(-__nx-1)     + VALUE2(-__nx+1)	 + VALUE2(+__nx-1)     + VALUE2(+__nx+1);
		
		s3= VALUE2( - 2*__nx) + VALUE2( - 2)	+ VALUE2( + 2) + VALUE2( + 2*__nx);
		
		*res2 = (__weight_imagePtr[i] + 6*__lambda1+42*__lambda2) * *x2  + ((s3 + 2*s2 - 12*s1) * __lambda2 - __lambda1 * s1);
		
	}
}

// loest die PDE  (w + __lambda1 * H1 + __lambda2 * H2)*m = f
void C2DSolveCG::init()
{
	TRACE_FUNCTION; 
	
	// set b and scaling
	for(unsigned long i = 0; i < __count; i++) {
		__border[i] = (fborder(i, __nx, __ny)? 1 : 0);
	}
	
	for(unsigned long i = 0; i < __count; i++) {
		// Wenn Element Randelement ist, setze b entsprechend
		if (__border[i]) {
			__b[i] = -__gain_image_ptr[i];
			__scale[i] = 1.0;
			__scale2[i] = 0;
			__v[i] = __gain_image_ptr[i] / __scale[i];
			continue;
		}
		
		// set b[i]
		__b[i] = -__fptr[i];
		
		// ziehe Anteile aus Randelementen ab
		
		// scaling
		__scale[i] = __scale2[i] = 1.0 / sqrt(__weight_imagePtr[i] + 6*__lambda1 + 42*__lambda2);
		__b[i] *= __scale[i];
		__v[i] = __gain_image_ptr[i] / __scale[i];
	}
}


void C2DSolveCG::get_solution(C2DFImage& m) {

	C2DFImage::iterator __gain_image_ptr = m.begin();
	for(unsigned long i = 0; i < __count; i++, ++__gain_image_ptr)
		*__gain_image_ptr = (float)__v[i] * __scale[i];
}


void C2DSolveCG::add_to_solution(C2DFImage *e) {

  C2DFImage::iterator eptr = e->begin();

  for(unsigned long i = 0; i < __count; i++)
    __v[i] += eptr[i] / __scale[i];
}

NS_MIA_END
