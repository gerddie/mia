/*
** Copyright Madrid (c) 2010 BIT ETSIT UPM
**                    Gert Wollny <gw.fossdev @ gmail.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
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

#ifndef mia_correlation_weight_hh
#define mia_correlation_weight_hh

#include <vector>
#include <mia/2d/2DImage.hh>

NS_MIA_BEGIN

class CCorrelationEvaluator {
public:
	typedef struct {
		C2DFImage horizontal;
		C2DFImage vertical;
	} result_type;

	CCorrelationEvaluator(double thresh);
	~CCorrelationEvaluator();

	result_type operator() (const std::vector<P2DImage>& images) const;

public:
	struct CCorrelationEvaluatorImpl *impl;
};

NS_MIA_END

#endif
