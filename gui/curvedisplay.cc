/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <gui/curvedisplay.hh>

#include <mia/2d/2dfilter.hh>
#include <mia/core/msgstream.hh>

using namespace std;
NS_MIA_USE;

struct CCurveDisplayImpl {
	CCurveDisplayImpl(CCurveDisplay *parent);
	void set_series(const C2DImageIOPluginHandler::Instance::PData& series);
	void set_coordinates(const wxPoint& point);
	const vector<double>& get_values() const;
	void get_get_scale(const wxSize& sz, double& x_scale, double& y_scale) const;
private:
	void evaluate() const;
	mia::C2DImageIOPluginHandler::Instance::PData m_series;
	CCurveDisplay *m_parent;
	wxPoint m_point;
	mutable vector<double> m_values;
	mutable bool m_values_valid;
	mutable double m_min;
	mutable double m_max;
};

CCurveDisplayImpl::CCurveDisplayImpl(CCurveDisplay *parent):
	m_parent(parent),
	m_point(0,0),
	m_values_valid(false)
{
}

void CCurveDisplayImpl::set_series(const C2DImageIOPluginHandler::Instance::PData& series)
{
	m_series = series;
	m_values_valid = false;
}

void CCurveDisplayImpl::set_coordinates(const wxPoint& point)
{
	m_point = point;
	m_values_valid = false;
}

class FValueAccumulator: public TFilter<int> {
public:
	FValueAccumulator(vector<double>& values, const wxPoint& x):
		m_values(values),
		m_min(0),
		m_max(0),
		m_x(x)
	{
	}
	template <typename T>
	int operator () (const T2DImage<T>& image) {
		if ((size_t) m_x.x < image.get_size().x && (size_t) m_x.y < image.get_size().y) {
			double v = image(m_x.x,m_x.y);
			if (m_min > v)
				m_min = v;
			if (m_max < v)
				m_max = v;
			m_values.push_back(v);
		} else
			m_values.push_back(0);
		return 0;
	}

	void get_min_max(double& min, double& max) const {
		min = m_min;
		max = m_max;
	}
private:
	vector<double>& m_values;
	double m_min;
	double m_max;
	const wxPoint& m_x;
};

void CCurveDisplayImpl::evaluate() const
{
	m_values.resize(0);
	m_min = 0;
	m_max = 256;
	if (m_series.get()) {
		FValueAccumulator va(m_values, m_point);
		for (C2DImageIOPluginHandler::Instance::Data::iterator i = m_series->begin();
		     i != m_series->end(); ++i)
			mia::accumulate(va, **i);
		va.get_min_max(m_min, m_max);
	}
	m_values_valid = true;
}

const vector<double>& CCurveDisplayImpl::get_values() const
{
	if (!m_values_valid)
		evaluate();
	return m_values;
}

void CCurveDisplayImpl::get_get_scale(const wxSize& sz, double& x_scale, double& y_scale) const
{
	if (!m_values_valid)
		evaluate();

	x_scale = m_values.empty() ? 0 : (double)sz.x / m_values.size();
	y_scale = (m_max == m_min) ? 0 : ((double)sz.y) / (m_max - m_min);

}



CCurveDisplay::CCurveDisplay(wxWindow *parent):
	wxPanel(parent, wxID_ANY, wxPoint(0,0), wxSize(300,256)),
	impl(new CCurveDisplayImpl(this))
{
}

CCurveDisplay::~CCurveDisplay()
{
	delete impl;
}

void CCurveDisplay::set_series(const C2DImageIOPluginHandler::Instance::PData& series)
{
	impl->set_series(series);
}

void CCurveDisplay::set_coordinates(const wxPoint& point)
{
	cvmsg() << "coord = " << point.x << ", " << point.y << "\n";
	impl->set_coordinates(point);
	Refresh();
}

void CCurveDisplay::OnPaint(wxPaintEvent& /*event*/)
{
	cvmsg() << "CCurveDisplay::OnPaint\n";
	wxPaintDC dc(this);
	wxSize sz = GetClientSize();

	const vector<double>& data = impl->get_values();

	double x_scale, y_scale;
	impl->get_get_scale(sz, x_scale, y_scale);
	cvmsg() << "x_scale:" << x_scale << ", y_scale:" << y_scale << "\n";


	vector<wxPoint> points;
	for ( size_t i = 0; i < data.size(); ++i)
		points.push_back(wxPoint(x_scale * i, y_scale * data[i]));

	dc.DrawLines(points.size(), &points[0]);

}

BEGIN_EVENT_TABLE(CCurveDisplay, wxWindow)
	EVT_PAINT(CCurveDisplay::OnPaint)
	END_EVENT_TABLE();

