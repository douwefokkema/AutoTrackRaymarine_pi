///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.10.1-0-g8feb16b3)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/dialog.h>

#include "wxWTranslateCatalog.h"

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class PreferencesDialogBase
///////////////////////////////////////////////////////////////////////////////
class PreferencesDialogBase : public wxDialog
{
	private:

	protected:
		wxStaticText* m_staticText101;
		wxSpinCtrl* m_sMaxAngle1;
		wxStaticText* m_staticText1011;
		wxSpinCtrl* m_sensitivity;
		wxStaticText* m_staticText10111;
		wxCheckBox* m_checkBox1;
		wxStaticText* m_staticText1011131;
		wxButton* m_button1;
		wxStaticText* m_staticText101111;
		wxTextCtrl* m_textCtrl1;
		wxStaticText* m_staticText101112;
		wxTextCtrl* m_textCtrl2;
		wxStaticText* m_staticText101113;
		wxTextCtrl* m_textCtrl3;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;

		// Virtual event handlers, override them in your derived class
		virtual void OnOk( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnDefault(wxCommandEvent& event) { event.Skip(); }


	public:

		PreferencesDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("AutoTrackRaymarine Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL );

		~PreferencesDialogBase();

};

