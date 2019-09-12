///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jul 28 2019)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/statline.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class m_dialog
///////////////////////////////////////////////////////////////////////////////
class m_dialog : public wxDialog
{
	private:

	public:
		wxStaticLine* StaticLine1;
		wxButton* buttonAuto;
		wxButton* buttonStandby;
		wxTextCtrl* pilot_state;
		wxTextCtrl* TextStatus11;
		wxTextCtrl* heading;
		wxTextCtrl* TextStatus12;
		wxTextCtrl* track;
		wxTextCtrl* TextStatus14;
		wxTextCtrl* XTE;
		wxTextCtrl* TextStatus121;
		wxStaticLine* StaticLine11;
		wxButton* buttonDecOne;
		wxButton* buttonDecTen;
		wxButton* buttonIncTen;
		wxButton* buttonIncOne;

		// Virtual event handlers, overide them in your derived class
		virtual void OnActiveApp( wxActivateEvent& event ) { event.Skip(); }
		virtual void OnAuto( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnStandby( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDecrementOne( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDecrementTen( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnIncrementTen( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnIncrementOne( wxCommandEvent& event ) { event.Skip(); }


	public:
		wxStaticLine* StaticLine3;

		m_dialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Autopilot and Track Control"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 400,350 ), long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxMINIMIZE_BOX|wxRESIZE_BORDER, const wxString& name = wxT("Raymarine Autopilot") );
		~m_dialog();

};

