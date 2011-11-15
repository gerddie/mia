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
	m_weight(w1),
	m_gain(gain_image),
	m_lambda1(l1),
	m_lambda2(l2),
	m_iter(0),
	m_nx(w1.get_size().x),
	m_ny(w1.get_size().y),
	m_count(m_nx*m_ny),
	m_weight_imagePtr(&w1(0, 0)),
	m_fptr(&f1(0, 0)),
	m_gain_image_ptr(&gain_image(0, 0)),

	m_b(m_count),
	m_v(m_count),
	m_r(m_count),

	m_rho(m_count),   // p^(k)
	m_g(m_count),
	m_Ag(m_count),   // speichert A * p

	m_scale(m_count),
	m_scale2(m_count),
	m_border(m_count),

	m_min_res(m_res),
	m_relres(r_res)

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
    ende = m_count;

    // Initialisierung
    cvmsg() << "mult = " << start << ", " << ende << "\n"; 
    multA(m_v, m_r, start, ende);

    // Berechnung normr0;
    double tmp_normr0 = 0;
    for(long i = start; i < ende; i++) {
      m_r[i] += m_b[i];
      double square = m_r[i]/m_scale[i];
      tmp_normr0 += square*square;
    }

    m_normr0 = tmp_normr0;

    if (me == 0)
	    *normr = m_normr0 = sqrt(m_normr0);


    if (*firstnormr0 == 1.0 && m_normr0 > 1.0)
	    *firstnormr0 = m_normr0;


    // Iterationen
    while (*normr >= m_min_res && m_iter < *maxit && *normr / *firstnormr0 > m_relres){

    	if (me == 0) {
	  		m_iter++;
			copy(m_r.begin(), m_r.end(), m_rho.begin()); 
	  		m_r2rho2 = m_r1rho1;
        }


	// r1rho1 = r * rho
	if (me == 0) m_r1rho1 = 0;

	double tmp_r1rho1 = 0;
	for(long i = start; i < ende; i++) {
	  tmp_r1rho1 += m_r[i] * m_rho[i];
	}

	m_r1rho1 = tmp_r1rho1;

	if (m_iter >= 2 && me == 0) m_e = m_r1rho1 / m_r2rho2; else m_e = 0;

	for(long i = start; i < ende; i++) m_g[i] = -m_rho[i] + m_e * m_g[i];

	multA(m_g, m_Ag, start, ende);

	// q = r1rho1 / (g * Ag)
	if (me == 0) m_sprod = 0;
	double tmp_sprod = 0;

	for(long i = start; i < ende; i++) tmp_sprod += m_g[i] * m_Ag[i];

	m_sprod = tmp_sprod;

	if (me == 0) m_q = m_r1rho1 / m_sprod;

	// v = v1 + q * g, r = r1 + q * Ag
	if (me == 0) *normr = 0;
	double tmp_normr = 0;

	for(long i = start; i < ende; i++) {

	  m_v[i] += m_q * m_g[i];
	  m_r[i] += m_q * m_Ag[i];
	  tmp_normr += pow(m_r[i]/m_scale[i], 2);

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
	assert(x.size() == m_border.size()); 
	assert(x.size() == m_scale2.size()); 
	assert(x.size() == m_count); 

	auto bord2 = m_border.begin(); 
	auto res2 = result.begin(); 
	auto x2 = x.begin(); 
	auto sc2 = m_scale2.begin(); 
	vector<double> help(x.size()); 
	
	transform(x.begin(), x.end(), m_scale2.begin(), help.begin(), 
		  [](double x, double s) {return x * s; }); 

	auto value = help.begin(); 
	
	for(int i = start; i < ende; i++, ++bord2, ++res2, ++x2, ++value, ++sc2) {
		if (*bord2) {
			*res2 = *x2;
		}else {
			
			double s1= value[-m_nx      ] + value[-1] + value[+1] + value[+m_nx]; 
			double s2= value[-m_nx   - 1] + value[-m_nx + 1]	+ value[+ m_nx -1]  + value[+m_nx + 1]; 
			double s3= value[ - 2 * m_nx] + value[ - 2     ]  + value[+ 2]        + value[ + 2*m_nx];
			
			*res2 += ((s3 + 2*s2 - 8*s1) * m_lambda2 - m_lambda1 * s1) * *sc2;
		}
	}

}


#define VALUE2(o) ((bord2[o])?0:x2[o])

void C2DSolveCG::multA_float(float *x, float *result) {

	auto bord2 = m_border.begin(); 
	float *x2, *res2;
	
	for(unsigned long i = 0; i < m_count; i++, ++bord2) {
		
		double s1, s2, s3;
		
		res2  = result + i;
		x2    = x + i;
		
		if (*bord2) {
			*res2 = (m_weight_imagePtr[i] + 4*m_lambda1 + 20*m_lambda2) * *x2;
			continue;
		}
		
		s1 = VALUE2(-m_nx)    + VALUE2(-1) + VALUE2(+1) + VALUE2(+m_nx);
		
		s2= VALUE2(-m_nx-1)     + VALUE2(-m_nx+1)	 + VALUE2(+m_nx-1)     + VALUE2(+m_nx+1);
		
		s3= VALUE2( - 2*m_nx) + VALUE2( - 2)	+ VALUE2( + 2) + VALUE2( + 2*m_nx);
		
		*res2 = (m_weight_imagePtr[i] + 4*m_lambda1+8*m_lambda2) * *x2  + ((s3 + 2*s2 - 12*s1) * m_lambda2 - m_lambda1 * s1);
		
	}
}

// loest die PDE  (w + m_lambda1 * H1 + m_lambda2 * H2)*m = f
void C2DSolveCG::init()
{
	TRACE_FUNCTION; 
	
	// set b and scaling
	for(unsigned long i = 0; i < m_count; i++) {
		m_border[i] = (fborder(i, m_nx, m_ny)? 1 : 0);
	}
	
	for(unsigned long i = 0; i < m_count; i++) {
		// Wenn Element Randelement ist, setze b entsprechend
		if (m_border[i]) {
			m_b[i] = -m_gain_image_ptr[i];
			m_scale[i] = 1.0;
			m_scale2[i] = 0;
			m_v[i] = m_gain_image_ptr[i] / m_scale[i];
			continue;
		}
		
		// set b[i]
		m_b[i] = -m_fptr[i];
		
		// ziehe Anteile aus Randelementen ab
		
		// scaling
		m_scale[i] = m_scale2[i] = 1.0 / sqrt(m_weight_imagePtr[i] + 4*m_lambda1 + 20*m_lambda2);
		m_b[i] *= m_scale[i];
		m_v[i] = m_gain_image_ptr[i] / m_scale[i];
	}
}


void C2DSolveCG::get_solution(C2DFImage& m) {

	C2DFImage::iterator m_gain_image_ptr = m.begin();
	for(unsigned long i = 0; i < m_count; i++, ++m_gain_image_ptr)
		*m_gain_image_ptr = (float)m_v[i] * m_scale[i];
}


void C2DSolveCG::add_to_solution(C2DFImage *e) {

  C2DFImage::iterator eptr = e->begin();

  for(unsigned long i = 0; i < m_count; i++)
    m_v[i] += eptr[i] / m_scale[i];
}

NS_MIA_END
