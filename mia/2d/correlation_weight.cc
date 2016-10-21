/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/2d/correlation_weight.hh>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN
using namespace std;



struct CCorrelationEvaluatorImpl {
	CCorrelationEvaluatorImpl(double thresh);

	CCorrelationEvaluator::result_type run(const vector<P2DImage>& images, size_t skip) const;
private:

	double m_thresh;
};

struct FCorrelationAccumulator : public TFilter<bool> {

	FCorrelationAccumulator(const C2DBounds & size, double thresh);

	template <typename T>
	bool operator ()(const T2DImage<T>& image);

	C2DFImage get_horizontal_corr() const;
	C2DFImage get_vertical_corr() const;
private:
	float corr(double x, double y, double xx, double yy, double xy )const;

	C2DDImage sx2;

	C2DDImage sxy_horizontal;
	C2DDImage sxy_vertical;

	C2DDImage sx;
	C2DDImage sy;
	C2DBounds size;
	double m_thresh;
	size_t len;
};


CCorrelationEvaluator::CCorrelationEvaluator(double thresh):
	impl(new CCorrelationEvaluatorImpl(thresh))
{
}

CCorrelationEvaluator::~CCorrelationEvaluator()
{
	delete impl;
}


CCorrelationEvaluator::result_type CCorrelationEvaluator::operator() (const vector<P2DImage>& images, size_t skip) const
{
	return impl->run(images, skip);
}


CCorrelationEvaluatorImpl::CCorrelationEvaluatorImpl(double thresh):
	m_thresh(thresh)
{
}

CCorrelationEvaluator::result_type CCorrelationEvaluatorImpl::run(const vector<P2DImage>& images, size_t skip) const
{
	// accumulate sums
	if (skip >= images.size())
		throw create_exception<invalid_argument>( "Want to skip ", skip, 
						"images, but series has only ", images.size() , " images"); 
	FCorrelationAccumulator acc(images[0]->get_size(), m_thresh);
	for (auto i = images.begin() + skip; i != images.end(); ++i)
		mia::accumulate(acc,**i);

	CCorrelationEvaluator::result_type result;
	result.horizontal = acc.get_horizontal_corr();
	result.vertical = acc.get_vertical_corr();
	return result;
}


FCorrelationAccumulator::FCorrelationAccumulator(const C2DBounds & _size, double thresh):
	sx2(_size),
	sxy_horizontal(_size),
	sxy_vertical(_size),
	sx(_size),
	size(_size),
	m_thresh(thresh),
	len(0)
{
}

template <typename T>
bool FCorrelationAccumulator::operator ()(const T2DImage<T>& image)
{
	if (image.get_size() != size)
		throw create_exception<invalid_argument>( "Input image size ", size, " expected, but got ",
						image.get_size());
	// sum x
	transform(image.begin(), image.end(), sx.begin(), sx.begin(), [](T a, T b){ return a + b; });

	// sum x^2
	transform(image.begin(), image.end(), sx2.begin(), sx2.begin(), [](T a, T b){return a * a + b; }); 

	// sum horizontal
	for (size_t y = 0; y < size.y; ++y) {
		auto irow = image.begin_at(0,y);
		auto orow = sxy_horizontal.begin_at(0,y);
		for (size_t x = 0; x < size.x-1; ++x, ++irow, ++orow) {
			*orow += irow[0] * irow[1];
		}
	}

	// sum vertical
	for (size_t y = 1; y < size.y; ++y) {
		auto irow0 = image.begin_at(0,y-1);
		auto irow1 = image.begin_at(0,y);
		auto orow = sxy_vertical.begin_at(0,y-1);
		for (size_t x = 0; x < size.x; ++x, ++irow0, ++irow1,++orow) {
			*orow += *irow0 * *irow1;
		}
	}
	++len;
	return true;
}


float FCorrelationAccumulator::corr(double x, double y, double xx, double yy, double xy )const
{
	const float ssxy = xy - x * y / len;
	const float ssxx = xx - x * x / len;
	const float ssyy = yy - y * y / len;

	if (ssxx == 0 && ssyy == 0)
		return  1.0;
	else if (ssxx == 0 || ssyy == 0)
		return 0.0;
	else {
		float v = ssxy /  sqrt(ssxx * ssyy);
		return v >= m_thresh ? v : 0.0f;
	}
}

C2DFImage FCorrelationAccumulator::get_horizontal_corr() const
{
	if (!len)
		throw create_exception<invalid_argument>( "No input images");


	C2DFImage result(C2DBounds(size.x-1, size.y));

	for (size_t y = 0; y < size.y; ++y) {
		auto irow_xy = sxy_horizontal.begin_at(0,y);
		auto irow_xx = sx2.begin_at(0,y);
		auto irow_yy = sx2.begin_at(1,y);
		auto irow_x  = sx.begin_at(0,y);
		auto irow_y  = sx.begin_at(1,y);
		auto orow    = result.begin_at(0,y);

		for (size_t x = 1; x < size.x;
		     ++x, ++irow_xy, ++irow_xx, ++irow_yy, ++irow_x, ++irow_y, ++orow) {
			*orow = corr(*irow_x, *irow_y, *irow_xx, *irow_yy, *irow_xy );
		}
		++irow_xy; ++irow_xx; ++irow_yy; ++irow_x; ++irow_y;
	}
	return result;
}

C2DFImage FCorrelationAccumulator::get_vertical_corr() const
{
	if (!len)
		throw create_exception<invalid_argument>("No input images");

	C2DFImage result(C2DBounds(size.x, size.y-1));

	for (size_t y = 0; y < size.y-1; ++y) {
		auto irow_xy = sxy_vertical.begin_at(0,y);
		auto irow_xx = sx2.begin_at(0,y);
		auto irow_yy = sx2.begin_at(0,y+1);
		auto irow_x  = sx.begin_at(0,y);
		auto irow_y  = sx.begin_at(0,y+1);
		auto orow    = result.begin_at(0,y);

		for (size_t x = 0; x < size.x;
		     ++x, ++irow_xy, ++irow_xx, ++irow_yy, ++irow_x, ++irow_y, ++orow) {
			*orow = corr(*irow_x, *irow_y, *irow_xx, *irow_yy, *irow_xy );
		}
	}
	return result;
}


NS_MIA_END
