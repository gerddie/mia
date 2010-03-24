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

#include <gui/serieswindow.hh>

#include <wx/splitter.h>
#include <wx/slider.h>

#include <mia/2d/2dfilter.hh>
#include <mia/core/msgstream.hh>

#include <gui/curvedisplay.hh>

NS_MIA_USE;

#define ID_IMAGE_SELECT (wxID_HIGHEST + 1)

class CImageDisplay: public wxWindow {
public:
	CImageDisplay(wxWindow *parent, CCurveDisplay* curve_display);

	void SetImage(const wxImage& image);

	void OnPaint(wxPaintEvent& event);
	void  OnLeftButton(wxMouseEvent& event);

private:
	DECLARE_EVENT_TABLE();

	wxBitmap m_currentBitmap;
	CCurveDisplay* m_curve_display;
};


struct  CSeriesPanelImpl {
	C2DImageIOPluginHandler::Instance::PData m_series;

	vector<wxImage> m_series_shadow;
	wxSlider *m_image_select;
	CImageDisplay *m_image_display;
	CCurveDisplay *m_curve_display;

	CSeriesPanelImpl(wxWindow *parent);
	void ImageSelect();
	void set_series(const C2DImageIOPluginHandler::Instance::PData& series);
};

void CSeriesPanelImpl::ImageSelect()
{

	size_t i = m_image_select->GetValue();

	if (i < m_series_shadow.size())
		m_image_display->SetImage( m_series_shadow[i] );
}

void CSeriesPanelImpl::set_series(const C2DImageIOPluginHandler::Instance::PData& series)
{
	m_series = series;
	m_curve_display->set_series(m_series);

	if (m_series.get()) {
		m_series_shadow.resize(series->size());

		C2DFilterPlugin::ProductPtr converter = C2DFilterPluginHandler::instance().produce("convert:repn=ubyte");

		size_t pos = 0;
		for(C2DImageIOPluginHandler::Instance::Data::const_iterator i = m_series->begin();
		    i != m_series->end(); ++i, ++pos) {
			P2DImage cimg = converter->filter(**i);
			C2DUBImage *image =  dynamic_cast<C2DUBImage *>(cimg.get());

			unsigned char *buf = (unsigned char *)malloc(3 * image->size());

			unsigned char* bi = buf;
			for (C2DUBImage::const_iterator k = image->begin(); k != image->end(); ++k, bi += 3)
				bi[0] = bi[1] = bi[2] = *k;

			m_series_shadow[pos] = wxImage(image->get_size().x, image->get_size().y, buf);

			m_image_select->SetRange(0, m_series_shadow.size());
			m_image_select->SetValue(m_series_shadow.size() / 2);
		}
	}else
		m_image_select->SetRange(0, 0);
}

CSeriesPanelImpl::CSeriesPanelImpl(wxWindow *parent)
{
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags sizer_flags;
		sizer_flags.Border(wxALL, 2).Expand();


	m_image_select = new wxSlider(parent, ID_IMAGE_SELECT, 0, 0, 10, wxDefaultPosition, wxSize(-1, 128),
				      wxSL_VERTICAL| wxSL_LABELS);
	m_image_select->Show();

	sizer->Add(m_image_select, sizer_flags);



	m_curve_display = new CCurveDisplay(parent);
	m_curve_display->Show();

	m_image_display = new CImageDisplay(parent, m_curve_display);
	m_image_display->Show();

	sizer->Add(m_image_display, sizer_flags);
	sizer->Add(m_curve_display, sizer_flags);

	parent->SetSizer(sizer);
	sizer->Fit(parent);
	sizer->SetSizeHints(parent);

}

CSeriesPanel::CSeriesPanel(wxWindow *parent, wxWindowID id, const wxString& name):
	wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxNO_BORDER, name),
	impl(new CSeriesPanelImpl(this))
{
}

CSeriesPanel::~CSeriesPanel()
{
	delete impl;
}

void CSeriesPanel::set_series(const C2DImageIOPluginHandler::Instance::PData& series)
{
	impl->set_series(series);
	impl->ImageSelect();
}

void CSeriesPanel::OnImageSelect(wxCommandEvent& /*event*/)
{
	impl->ImageSelect();
}


CImageDisplay::CImageDisplay(wxWindow *parent, CCurveDisplay* curve_display):
	wxWindow(parent, wxID_ANY, wxPoint(0,0), wxSize(300,300)),
	m_curve_display(curve_display)
{
}

void CImageDisplay::SetImage(const wxImage& image)
{

	wxSize sz = GetClientSize();
	int size = sz.x <  sz.y ? sz.x : sz.y;
	m_currentBitmap = wxBitmap(image.Scale(size, size));
	// now redraw
	Refresh();
}

void CImageDisplay::OnPaint(wxPaintEvent& /*event*/)
{
	wxPaintDC dc(this);
	wxSize sz = GetClientSize();

	wxMemoryDC dcSource;
	dcSource.SelectObject(m_currentBitmap);

	dc.DrawBitmap(m_currentBitmap, 0,0, false);

	dcSource.SelectObject(wxNullBitmap);

	// draw overlay


}

void  CImageDisplay::OnLeftButton(wxMouseEvent& event)
{
	wxPoint pos = event.GetPosition();
	wxSize  sz = GetClientSize();

	wxPoint p( (pos.x * 128 + (sz.x >> 1)) / sz.x,
		   (pos.y * 128 + (sz.y >> 1)) / sz.y);

	m_curve_display->set_coordinates(p);
}

BEGIN_EVENT_TABLE(CSeriesPanel, wxPanel)
	EVT_SLIDER(ID_IMAGE_SELECT, CSeriesPanel::OnImageSelect)
	END_EVENT_TABLE();

BEGIN_EVENT_TABLE(CImageDisplay, wxWindow)
	EVT_PAINT(CImageDisplay::OnPaint)
	EVT_LEFT_DOWN( CImageDisplay::OnLeftButton)
	END_EVENT_TABLE();
