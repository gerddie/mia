/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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

#include <wx/wx.h>
#include <sstream>

#include <mia/2d/2dimageio.hh>
#include <mia/core.hh>

#include <gui/serieswindow.hh>

NS_MIA_USE;
using namespace std;


class CMyoCardApp: public wxApp {
public:
	virtual bool OnInit();
};

class CMyoMainframe : public wxFrame {
public:
	CMyoMainframe(const wxString& title);

	// event handlers
	void OnQuit(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
private:
	DECLARE_EVENT_TABLE();

	CSeriesPanel *panel;
};


DECLARE_APP(CMyoCardApp);
IMPLEMENT_APP_NO_MAIN(CMyoCardApp);

bool CMyoCardApp::OnInit()
{
	CMyoMainframe *frame =  new CMyoMainframe(wxT("Myocardial Perfusion Analysis"));
	frame->Show(true);

	return true;
}

BEGIN_EVENT_TABLE(CMyoMainframe, wxFrame)
	EVT_MENU(wxID_ABOUT, CMyoMainframe::OnAbout)
	EVT_MENU(wxID_OPEN,  CMyoMainframe::OnOpen)
	EVT_MENU(wxID_EXIT,  CMyoMainframe::OnQuit)
	END_EVENT_TABLE();

void CMyoMainframe::OnQuit(wxCommandEvent& /*event*/)
{
	Close();
}

void CMyoMainframe::OnOpen(wxCommandEvent& /*event*/)
{
	wxString caption = wxT("Choos a set of files");
	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	const C2DImageIOPluginHandler::CSuffixmap& supported_types = imageio.get_supported_filetype_map();
	if (supported_types.empty()) {
		wxString msg;
		msg.Printf(wxT("No image types supported!"));
		wxMessageBox(msg, wxT("Error"), wxOK | wxICON_ERROR, this);
		return;
	}


	wxString wildcard;

	wildcard.append(wxT("All files (*.*)|*.*"));

	C2DImageIOPluginHandler::CSuffixmap::const_iterator i = supported_types.begin();
	for ( ; i != supported_types.end(); ++i) {
		wxString name(i->second.c_str(), wxConvUTF8);
		wxString suffix(i->first.c_str(), wxConvUTF8);
		wildcard.append(wxT("|"));
		wildcard.append(name);
		wildcard.append(wxT(" files (*"));
		wildcard.append(suffix);
		wildcard.append(wxT(")|*"));
		wildcard.append(suffix);
	}
	wxString defaultDir = wxT(".");
	wxString defaultFilename = wxEmptyString;

	wxFileDialog zelga(this, caption, defaultDir, defaultFilename, wildcard, wxMULTIPLE);
	if (zelga.ShowModal() == wxID_OK) {
		wxArrayString filenames;
		zelga.GetPaths(filenames);
		filenames.Sort();

		C2DImageIOPluginHandler::Instance::PData all_images(new C2DImageIOPluginHandler::Instance::Data);
		// open the files using the MIA interface
		for (size_t i = 0; i < filenames.GetCount(); ++i) {
			SetStatusText(filenames[i]);
			string name(filenames[i].char_str());
			cvmsg() << name << "\n";
			C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(name);
			all_images->insert(all_images->end(), in_image_list->begin(), in_image_list->end());
		}
		panel->set_series(all_images);

		stringstream msg;
		msg << "Loaded " << all_images->size() << " frames";
		SetStatusText(wxString(msg.str().c_str(), wxConvUTF8));
		// redraw the viewer ...
	}
}

void CMyoMainframe::OnAbout(wxCommandEvent& /*event*/)
{
	wxString msg;
	msg.Printf(wxT("This is a tool to analyse myocardial perfusion MRI series"));
	wxMessageBox(msg, wxT("About"), wxOK | wxICON_INFORMATION, this);
}

CMyoMainframe::CMyoMainframe(const wxString& title):
	wxFrame(NULL, wxID_ANY, title)
{
	wxMenu *fileMenu = new wxMenu;
	wxMenu *helpMenu = new wxMenu;

	helpMenu->Append(wxID_ABOUT, wxT("&About...\tF1"), wxT("Show abput dialog"));

	fileMenu->Append(wxID_OPEN, wxT("&Open\tCtrl-O"), wxT("Open a set of images"));
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, wxT("&Quit\tCtrl-Q"), wxT("Quit this program"));

	wxMenuBar *menuBar = new wxMenuBar;

	menuBar->Append(fileMenu, wxT("&File"));
	menuBar->Append(helpMenu, wxT("&Help"));
	SetMenuBar(menuBar);

	panel = new CSeriesPanel(this, wxID_ANY, wxT("the display"));
	panel->Show();

	CreateStatusBar(2);
	SetStatusText(wxT("Welcome ..."));
}



int main(int argc, char **argv)
{
	CCmdOptionList options("This program does nothing.");

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	// normally the next call should use the remaining arguments
	wxEntry(argc, argv);

}
