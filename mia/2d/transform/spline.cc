/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


/* 
  LatexBeginPluginDescription{2D Transformations}
   
   \subsection{B-spline based transformation}
   \label{transform2d:spline}
   
   \begin{description}
   
   \item [Plugin:] spline
   \item [Description:] This plug-in implements a free-form transfromation that can be described by 
   a set of B-spline coefficients and an underlying B-spline kernel. 
   \item [Degrees of Freedom:] depends on the coefficient rate and the size of the domain where the 
      transformation is defined on 
  
   \end{description}

   \plugtabstart
   interp & string & interpolating B-spline kernel (bsplineX, omoms3) with X the degree (2-5)& bspline3 \\ 
   rate   & float  & coefficient rate (currently the same in all dimenions & 10 \\
   \plugtabend

   LatexEnd  
 */


#include <cstdlib>
#include <cassert>
#include <limits>
#include <iomanip>
#include <numeric>
#include <mia/2d/transform/spline.hh>
#include <mia/2d/transformfactory.hh>


NS_MIA_BEGIN
using namespace std;


C2DSplineTransformation::C2DSplineTransformation(const C2DBounds& range, PSplineKernel kernel):
	m_range(range),
	m_target_c_rate(1,1),
	m_kernel(kernel),
	m_shift(0), 
	m_enlarge(0), 
	m_scale(1.0, 1.0),
	m_interpolator_valid(false),
	m_x_weights(m_range.x), 
	m_x_indices(m_range.x), 
	m_y_weights(m_range.y),
	m_y_indices(m_range.y), 
	m_grid_valid(false), 
	m_xbc(produce_spline_boundary_condition("zero")), 
	m_ybc(produce_spline_boundary_condition("zero"))
{
	
	m_shift = m_kernel->get_active_halfrange() - 1; 
	m_enlarge = 2*m_shift; 
	
	TRACE_FUNCTION;
	assert(m_range.x > 0);
	assert(m_range.y > 0);
}

C2DSplineTransformation::C2DSplineTransformation(const C2DSplineTransformation& org):
   	m_range(org.m_range),
	m_target_c_rate(org.m_target_c_rate),
	m_coefficients( org.m_coefficients),
	m_kernel(org.m_kernel),
	m_shift(org.m_shift),
	m_enlarge(org.m_enlarge),
	m_interpolator_valid(false),
	m_x_weights(m_range.x), 
	m_x_indices(m_range.x), 
	m_y_weights(m_range.y),
	m_y_indices(m_range.y),
	m_grid_valid(false), 
	m_xbc(produce_spline_boundary_condition("zero")), 
	m_ybc(produce_spline_boundary_condition("zero"))
{
}

C2DSplineTransformation::C2DSplineTransformation(const C2DBounds& range, PSplineKernel kernel, 
						 const C2DFVector& c_rate):
	m_range(range),
	m_target_c_rate(c_rate),
	m_kernel(kernel),
	m_interpolator_valid(false),
	m_x_weights(m_range.x), 
	m_x_indices(m_range.x), 
	m_y_weights(m_range.y),
	m_y_indices(m_range.y),
	m_grid_valid(false), 
	m_xbc(produce_spline_boundary_condition("zero")), 
	m_ybc(produce_spline_boundary_condition("zero"))
{
	TRACE_FUNCTION;
	assert(m_range.x > 0);
	assert(m_range.y > 0);
	assert(c_rate.x >= 1.0);
	assert(c_rate.y >= 1.0);
	assert(kernel); 

	
	m_shift = m_kernel->get_active_halfrange() - 1; 
	m_enlarge = 2*m_shift; 

	C2DBounds csize(size_t((range.x + c_rate.x - 1) / c_rate.x) + m_enlarge,
			size_t((range.y + c_rate.y - 1) / c_rate.y) + m_enlarge);
	m_coefficients = C2DFVectorfield(csize);
	m_xbc->set_width(m_coefficients.get_size().x); 
	m_ybc->set_width(m_coefficients.get_size().y); 

	reinit();
}

void C2DSplineTransformation::set_coefficients_and_prefilter(const C2DFVectorfield& field)
{
	vector<C2DFVector> buffer(field.get_size().y); 
	C2DFVectorfield help1(field.get_size());
	m_xbc->set_width(field.get_size().x); 
	m_ybc->set_width(field.get_size().y); 

	for(size_t x = 0; x < field.get_size().x; ++x) {
		field.get_data_line_y(x, buffer); 
		m_xbc->filter_line(buffer, m_kernel->get_poles()); 
		help1.put_data_line_y(x, buffer); 
	}
	buffer.resize(field.get_size().x); 
	for(size_t y = 0; y < field.get_size().y; ++y) {
		help1.get_data_line_x(y, buffer); 
		m_ybc->filter_line(buffer, m_kernel->get_poles()); 
		help1.put_data_line_x(y, buffer); 
	}
	set_coefficients(help1); 
}

void C2DSplineTransformation::set_coefficients(const C2DFVectorfield& field)
{
	TRACE_FUNCTION;
	m_interpolator_valid &= (m_coefficients.get_size() != field.get_size());
	m_coefficients = field;

	m_xbc->set_width(field.get_size().x); 
	m_ybc->set_width(field.get_size().y); 

/*
	m_target_c_rate.x = float(m_range.x) / (field.get_size().x - m_enlarge);
	m_target_c_rate.y = float(m_range.y) / (field.get_size().y - m_enlarge);
*/
}

void C2DSplineTransformation::reinit() const
{
	TRACE_FUNCTION;
	if (!m_interpolator_valid) {

		cvdebug() << "C2DSplineTransformation::reinit applies\n";
		m_scale.x = float(m_coefficients.get_size().x - 1 - m_enlarge) / (m_range.x - 1);
		m_scale.y = float(m_coefficients.get_size().y - 1 - m_enlarge) / (m_range.y - 1);
		m_inv_scale.x = float(m_range.x - 1) / (m_coefficients.get_size().x - 1 - m_enlarge);
		m_inv_scale.y = float(m_range.y - 1) / (m_coefficients.get_size().y - 1 - m_enlarge);

		m_mx = get_derivative_row(m_range.x, m_coefficients.get_size().x, m_scale.x); 
		m_my = get_derivative_row(m_range.y, m_coefficients.get_size().y, m_scale.y); 
		m_interpolator_valid = true;
	}
}

C2DFVector C2DSplineTransformation::interpolate(const C2DFVector& x) const 
{
	std::vector<double> xweights(m_kernel->size()); 
	std::vector<double> yweights(m_kernel->size()); 
	size_t startx = m_kernel->get_start_idx_and_value_weights(x.x, xweights); 
	size_t y = m_kernel->get_start_idx_and_value_weights(x.y, yweights); 

	C2DFVector result; 
	for(auto wy = yweights.begin(); y < m_coefficients.get_size().y && wy != yweights.end()
		    ; ++y, ++wy)  {
		C2DFVector h; 
		size_t x = startx; 
		auto wx = xweights.begin();
		for(auto cx = m_coefficients.begin_at(startx,y); 
		    x < m_coefficients.get_size().x && wx != xweights.end(); ++x, ++wx, ++cx)  {
			h += *wx * *cx; 
		}
		result += h * *wy; 
	}
	return result; 
}

C2DFVector C2DSplineTransformation::apply(const C2DFVector& x) const
{
	assert(m_interpolator_valid);
	return interpolate(scale(x));
}

C2DTransformation *C2DSplineTransformation::do_clone()const
{
	return new C2DSplineTransformation(*this);
}

C2DTransformation *C2DSplineTransformation::invert() const
{
	assert(0 && "not implemented"); 
	return new C2DSplineTransformation(*this);
}


C2DFVector C2DSplineTransformation::operator () (const C2DFVector& x) const
{
	return x - apply(x);
}

C2DFVector C2DSplineTransformation::scale( const C2DFVector& x) const
{
	assert(m_interpolator_valid);
	return x * m_scale + C2DFVector(m_shift,m_shift);
}

const C2DBounds& C2DSplineTransformation::get_size() const
{
	return m_range;
}

CDoubleVector C2DSplineTransformation::get_parameters() const
{
	TRACE_FUNCTION;
	CDoubleVector result(m_coefficients.size() * 2);
	auto r = result.begin();
	for(auto f = m_coefficients.begin(); f != m_coefficients.end(); ++f) {
		*r++ = f->x;
		*r++ = f->y;
	}
	return result;
}

void C2DSplineTransformation::set_parameters(const CDoubleVector& params)
{
	TRACE_FUNCTION;
	assert(2 * m_coefficients.size() == params.size());
	auto r = params.begin();
	for(auto f = m_coefficients.begin(); f != m_coefficients.end(); ++f) {
		f->x = *r++;
		f->y = *r++;
	}
	m_interpolator_valid = false; 
}

bool C2DSplineTransformation::save(const std::string& /*filename*/) const
{
	TRACE_FUNCTION;
	assert(!"not implemented");
	return false;
}


bool C2DSplineTransformation::refine()
{
	TRACE_FUNCTION;
	C2DBounds csize(size_t((m_range.x + m_target_c_rate.x - 1) / m_target_c_rate.x) + m_enlarge,
			size_t((m_range.y + m_target_c_rate.y - 1) / m_target_c_rate.y) + m_enlarge);
	
	// no refinement necessary? 
	if (csize.x <= m_coefficients.get_size().x && 
	    csize.y <= m_coefficients.get_size().y) 
		return false; 

	// now interpolate the new coefficients 
	// \todo this should be done faster by a filter 
	reinit();
	T2DConvoluteInterpolator<C2DFVector> interp(m_coefficients, m_kernel);

	C2DFVectorfield coeffs(csize);
	C2DFVector dx((float)(m_coefficients.get_size().x - 1 - m_enlarge) / (float)(csize.x - 1 - m_enlarge),
		      (float)(m_coefficients.get_size().y - 1 - m_enlarge) / (float)(csize.y - 1 - m_enlarge));

	C2DFVectorfield::iterator ic = coeffs.begin();

	cvdebug() << "Refine from " << m_coefficients.get_size() << " to " << csize << "\n"; 

	for (size_t y = 0; y < csize.y; ++y)
		for (size_t x = 0; x < csize.x; ++x, ++ic) {
			C2DFVector X(dx.x * (float(x) - m_shift) + m_shift, 
				     dx.y * (float(y) - m_shift) + m_shift); 
			
			if (y == x) {
				cvdebug() << "<x,y> " << x << ", " << y << " to " << X << "\n"; 
			}
			*ic = interp(X);
		}

	set_coefficients(coeffs);
	reinit();
	m_grid_valid = false; 
	return true; 
}


P2DTransformation C2DSplineTransformation::do_upscale(const C2DBounds& size) const
{
	TRACE_FUNCTION;
	C2DFVector mx(((float)size.x)/ ((float)m_range.x),
		      ((float)size.y)/ ((float)m_range.y));

	C2DSplineTransformation *help = new C2DSplineTransformation(size, m_kernel);
	C2DFVectorfield new_coefs(m_coefficients.get_size()); 
	
	transform(m_coefficients.begin(), m_coefficients.end(), new_coefs.begin(), 
		  [&mx](const C2DFVector& x){return x * mx;});
	help->set_coefficients(new_coefs);
	help->m_target_c_rate = m_target_c_rate; 
	return P2DTransformation(help);
}

void C2DSplineTransformation::add(const C2DTransformation& a)
{
	TRACE_FUNCTION;
	assert(a.get_size() == get_size());

	reinit();
	a.reinit();
	
	C2DFVectorfield new_coef(m_coefficients.get_size()); 
	auto i = new_coef.begin();

	for (size_t y = 0; y < m_coefficients.get_size().y; ++y)  {
		for (size_t x = 0; x < m_coefficients.get_size().x; ++x, ++i)  {
			C2DFVector v = C2DFVector(x, y) * m_inv_scale;
			C2DFVector u = a(v);
			*i = v + apply(u) - u;
		}
	}
	
	m_coefficients = new_coef; 

	m_interpolator_valid = false;
}

size_t C2DSplineTransformation::degrees_of_freedom() const
{
	return m_coefficients.size() * 2;
}

P2DImage C2DSplineTransformation::apply(const C2DImage& image,
					const C2DInterpolatorFactory& ipf) const
{
	TRACE_FUNCTION;
	return transform2d(image, ipf, *this);
}

void C2DSplineTransformation::update(float step, const C2DFVectorfield& a)
{
	TRACE_FUNCTION;
	assert(a.get_size() == m_coefficients.get_size());
	C2DFVectorfield::iterator ci = m_coefficients.begin();
	C2DFVectorfield::iterator ce = m_coefficients.end();
	C2DFVectorfield::const_iterator ai = a.begin();

	while( ci != ce ) {
		*ci++ += step * *ai++;
	}
	m_interpolator_valid = false;
}

C2DFMatrix C2DSplineTransformation::derivative_at(const C2DFVector& x) const
{
	std::vector<double> xweights(m_kernel->size()); 
	std::vector<double> yweights(m_kernel->size()); 
	size_t startx = m_kernel->get_start_idx_and_value_weights(x.x, xweights); 
	size_t y = m_kernel->get_start_idx_and_derivative_weights(x.y, yweights); 
	
	C2DFMatrix result; 
	for(auto wy = yweights.begin(); y < m_coefficients.get_size().y && wy != yweights.end(); ++y, ++wy)  {
		C2DFVector h; 
		size_t x = startx; 
		auto wx = xweights.begin();
		for(auto cx = m_coefficients.begin_at(startx,y); 
		    x < m_coefficients.get_size().x && wx != xweights.end(); ++x, ++wx, ++cx)  {
			h += *wx * *cx; 
		}
		result.y += h * *wy; 
	}
	startx = m_kernel->get_start_idx_and_derivative_weights(x.x, xweights); 
	y = m_kernel->get_start_idx_and_value_weights(x.y, yweights); 

	for(auto wy = yweights.begin(); y < m_coefficients.get_size().y && wy != yweights.end(); ++y, ++wy)  {
		C2DFVector h; 
		size_t x = startx; 
		auto wx = xweights.begin();
		for(auto cx = m_coefficients.begin_at(startx,y); 
		    x < m_coefficients.get_size().x && wx != xweights.end(); ++x, ++wx, ++cx)  {
			h += *wx * *cx; 
		}
		result.x += h * *wy; 
	}
	return result; 
	
}

C2DFMatrix C2DSplineTransformation::derivative_at(int x, int y) const
{
	TRACE_FUNCTION;
	assert(m_interpolator_valid);
	const C2DFVector l = scale(C2DFVector(x,y));
	C2DFMatrix d = derivative_at(l);
	cvinfo() << C2DFVector(x,y) << ":" << l << " = [" <<  d.x << d.y << "]\n"; 
	d.x.x = 1.0f - d.x.x * m_scale.x;
	d.x.y =      - d.x.y * m_scale.x;
	d.y.x =      - d.y.x * m_scale.y;
	d.y.y = 1.0f - d.y.y * m_scale.y;
	return d;
}

void C2DSplineTransformation::set_identity()
{
	TRACE_FUNCTION;
	fill(m_coefficients.begin(), m_coefficients.end(), C2DFVector(0,0));
	m_interpolator_valid = false;
}

float C2DSplineTransformation::get_max_transform() const
{
	TRACE_FUNCTION;
	// assuming the maximum spline coef is equal
	C2DFVectorfield::const_iterator i = m_coefficients.begin();
	C2DFVectorfield::const_iterator e = m_coefficients.end();

	assert(i != e);
	float value = i->norm2();

	++i;
	while (i != e) {
		float v = i->norm2();
		if (value < v)
			value = v;
		++i;
	}
	return sqrt(value);


}

void C2DSplineTransformation::init_grid()const
{
	TRACE_FUNCTION; 
	if (!m_grid_valid) {
		cvdebug() << "really run C2DSplineTransformation::init_grid\n"; 
		cvdebug() << "Range = " << m_range << "\n"; 
		cvdebug() << "Coefs = " << m_coefficients.get_size() << "\n"; 			
		cvdebug() << "scale = " << m_scale << "\n"; 			
		// pre-evaluateof fixed-grid coefficients
		// note: here, boundary conditions are of no consequence 
		size_t n_elms = m_kernel->size(); 
		std::vector<double> weights(n_elms); 
		for (size_t i = 0; i < m_range.x; ++i) {
			m_x_indices[i] = m_kernel->get_start_idx_and_value_weights(i * m_scale.x + m_shift, weights);
			m_x_weights[i] = weights;
			cvdebug() << "xrow("<< i << ") start=" << m_x_indices[i] << ", weights=" << weights<< "\n"; 

		}
		for (size_t i = 0; i < m_range.y; ++i) {
			m_y_indices[i] = m_kernel->get_start_idx_and_value_weights(i * m_scale.y + m_shift, weights);
			m_y_weights[i] = weights; 
		}
		m_grid_valid = true; 
	}
}

C2DTransformation::const_iterator C2DSplineTransformation::begin() const
{
	reinit();
	init_grid(); 
	return C2DTransformation::const_iterator(new iterator_impl(C2DBounds(0,0), get_size(), *this));
}

C2DTransformation::const_iterator C2DSplineTransformation::end() const
{
	return C2DTransformation::const_iterator(new iterator_impl(get_size(), get_size(), *this));
}

struct FCopyX {
	double operator() (const C2DFVector& x) const {
		return x.x; 
	}
}; 

struct FCopyY {
	double operator() (const C2DFVector& x) const {
		return x.y; 
	}
}; 

// quadratic extrapolation 
static double extrapolate(double x, double ym, double y0, double yp) 
{
	const double c = y0; 
	const double b = 0.5 * (yp - ym); 
	const double a = ym + b - c; 
	return a * x * x + b * x + c; 
}

void C2DSplineTransformation::run_downscaler(C1DScalarFixed& scaler, vector<double>& out_buffer) const 
{
	const int size = out_buffer.size() - m_shift; 
	scaler.run(); 
	copy(scaler.output_begin(), scaler.output_end(), out_buffer.begin() + m_shift); 
	
	// continue the gradient a both sides with quatratic extrapolation 
	for(int j = 0; j < m_shift; ++j) {
		out_buffer[m_shift - j - 1] = extrapolate(-2, out_buffer[m_shift - j], 
							   out_buffer[m_shift - j + 1], 
							   out_buffer[m_shift - j + 2]); 
		out_buffer[size + j] = extrapolate(2, out_buffer[size + j - 3], 
						   out_buffer[size + j - 2], out_buffer[size + j - 1]); 
	}
}

C2DSplineTransformation::CSplineDerivativeRow 
C2DSplineTransformation::get_derivative_row(size_t nin, size_t nout, double scale) const 
{
	CSplineDerivativeRow result; 
	for(size_t o = 0; o < nout; ++o) {
		CSplineDerivativeRow::value_type v; 
		v.first = -1; 
		for(size_t i = 0; i < nin; ++i) {
			double x = i * scale + m_shift - o; 
			if (fabs(x) <  m_kernel->get_nonzero_radius()) {
				if (v.first < 0)
					v.first = i; 
				double y = m_kernel->get_weight_at(x, 0); 
				if (y > 0) 
					v.second.push_back(y); 
				else 
					break; 
			}
		}
		result.push_back(v); 
	}
	return result; 
}

void C2DSplineTransformation::translate(const C2DFVectorfield& gradient, CDoubleVector& params) const
{
	TRACE_FUNCTION;
	assert(params.size() == m_coefficients.size() * 2);
	assert(gradient.get_size() == m_range); 

	reinit(); 
	
	C2DFVectorfield tmp(C2DBounds(gradient.get_size().x, m_coefficients.get_size().y));
	vector<C2DFVector> in_buffer(gradient.get_size().y); 
	vector<C2DFVector> out_buffer(m_coefficients.get_size().y); 
	
	for (size_t ix = 0; ix < gradient.get_size().x; ++ix) {
		gradient.get_data_line_y(ix, in_buffer);
		for(size_t i = 0; i < m_coefficients.get_size().y; ++i) {
			const CSplineDerivativeRow::value_type& myrow = m_my[i]; 
			out_buffer[i] = inner_product(myrow.second.begin(), myrow.second.end(), 
						      in_buffer.begin() + myrow.first, C2DFVector());
						   
		}
		tmp.put_data_line_y(ix, out_buffer);
	}


	in_buffer.resize(gradient.get_size().x); 

	// x convolution and copy to output
	auto r = params.begin(); 
	for (size_t iy = 0; iy < m_coefficients.get_size().y; ++iy) {
		tmp.get_data_line_x(iy, in_buffer);
		
		for(size_t x = 0; x < m_coefficients.get_size().x; ++x, r+=2) {
			const CSplineDerivativeRow::value_type& mxrow = m_mx[x]; 
			const C2DFVector v = inner_product(mxrow.second.begin(), mxrow.second.end(), 
						      in_buffer.begin() + mxrow.first, C2DFVector());
			r[0] = -v.x; 
			r[1] = -v.y; 
		}
	}
}

float  C2DSplineTransformation::pertuberate(C2DFVectorfield& v) const
{
	TRACE_FUNCTION;
	reinit();
	C2DFVectorfield::iterator iv = v.begin();
	float max_gamma = 0.0f;
	C2DFVector lx_max(0,0);
	for (size_t y = 0; y < v.get_size().y; ++y)
		for (size_t x = 0; x < v.get_size().x; ++x, ++iv){
			C2DFVector lx(x,y);
			C2DFMatrix j = derivative_at(lx);
			j.x.x = j.x.x * m_scale.x;
			j.x.y = j.x.y * m_scale.x;
			j.y.x = j.y.x * m_scale.y;
			j.y.y = j.y.y * m_scale.y;

			const C2DFVector u = j *  *iv;
			*iv -= u;
			float gamma = iv->norm2();
			if (gamma > max_gamma) {
				lx_max = lx;
				max_gamma = gamma;
			}
		}
	cvdebug() << lx_max << "\n";
	return sqrt(max_gamma);
}

float C2DSplineTransformation::get_jacobian(const C2DFVectorfield& v, float delta) const
{
	assert(v.get_size() == get_coeff_size());
	const int dx = v.get_size().x;
	float j_min = numeric_limits<float>::max();
	delta *= 0.5;

	C2DFVector lx_min(0,0);
	for(size_t y = 1; y < v.get_size().y - 1; ++y) {
		C2DFVectorfield::const_iterator iv = v.begin_at(1,y);
		for(size_t x = 1; x < v.get_size().x - 1; ++x, ++iv) {
			const C2DFVector lx(x,y);
			C2DFMatrix J =  derivative_at(lx);
			J.x.x = m_inv_scale.x - J.x.x;
			J.x.y = - J.x.y;
			J.y.x = - J.y.x;
			J.y.y = m_inv_scale.y - J.y.y;

			J.x -= delta * (iv[1]  - iv[-1]);
			J.y -= delta * (iv[dx] - iv[-dx]);

			const float j = J.x.x * J.y.y - J.x.y * J.y.x;
			if ( j_min > j) {
				j_min = j;
				lx_min = lx;
			}
		}

	}
	cvdebug() << lx_min << "\n";
	return j_min * m_scale.y * m_scale.x;
}

C2DFVector C2DSplineTransformation::on_grid(const C2DBounds& x) const
{
	assert(x.x < m_range.x); 
	assert(x.y < m_range.y); 
	assert(m_grid_valid); 
	assert(m_interpolator_valid);

	C2DFVector result; 
	auto & yweights =  m_y_weights[x.y]; 
	auto & yindices =  m_y_indices[x.y]; 
	auto & xweights =  m_x_weights[x.x]; 
	auto & xindices =  m_x_indices[x.x]; 
		
	// on the right boundary the last weight is zero, and access to the coefficient 
	// field would be out of range to multiply with this zero, therefore, we stop 
	// accumulating the values at the right boundary 
	for(size_t y = 0; y < yweights.size() && yindices + y  < m_coefficients.get_size().y; ++y) {
		C2DFVector hr; 
		auto ic = m_coefficients.begin_at(xindices, yindices + y); 
		for(size_t ix = 0; ix < xweights.size() && xindices + ix < m_coefficients.get_size().x; ++ix) {
			hr += xweights[ix] * ic[ix]; 
		}
		result += yweights[y] * hr; 
	}
	return result; 
}

const C2DBounds& C2DSplineTransformation::get_coeff_size() const
{
	return m_coefficients.get_size();
}


C2DSplineTransformation::iterator_impl::iterator_impl(const C2DBounds& pos, const C2DBounds& size, 
						      const C2DSplineTransformation& trans):
	C2DTransformation::iterator_impl(pos,size), 
	m_trans(trans), 
	m_value_valid(false)
{
}
 
C2DTransformation::iterator_impl * C2DSplineTransformation::iterator_impl::clone() const
{
	return new C2DSplineTransformation::iterator_impl(get_pos(), get_size(), m_trans); 
}

const C2DFVector&  C2DSplineTransformation::iterator_impl::do_get_value()const
{
	if (!m_value_valid) {
		m_value = C2DFVector(get_pos()) - m_trans.on_grid(get_pos());
		m_value_valid = true; 
	}
	return m_value; 
}

void C2DSplineTransformation::iterator_impl::do_x_increment()
{
	m_value_valid = false; 
}
void C2DSplineTransformation::iterator_impl::do_y_increment()
{
	m_value_valid = false; 
}


double C2DSplineTransformation::get_divcurl_cost(double wd, double wr, CDoubleVector& gradient) const
{
	reinit(); 
	
	// create PP matrices or adapt size 
	if (!m_divcurl_matrix) 
		m_divcurl_matrix.reset(new C2DPPDivcurlMatrix(m_coefficients.get_size(), m_range, 
							       *m_kernel, wd, wr)); 
	else 
		m_divcurl_matrix->reset(m_coefficients.get_size(), m_range, 
					 *m_kernel, wd, wr); 
	
	
	return m_divcurl_matrix->evaluate(m_coefficients, gradient); 
}

double C2DSplineTransformation::get_divcurl_cost(double wd, double wr) const
{
	reinit(); 
	
	// create PP matrices or adapt size 
	if (!m_divcurl_matrix) 
		m_divcurl_matrix.reset(new C2DPPDivcurlMatrix(m_coefficients.get_size(), m_range, 
							       *m_kernel, wd, wr)); 
	else 
		m_divcurl_matrix->reset(m_coefficients.get_size(), m_range, 
					 *m_kernel, wd, wr); 
	

	return *m_divcurl_matrix * m_coefficients; 
}


class C2DSplineTransformCreator: public C2DTransformCreator {
public:
	C2DSplineTransformCreator(PSplineKernel kernel, const C2DFVector& rates); 
	virtual P2DTransformation do_create(const C2DBounds& size) const;
private:
	PSplineKernel m_kernel;
	C2DFVector m_rates;
};

C2DSplineTransformCreator::C2DSplineTransformCreator(PSplineKernel kernel, const C2DFVector& rates):
	m_kernel(kernel),
	m_rates(rates)
{
}


P2DTransformation C2DSplineTransformCreator::do_create(const C2DBounds& size) const
{
	assert(m_kernel); 
	return P2DTransformation(new C2DSplineTransformation(size, m_kernel, m_rates));
}


class C2DSplineTransformCreatorPlugin: public C2DTransformCreatorPlugin {
public:
	typedef C2DTransformCreatorPlugin::ProductPtr ProductPtr;

	C2DSplineTransformCreatorPlugin();
	virtual C2DTransformCreator *do_create() const;
	virtual bool do_test() const;
	const std::string do_get_descr() const;
private:
	PSplineKernel m_interpolator;
	float m_rate;
};

C2DSplineTransformCreatorPlugin::C2DSplineTransformCreatorPlugin():
	C2DTransformCreatorPlugin("spline"),
	m_interpolator(CSplineKernelPluginHandler::instance().produce("bspline:d=3")),
	m_rate(10)
{
	add_parameter("interp", new CFactoryParameter<CSplineKernelPluginHandler>(m_interpolator, false, 
										  "image interpolator kernel"));
	add_parameter("rate",   new CFloatParameter(m_rate, 1, numeric_limits<float>::max(), false,
						    "isotropic coefficient rate in pixels"));
}

C2DTransformCreator *C2DSplineTransformCreatorPlugin::do_create() const
{
	return new C2DSplineTransformCreator(m_interpolator, C2DFVector(m_rate, m_rate));
}

bool C2DSplineTransformCreatorPlugin::do_test() const
{
	return true;
}

const std::string C2DSplineTransformCreatorPlugin::do_get_descr() const
{
	return "plugin to create spline transformations";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DSplineTransformCreatorPlugin();
}

NS_MIA_END

