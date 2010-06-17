
/*
** Copyright Madrid (c) 2010 BIT ETSIT UPM
**                    Gert Wollny <gw.fossdev @ gmail.com>
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
*/

#include <mia/2d/ground_truth_evaluator.hh>

NS_MIA_BEGIN

struct C2DGroundTruthEvaluatorImpl {
	C2DGroundTruthEvaluatorImpl(double alpha, double beta, double rho); 
	std::vector<P2DImage> run(const std::vector<P2DImage>& originals) const; 
private: 
	double m_alpha;
	double m_beta; 
	double m_rho;
};

C2DGroundTruthEvaluator::C2DGroundTruthEvaluator(double alpha, double beta, double rho):
	impl(new C2DGroundTruthEvaluatorImpl(alpha, beta, rho))
{
}


C2DGroundTruthEvaluator::~C2DGroundTruthEvaluator()
{
	delete impl; 
}

std::vector<P2DImage> C2DGroundTruthEvaluator::operator () (const std::vector<P2DImage>& originals) const
{
	return impl->run(originals); 
}

C2DGroundTruthEvaluatorImpl::C2DGroundTruthEvaluatorImpl(double alpha, double beta, double rho):
	m_alpha(alpha), 
	m_beta(beta), 
	m_rho(rho)

{
}

std::vector<P2DImage> C2DGroundTruthEvaluatorImpl::run(const std::vector<P2DImage>& originals) const
{
	return originals; 
}

NS_MIA_END
