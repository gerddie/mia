/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/2d/transform.hh>

NS_MIA_BEGIN

struct C2DTransformMock: public C2DTransformation {
	C2DTransformMock();
	C2DTransformMock(const C2DBounds& size);
	virtual C2DTransformation *clone() const;
	virtual bool save(const std::string& filename, const std::string& type) const;
	virtual P2DTransformation upscale(const C2DBounds& size) const;
	virtual void add(const C2DTransformation& a);
	virtual void update(float step, const C2DFVectorfield& a);
	virtual size_t degrees_of_freedom() const;
	virtual void set_identity();
	virtual C2DFMatrix derivative_at(int x, int y) const;
	virtual void translate(const C2DFVectorfield& gradient, gsl::DoubleVector& params) const;
	virtual float get_max_transform() const;
	virtual gsl::DoubleVector  get_parameters() const;
	virtual void set_parameters(const gsl::DoubleVector& params);
	virtual const C2DBounds& get_size() const;
	virtual float pertuberate(C2DFVectorfield& v) const;
	virtual C2DFVector apply(const C2DFVector& x) const;
	virtual C2DFVector operator () (const C2DFVector& x) const;
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const;
	virtual float divergence() const;
	virtual float curl() const;

private:
        virtual P2DImage apply(const C2DImage& image, const C2DInterpolatorFactory& ipf) const;
	C2DBounds m_size;

};

NS_MIA_END
