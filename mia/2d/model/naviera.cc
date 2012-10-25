/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

/*
  LatexBeginPluginDescription{2D model based registration}

  \subsection{Navier-Stokes PDE, Gauss-Southwell version}
  \label{model2d:naviera}
  
  \begin{description}
   
  \item [Plugin:] navier
   \item [Description:] This plug-in provides a version of the Navier-Stockes based 
   registration model that allow for linear-elastic or fluid dynamic registration models. 
   \begin{equation}
   \mu \nabla ^{2}\vu(\vx)+(\mu +\lambda )\nabla (\nabla \cdot \vu(\vx)) = -[S(\vx)-R(\vx)]\left. \nabla S\right| _{\vx}\ \
   \end{equation}
   
   The plug-in solves the underlying PDE by running a Gauss-Southwell relaxation. 
   For a version that uses Succesive Overrelaxation use \emph{navier}
   \ref{model2d:navier}.

   \plugtabstart
   mu & float & shear parameter $\mu$  & 1.0 \\
   lambda & float & dilation parameter $\lambda$  & 1.0 \\
   omega & float & relaxation parameter & 1.0 \\
   epsilon & float & stopping parameter & 0.0001 \\
   iter & int & maximum number of iterations & 100 \\
   \plugtabend

   \end{description}

						
  LatexEnd 
*/ 
#include <limits>
#include <mia/2d/model/naviera.hh>

NS_BEGIN(naviera_regmodel)
NS_MIA_USE
using namespace boost;
using namespace std;


C2DNavierRegModel::C2DNavierRegModel(float mu, float lambda, size_t maxiter, float omega, float epsilon):
	m_mu(mu),
	m_lambda(lambda),
	m_omega(omega),
	m_epsilon(epsilon),
	m_max_iter(maxiter)
{
	float a = mu;
	float b = lambda + mu;
	float c = 1 / (4*a+2*b);
	m_b_4 = 0.25 * b * c;
	m_a_b = ( a + b ) * c;
	m_a = a * c;
}

void set_update(C2DUBImage::iterator isetupt, size_t dx)
{

	isetupt[-1-dx] = true;
	isetupt[0-dx] = true;
	isetupt[1-dx] = true;
	isetupt[-1] = true;
	*isetupt = true;
	isetupt[1] = true;
	isetupt[dx-1] = true;
	isetupt[dx] = true;
	isetupt[dx+1] = true;
}

void C2DNavierRegModel::do_solve(const C2DFVectorfield& b, C2DFVectorfield& v) const
{
	// init velocity fields
	v.clear();
	float start_residuum = 0.0;
	float residuum;
        const size_t end_x = b.get_size().x-1;
	const size_t dx = b.get_size().x;
	size_t i = 0;

        C2DFImage residua(b.get_size());
	C2DUBImage needupdate1(b.get_size());
	C2DUBImage needupdate2(b.get_size());

	C2DUBImage *needupdate_get = &needupdate1;
	C2DUBImage *needupdate_set = &needupdate2;

	fill(needupdate_get->begin(), needupdate_get->end(), true);

	fill(residua.begin(), residua.end(), 0.0f);
	{
		C2DFVectorfield::const_iterator ib = b.begin() + end_x+1;
		C2DFVectorfield::iterator iv = v.begin() + dx;
		C2DFImage::iterator ires = residua.begin() + dx;

		for (size_t y = 1; y < b.get_size().y-1; y++) {
			++ib;
			++ires;
			for (size_t x = 1; x < end_x; x++, ++ib, ++iv, ++ires) {
				*ires = solve_at(x, y, *ib, iv, dx);
				start_residuum += *ires;
			}
			++ib;
			++ires;
		}

	}

	residuum = start_residuum;


	do {
		fill(needupdate_set->begin(), needupdate_set->end(), false);
		float rthresh = residuum / (b.size() + i);

		++i;
		residuum = 0;
		C2DFVectorfield::iterator iv = v.begin() + dx;

		C2DFVectorfield::const_iterator ib = b.begin() + dx;
		C2DFImage::iterator ires = residua.begin() + dx;
		C2DUBImage::const_iterator igetupt = needupdate_get->begin()  + dx;
		C2DUBImage::iterator isetupt = needupdate_set->begin()  + dx;
// don't use get_size (or inline it)
// don't use a bit image for the update, since it needs shifts
		for (size_t y = 1; y < b.get_size().y-1; y++) {
			++ib;
			++ires;
			++igetupt;
			++isetupt;
			++iv;

			for (size_t x = 1; x < end_x; x++, ++iv, ++ib, ++ires, ++igetupt, ++isetupt){
				if (*igetupt)
					*ires = solve_at(x, y, *ib, iv, dx);

				if (*ires > rthresh)
					set_update(isetupt, dx);

				residuum += *ires;
			}
			++ib;
			++ires;
			++igetupt;
			++isetupt;
			++iv;
		}


		cvdebug() << i << ":" << residuum << "\r";
		if (residuum < 1)
			break;

		swap(needupdate_get, needupdate_set);


	} while (i < m_max_iter && residuum / start_residuum > m_epsilon);
	cverb << "\n";
}

float  C2DNavierRegModel::solve_at(unsigned int /*x*/, unsigned int /*y*/,
				   const C2DFVector& b, C2DFVectorfield::iterator vp, int dx)const
{

	C2DFVectorfield::iterator vpp = vp + dx;
	C2DFVectorfield::iterator vpm = vp - dx;

	C2DFVector p(b.x + m_a_b * ( vp[-1].x + vp[+1].x ) + m_a * ( vpp->x   + vpm->x  ),
		     b.y + m_a_b * ( vpm->y   + vpp->y   ) + m_a * ( vp[-1].y + vp[1].y ));


	C2DFVector q;
	q.y = ( ( vpm[-1].x + vpp[1].x ) - ( vpm[1].x + vpp[-1].x ) ) * m_b_4;
	q.x = ( ( vpm[-1].y + vpp[1].y ) - ( vpm[1].y + vpp[-1].y ) ) * m_b_4;

	C2DFVector hmm((( p + q ) - *vp) * m_omega);

	*vp += hmm;
	return hmm.norm();
}

C2DNavierRegModelPlugin::C2DNavierRegModelPlugin():
	C2DRegModelPlugin("naviera"),
	m_mu(1.0),
	m_lambda(1.0),
	m_omega(1.0),
	m_epsilon(0.0001),
	m_maxiter(100)
{
	typedef CParamList::PParameter PParameter;
	add_parameter("mu", new CFloatParameter(m_mu, 0.0, numeric_limits<float>::max(),
							   false, "isotropic compliance"));
	add_parameter("lambda", new CFloatParameter(m_lambda, 0.0, numeric_limits<float>::max(),
							       false, "isotropic compression"));
	add_parameter("omega", new CFloatParameter(m_omega, 0.1, 10,
							      false, "relexation parameter"));
	add_parameter("epsilon", new CFloatParameter(m_epsilon, 0.000001, 0.1,
								false, "stopping parameter"));
	add_parameter("iter", new CIntParameter(m_maxiter, 10, 10000,
							   false, "maximum number of iterations"));
}

C2DRegModel *C2DNavierRegModelPlugin::do_create()const
{
	return new C2DNavierRegModel(m_mu, m_lambda, m_maxiter, m_omega, m_epsilon);
}

const string C2DNavierRegModelPlugin::do_get_descr()const
{
	return "navier-stokes based registration model using a Gauss-Southwell relaxation as solver";
}

extern "C"  EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DNavierRegModelPlugin();
}

NS_END

