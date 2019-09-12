///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jul 28 2019)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "AutotrackPrefsUI.h"

///////////////////////////////////////////////////////////////////////////

PreferencesDialogBase::PreferencesDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );

	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer8->AddGrowableCol( 0 );
	fgSizer8->AddGrowableCol( 1 );
	fgSizer8->AddGrowableRow( 0 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

	wxFlexGridSizer* fgSizer162;
	fgSizer162 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer162->AddGrowableCol( 0 );
	fgSizer162->SetFlexibleDirection( wxBOTH );
	fgSizer162->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Settings") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer191;
	fgSizer191 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer191->SetFlexibleDirection( wxBOTH );
	fgSizer191->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText101 = new wxStaticText( sbSizer5->GetStaticBox(), wxID_ANY, _("Max Error Angle"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText101->Wrap( -1 );
	fgSizer191->Add( m_staticText101, 0, wxALL, 5 );

	m_sMaxAngle1 = new wxSpinCtrl( sbSizer5->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 1, 90, 30 );
	fgSizer191->Add( m_sMaxAngle1, 0, wxALL, 5 );


	sbSizer5->Add( fgSizer191, 1, wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer192;
	fgSizer192 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer192->SetFlexibleDirection( wxBOTH );
	fgSizer192->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText102 = new wxStaticText( sbSizer5->GetStaticBox(), wxID_ANY, _("Com port NGT-1"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText102->Wrap( -1 );
	fgSizer192->Add( m_staticText102, 0, wxALL, 5 );

	m_comboBox1 = new wxComboBox( sbSizer5->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	m_comboBox1->Append( _("COM1") );
	m_comboBox1->Append( _("COM2") );
	m_comboBox1->Append( _("COM3") );
	fgSizer192->Add( m_comboBox1, 0, wxALL, 5 );


	sbSizer5->Add( fgSizer192, 1, wxEXPAND, 5 );


	fgSizer162->Add( sbSizer5, 1, wxEXPAND, 5 );


	fgSizer8->Add( fgSizer162, 1, wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer181;
	fgSizer181 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer181->AddGrowableCol( 0 );
	fgSizer181->SetFlexibleDirection( wxBOTH );
	fgSizer181->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticBoxSizer* sbSizer10;
	sbSizer10 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Options") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer45;
	fgSizer45 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer45->SetFlexibleDirection( wxBOTH );
	fgSizer45->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_cbConfirmBearingChange = new wxCheckBox( sbSizer10->GetStaticBox(), wxID_ANY, _("Confirm Bearing Change\nOn Waypoint Arrival"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer45->Add( m_cbConfirmBearingChange, 0, wxALL, 5 );

	m_cbInterceptRoute = new wxCheckBox( sbSizer10->GetStaticBox(), wxID_ANY, _("Intercept Route\nOn Current Course"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer45->Add( m_cbInterceptRoute, 0, wxALL, 5 );


	sbSizer10->Add( fgSizer45, 1, wxEXPAND, 5 );


	fgSizer181->Add( sbSizer10, 1, wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer21;
	fgSizer21 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer21->AddGrowableCol( 1 );
	fgSizer21->SetFlexibleDirection( wxBOTH );
	fgSizer21->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();

	fgSizer21->Add( m_sdbSizer1, 1, wxALIGN_RIGHT|wxEXPAND, 5 );


	fgSizer181->Add( fgSizer21, 1, wxEXPAND, 5 );


	fgSizer8->Add( fgSizer181, 1, wxEXPAND, 5 );


	this->SetSizer( fgSizer8 );
	this->Layout();
	fgSizer8->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	m_sdbSizer1OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnOk ), NULL, this );
}

PreferencesDialogBase::~PreferencesDialogBase()
{
	// Disconnect Events
	m_sdbSizer1OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PreferencesDialogBase::OnOk ), NULL, this );

}
