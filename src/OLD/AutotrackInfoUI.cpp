///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.10.1-0-g8feb16b3)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "AutotrackInfoUI.h"

///////////////////////////////////////////////////////////////////////////

m_dialog::m_dialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxDialog( parent, id, title, pos, size, style, name )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );
	this->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_MENU ) );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );

	StaticLine1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxLI_HORIZONTAL );
	bSizer10->Add( StaticLine1, 0, wxEXPAND | wxALL, 5 );

	wxGridSizer* gSizer7;
	gSizer7 = new wxGridSizer( 0, 3, 0, 0 );

	buttonAuto = new wxButton( this, wxID_ANY, wxT("Auto"), wxDefaultPosition, wxSize( -1,-1 ), 0|wxBORDER_DEFAULT );
	buttonAuto->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	buttonAuto->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNTEXT ) );
	buttonAuto->SetBackgroundColour( wxColour( 255, 0, 0 ) );

	gSizer7->Add( buttonAuto, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );

	buttonStandby = new wxButton( this, wxID_ANY, wxT("Standby"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	buttonStandby->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	buttonStandby->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNTEXT ) );
	buttonStandby->SetBackgroundColour( wxColour( 0, 255, 0 ) );

	gSizer7->Add( buttonStandby, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );

	buttonTrack = new wxButton( this, wxID_ANY, wxT("Tracking"), wxDefaultPosition, wxSize( -1,-1 ), 0|wxBORDER_DEFAULT );
	buttonTrack->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	buttonTrack->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNTEXT ) );
	buttonTrack->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION ) );

	gSizer7->Add( buttonTrack, 0, wxALL, 5 );


	bSizer10->Add( gSizer7, 0, wxALIGN_CENTER|wxRIGHT|wxLEFT, 4 );

	StaticLine3 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer10->Add( StaticLine3, 0, wxALL|wxEXPAND, 5 );

	wxGridSizer* gSizer4;
	gSizer4 = new wxGridSizer( 4, 2, 0, 0 );

	pilot_state = new wxTextCtrl( this, wxID_ANY, wxT("State"), wxDefaultPosition, wxSize( -1,-1 ), wxTE_READONLY|wxTE_CENTER );
	#ifdef __WXGTK__
	if ( !pilot_state->HasFlag( wxTE_MULTILINE ) )
	{
	pilot_state->SetMaxLength( 100 );
	}
	#else
	pilot_state->SetMaxLength( 100 );
	#endif
	pilot_state->SetFont( wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	pilot_state->SetForegroundColour( wxColour( 0, 0, 128 ) );
	pilot_state->SetBackgroundColour( wxColour( 255, 255, 225 ) );

	gSizer4->Add( pilot_state, 0, wxALL, 5 );

	TextStatus11 = new wxTextCtrl( this, wxID_ANY, wxT("Standby"), wxDefaultPosition, wxSize( -1,-1 ), wxTE_CENTER );
	#ifdef __WXGTK__
	if ( !TextStatus11->HasFlag( wxTE_MULTILINE ) )
	{
	TextStatus11->SetMaxLength( 200 );
	}
	#else
	TextStatus11->SetMaxLength( 200 );
	#endif
	TextStatus11->SetFont( wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	TextStatus11->SetForegroundColour( wxColour( 0, 0, 128 ) );
	TextStatus11->SetBackgroundColour( wxColour( 255, 255, 225 ) );

	gSizer4->Add( TextStatus11, 0, wxALL, 5 );

	heading = new wxTextCtrl( this, wxID_ANY, wxT("Heading"), wxDefaultPosition, wxSize( -1,-1 ), wxTE_READONLY|wxTE_CENTER );
	#ifdef __WXGTK__
	if ( !heading->HasFlag( wxTE_MULTILINE ) )
	{
	heading->SetMaxLength( 100 );
	}
	#else
	heading->SetMaxLength( 100 );
	#endif
	heading->SetFont( wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	heading->SetForegroundColour( wxColour( 0, 0, 128 ) );
	heading->SetBackgroundColour( wxColour( 255, 255, 225 ) );

	gSizer4->Add( heading, 0, wxALL, 5 );

	TextStatus12 = new wxTextCtrl( this, wxID_ANY, wxT("-----"), wxDefaultPosition, wxSize( -1,-1 ), wxTE_CENTER );
	#ifdef __WXGTK__
	if ( !TextStatus12->HasFlag( wxTE_MULTILINE ) )
	{
	TextStatus12->SetMaxLength( 200 );
	}
	#else
	TextStatus12->SetMaxLength( 200 );
	#endif
	TextStatus12->SetFont( wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	TextStatus12->SetForegroundColour( wxColour( 0, 0, 128 ) );
	TextStatus12->SetBackgroundColour( wxColour( 255, 255, 225 ) );

	gSizer4->Add( TextStatus12, 0, wxALL, 5 );

	track = new wxTextCtrl( this, wxID_ANY, wxT("Track"), wxDefaultPosition, wxSize( -1,-1 ), wxTE_READONLY|wxTE_CENTER );
	#ifdef __WXGTK__
	if ( !track->HasFlag( wxTE_MULTILINE ) )
	{
	track->SetMaxLength( 100 );
	}
	#else
	track->SetMaxLength( 100 );
	#endif
	track->SetFont( wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	track->SetForegroundColour( wxColour( 0, 0, 128 ) );
	track->SetBackgroundColour( wxColour( 255, 255, 225 ) );

	gSizer4->Add( track, 0, wxALL, 5 );

	TextStatus14 = new wxTextCtrl( this, wxID_ANY, wxT("-----"), wxDefaultPosition, wxSize( -1,-1 ), wxTE_CENTER );
	#ifdef __WXGTK__
	if ( !TextStatus14->HasFlag( wxTE_MULTILINE ) )
	{
	TextStatus14->SetMaxLength( 200 );
	}
	#else
	TextStatus14->SetMaxLength( 200 );
	#endif
	TextStatus14->SetFont( wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	TextStatus14->SetForegroundColour( wxColour( 0, 0, 128 ) );
	TextStatus14->SetBackgroundColour( wxColour( 255, 255, 225 ) );

	gSizer4->Add( TextStatus14, 0, wxALL, 5 );

	XTE = new wxTextCtrl( this, wxID_ANY, wxT("XTE"), wxDefaultPosition, wxSize( -1,-1 ), wxTE_READONLY|wxTE_CENTER );
	#ifdef __WXGTK__
	if ( !XTE->HasFlag( wxTE_MULTILINE ) )
	{
	XTE->SetMaxLength( 100 );
	}
	#else
	XTE->SetMaxLength( 100 );
	#endif
	XTE->SetFont( wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	XTE->SetForegroundColour( wxColour( 0, 0, 128 ) );
	XTE->SetBackgroundColour( wxColour( 255, 255, 225 ) );

	gSizer4->Add( XTE, 0, wxALL, 5 );

	TextStatus121 = new wxTextCtrl( this, wxID_ANY, wxT("-----"), wxDefaultPosition, wxSize( -1,-1 ), wxTE_CENTER );
	#ifdef __WXGTK__
	if ( !TextStatus121->HasFlag( wxTE_MULTILINE ) )
	{
	TextStatus121->SetMaxLength( 200 );
	}
	#else
	TextStatus121->SetMaxLength( 200 );
	#endif
	TextStatus121->SetFont( wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	TextStatus121->SetForegroundColour( wxColour( 0, 0, 128 ) );
	TextStatus121->SetBackgroundColour( wxColour( 255, 255, 225 ) );

	gSizer4->Add( TextStatus121, 0, wxALL, 5 );


	bSizer10->Add( gSizer4, 1, wxEXPAND, 5 );

	StaticLine11 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxLI_HORIZONTAL );
	bSizer10->Add( StaticLine11, 0, wxEXPAND | wxALL, 5 );

	wxGridSizer* gSizer6;
	gSizer6 = new wxGridSizer( 0, 4, 0, 0 );

	buttonDecOne = new wxButton( this, wxID_ANY, wxT("-1"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	buttonDecOne->SetFont( wxFont( 14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	buttonDecOne->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );

	gSizer6->Add( buttonDecOne, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );

	buttonDecTen = new wxButton( this, wxID_ANY, wxT("-10"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	buttonDecTen->SetFont( wxFont( 14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	buttonDecTen->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );

	gSizer6->Add( buttonDecTen, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxLEFT, 3 );

	buttonIncTen = new wxButton( this, wxID_ANY, wxT("+10"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	buttonIncTen->SetFont( wxFont( 14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	buttonIncTen->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );

	gSizer6->Add( buttonIncTen, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxLEFT, 3 );

	buttonIncOne = new wxButton( this, wxID_ANY, wxT("+1"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	buttonIncOne->SetFont( wxFont( 14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Arial") ) );
	buttonIncOne->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );

	gSizer6->Add( buttonIncOne, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );


	bSizer10->Add( gSizer6, 0, wxFIXED_MINSIZE|wxLEFT|wxRIGHT|wxSHAPED|wxALIGN_CENTER_HORIZONTAL, 5 );


	this->SetSizer( bSizer10 );
	this->Layout();
	bSizer10->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_ACTIVATE_APP, wxActivateEventHandler( m_dialog::OnActiveApp ) );
	buttonAuto->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnAuto ), NULL, this );
	buttonStandby->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnStandby ), NULL, this );
	buttonTrack->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnTracking ), NULL, this );
	buttonDecOne->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnMinusOne ), NULL, this );
	buttonDecTen->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnMinusTen ), NULL, this );
	buttonIncTen->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnPlusTen ), NULL, this );
	buttonIncOne->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnPlusOne ), NULL, this );
}

m_dialog::~m_dialog()
{
	// Disconnect Events
	this->Disconnect( wxEVT_ACTIVATE_APP, wxActivateEventHandler( m_dialog::OnActiveApp ) );
	buttonAuto->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnAuto ), NULL, this );
	buttonStandby->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnStandby ), NULL, this );
	buttonTrack->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnTracking ), NULL, this );
	buttonDecOne->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnMinusOne ), NULL, this );
	buttonDecTen->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnMinusTen ), NULL, this );
	buttonIncTen->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnPlusTen ), NULL, this );
	buttonIncOne->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_dialog::OnPlusOne ), NULL, this );

}
