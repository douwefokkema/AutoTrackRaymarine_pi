/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Console Canvas
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 *
 *
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/aui/aui.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "wx/datetime.h"

#include "concanv.h"


enum eMenuItems {
    ID_DEACTIVATE,
    ID_PREFERENCES,
    ID_UNDOCK
} menuItems;

//------------------------------------------------------------------------------
//    ConsoleCanvas Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ConsoleCanvas, wxWindow)
    EVT_PAINT(ConsoleCanvas::OnPaint)
    EVT_SHOW(ConsoleCanvas::OnShow)
    EVT_CONTEXT_MENU(ConsoleCanvas::OnContextMenu)
    EVT_MENU(ID_DEACTIVATE, ConsoleCanvas::OnContextMenuSelection)
    EVT_MENU(ID_PREFERENCES, ConsoleCanvas::OnContextMenuSelection)
    EVT_MENU(ID_UNDOCK, ConsoleCanvas::OnContextMenuSelection)
    EVT_BUTTON(ID_DEACTIVATE, ConsoleCanvas::OnContextMenuSelection)
    
END_EVENT_TABLE()

// Define a constructor for my canvas
ConsoleCanvas::ConsoleCanvas( wxWindow *frame, AutoTrackRaymarine_pi &pi )
:         wxWindow( frame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE,
                    _T("Dashboard") ), m_pi(pi)
{
    page = 0;
    pbackBrush = NULL;
    m_bNeedClear = false;

//    long style = wxBORDER_NONE;//0;//wxSIMPLE_BORDER | wxCLIP_CHILDREN;
//#ifdef __WXOSX__
//    style |= wxSTAY_ON_TOP;
//#endif

//    wxDialog::Create( frame, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, style );
    
    m_pParent = frame;

    m_pitemBoxSizerLeg = new wxBoxSizer( wxVERTICAL );

    pXTE = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pXTE->SetALabel( _T("XTE") );
    m_pitemBoxSizerLeg->Add( pXTE, 1, wxALIGN_LEFT | wxALL, 2 );

    pBRG = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pBRG->SetALabel( _T("BRG") );
    m_pitemBoxSizerLeg->Add( pBRG, 1, wxALIGN_LEFT | wxALL, 2 );

    pRNG = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pRNG->SetALabel( _T("RNG") );
    m_pitemBoxSizerLeg->Add( pRNG, 1, wxALIGN_LEFT | wxALL, 2 );

    pTRNG = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pTRNG->SetALabel( _T("Route RNG") );
    m_pitemBoxSizerLeg->Add( pTRNG, 1, wxALIGN_LEFT | wxALL, 2 );

    pVMG = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pVMG->SetALabel( _T("VMG") );
    m_pitemBoxSizerLeg->Add( pVMG, 1, wxALIGN_LEFT | wxALL, 2 );

    pETA = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pETA->SetALabel( _T("Route ETA") );
    m_pitemBoxSizerLeg->Add( pETA, 1, wxALIGN_LEFT | wxALL, 2 );

    pTTTG = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pTTTG->SetALabel( _T("Route TTG") );
    m_pitemBoxSizerLeg->Add( pTTTG, 1, wxALIGN_LEFT | wxALL, 2 );

    pTTG = new AnnunText( this, -1, _("Console Legend"), _("Console Value") );
    pTTG->SetALabel( _T("TTG") );
    m_pitemBoxSizerLeg->Add( pTTG, 1, wxALIGN_LEFT | wxALL, 2 );

    //    Create CDI Display Window
    pCDI = new CDI( this, -1, wxSIMPLE_BORDER, _T("CDI") );
    m_pitemBoxSizerLeg->AddSpacer( 5 );
    m_pitemBoxSizerLeg->Add( pCDI, 0, wxALL | wxEXPAND, 2 );

    // Deactivate button
    pDeactivate = new wxButton( this, ID_DEACTIVATE, _("Passive") );
    m_pitemBoxSizerLeg->Add( pDeactivate, 0, wxALL | wxEXPAND, 2 );

    SetSizer( m_pitemBoxSizerLeg );      // use the sizer for layout
    m_pitemBoxSizerLeg->SetSizeHints( this );
    Layout();
    Fit();
    
//    Hide();

    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath ( _T( "/Settings/AutoTrackRaymarine" ) );
#ifdef __WXGTK__
//    Move(0, 0);        // workaround for gtk autocentre dialog behavior
#endif
//    Move(pConf->Read ( _T ( "ConsolePosX" ), 20L ), pConf->Read ( _T ( "ConsolePosY" ), 20L ));
}

ConsoleCanvas::~ConsoleCanvas()
{
    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath ( _T( "/Settings/AutoTrackRaymarine" ) );
    wxPoint p = GetPosition();
    pConf->Write ( _T ( "ConsolePosX" ), p.x );
    pConf->Write ( _T ( "ConsolePosY" ), p.y );
    delete pCDI;
}
    
void ConsoleCanvas::SetColorScheme( PI_ColorScheme cs )
{
    pbackBrush = wxTheBrushList->FindOrCreateBrush( *wxBLACK,
            wxBRUSHSTYLE_SOLID );
    SetBackgroundColour( *wxBLACK );

    //  Also apply color scheme to all known children

    pXTE->SetColorScheme( cs );
    pBRG->SetColorScheme( cs );
    pRNG->SetColorScheme( cs );
    pTTG->SetColorScheme( cs );
    pVMG->SetColorScheme( cs );
    pETA->SetColorScheme( cs );
    pTRNG->SetColorScheme( cs );
    pTTTG->SetColorScheme( cs );
    pCDI->SetColorScheme( cs );
}

void ConsoleCanvas::OnPaint( wxPaintEvent& event )
{
    /*wxPaintDC dc( this );

#if 0    
    if( g_pRouteMan->GetpActiveRoute() ) {
        if( m_bNeedClear ) {
            pThisLegText->Refresh();
            m_bNeedClear = false;
        }

        UpdateRouteData();
    }
#endif */   
}

void ConsoleCanvas::OnShow( wxShowEvent& event )
{
    /*pXTE->Show( m_pi.prefs.ActiveRouteLabel(page, "XTE") );
    pBRG->Show( m_pi.prefs.ActiveRouteLabel(page, "BRG") );
    pRNG->Show( m_pi.prefs.ActiveRouteLabel(page, "RNG") );
    pTTG->Show( m_pi.prefs.ActiveRouteLabel(page, "TTG") );
    pVMG->Show( m_pi.prefs.ActiveRouteLabel(page, "VMG") );
    pETA->Show( m_pi.prefs.ActiveRouteLabel(page, "Route ETA") );
    pTRNG->Show(m_pi.prefs.ActiveRouteLabel(page, "Route RNG") );
    pTTTG->Show(m_pi.prefs.ActiveRouteLabel(page, "Route TTG") );
    pCDI->Show( m_pi.prefs.ActiveRouteLabel(page, "Highway") );
    pDeactivate->Show( m_pi.prefs.ActiveRouteLabel(page, "Deactivate") );

    m_pitemBoxSizerLeg->SetSizeHints( this );*/
}

void ConsoleCanvas::OnContextMenu( wxContextMenuEvent& event ) {
    wxMenu* contextMenu = new wxMenu();
    wxMenuItem* btnDeactivate = new wxMenuItem(contextMenu, ID_DEACTIVATE, _("Deactivate"), _T("") );
    wxMenuItem* btnPreferences = new wxMenuItem(contextMenu, ID_PREFERENCES, _("Preferences"), _T("") );
    wxMenuItem* btnUndock = new wxMenuItem(contextMenu, ID_UNDOCK, _("Undock"), _T("") );
    contextMenu->Append( btnDeactivate );
    contextMenu->Append( btnPreferences );
    contextMenu->Append( btnUndock );

    PopupMenu( contextMenu );

    delete contextMenu;
}

void ConsoleCanvas::OnContextMenuSelection( wxCommandEvent& event ) {
    switch( event.GetId() ) {
        case ID_DEACTIVATE:
            m_pi.DeactivateRoute();
            break;
        case ID_PREFERENCES:
            m_pi.ShowPreferences();
            break;
        case ID_UNDOCK:
            GetFrameAuiManager()->GetPane(this).Float();
            GetFrameAuiManager()->Update();
            break;
    }
}

void ConsoleCanvas::UpdateRouteData()
{
    wxString str_buf;
    double sog, cog, brg, xte, rng, nrng;
    if(!m_pi.GetConsoleInfo(brg, xte)){
      return;
    }

    if (m_pi.m_pilot_state == TRACKING)
    pDeactivate->SetLabel(_("Tracking"));
    if (m_pi.m_pilot_state == AUTO)
      pDeactivate->SetLabel(_("Auto"));
    if (m_pi.m_pilot_state == STANDBY)
      pDeactivate->SetLabel(_("Standby"));

    wxString cogstr;
//            if( g_bShowTrue )
    cogstr << wxString::Format( wxString("%6.0f", wxConvUTF8 ), brg );
//            if( g_bShowMag )
//                cogstr << wxString::Format( wxString("%6.0f(M)", wxConvUTF8 ), gFrame->GetMag( dcog ) );
    pBRG->SetAValue( cogstr );

    // VMG
    // VMG is always to next waypoint, not to end of route
    // VMG is SOG x cosine (difference between COG and BRG to Waypoint)
    double VMG = 0.;
    if( !wxIsNaN(cog) && !wxIsNaN(sog) )
    {
        VMG = sog * cos( ( brg - cog ) * M_PI / 180. ) ;
        str_buf.Printf( _T("%6.2f"), toUsrSpeed_Plugin( VMG ) );
    }
    else
        str_buf = _T("---");

    pVMG->SetAValue( str_buf );
        
    wxString srng;
    double deltarng = fabs( rng - nrng );
    // show if there is more than 10% difference in ranges, etc...        
    if( ( deltarng > .01 ) && ( ( deltarng / rng ) > .10 ) && ( rng < 10.0 ) ) {
        if( nrng < 10.0 )
            srng.Printf( _T("%5.2f/%5.2f"), toUsrDistance_Plugin( rng ), toUsrDistance_Plugin( nrng ) );
        else
            srng.Printf( _T("%5.1f/%5.1f"), toUsrDistance_Plugin( rng ), toUsrDistance_Plugin( nrng ) );
    } else {
        if( rng < 10.0 )
            srng.Printf( _T("%6.2f"), toUsrDistance_Plugin( rng ) );
        else
            srng.Printf( _T("%6.1f"), toUsrDistance_Plugin( rng ) );
    }
            
    //RNG to the next WPT
    pRNG->SetAValue( srng );
    // XTE
    str_buf.Printf( _T("%6.2f"), 1852. * (toUsrDistance_Plugin( fabs(xte))));
    pXTE->SetAValue( str_buf );
    if( xte < 0 )
        pXTE->SetALabel( "XTE         L" );
    else
        pXTE->SetALabel( "XTE         R" );
    
    // TTG
    // In all cases, ttg/eta are declared invalid if VMG <= 0.
    // If showing only "this leg", use VMG for calculation of ttg
    wxString ttg_s;
    if( ( VMG > 0. ) && !wxIsNaN(cog) && !wxIsNaN(sog) ) {
        float ttg_sec = ( rng / VMG ) * 3600.;
        wxTimeSpan ttg_span( 0, 0, long( ttg_sec ), 0 );
        ttg_s = ttg_span.Format();
    } else
        ttg_s = _T("---");
    
    pTTG->SetAValue( ttg_s );

    //    Remainder of route
    float trng = rng;

   // ap_route &rt = m_pi.m_route;
    int n_addflag = 0;
    double lat0, lon0;
   /* for(ap_route_iterator it = rt.begin(); it != rt.end(); it++) {
        waypoint &wp = *it;
        if( n_addflag ) {
            double dist;
            ll_gc_ll_reverse(lat0, lon0, wp.lat, wp.lon, 0, &dist);
            trng += dist;
        }
        lat0 = wp.lat, lon0 = wp.lon;

        if( wp.GUID == m_pi.m_next_route_wp_GUID)
            n_addflag=1;
    }*/

    //                total rng
    wxString strng;
    if( trng < 10.0 )
        strng.Printf( _T("%6.2f"), toUsrDistance_Plugin( trng ) );
    else
        strng.Printf( _T("%6.1f"), toUsrDistance_Plugin( trng ) );

    pTRNG->SetAValue( strng );

    // total TTG
    // If showing total route TTG/ETA, use gSog for calculation

    wxString tttg_s;
    wxTimeSpan tttg_span;
    float tttg_sec;
    if( VMG > 0. ) {
        tttg_sec = ( trng / sog ) * 3600.;
        tttg_span = wxTimeSpan::Seconds( (long) tttg_sec );
        //Show also #days if TTG > 24 h
        tttg_s = tttg_sec > SECONDS_PER_DAY ? 
            tttg_span.Format(_("%Dd %H:%M")) : tttg_span.Format("%H:%M:%S");
    } else {
        tttg_span = wxTimeSpan::Seconds( 0 );
        tttg_s = _T("---");
    }

    pTTTG->SetAValue( tttg_s );

    //                total ETA to be shown on XTE panel
    wxDateTime dtnow, eta;
    dtnow.SetToCurrent();
    eta = dtnow.Add( tttg_span );
    wxString seta;

    if (VMG > 0.) {
        // Show date, e.g. Feb 15, if TTG > 24 h
        seta = tttg_sec > SECONDS_PER_DAY ?
            eta.Format(_T("%b %d %H:%M")) : eta.Format(_T("%H:%M"));
    } else {
        seta = _T("---");
    }
    pETA->SetAValue( seta );

    pXTE->Refresh();
    pBRG->Refresh();
    pRNG->Refresh();
    pTTG->Refresh();
    pVMG->Refresh();
    pETA->Refresh();
    pTRNG->Refresh();
    pTTTG->Refresh();
    pCDI->Refresh();
}

void ConsoleCanvas::ShowWithFreshFonts( void )
{
    Hide();
    UpdateFonts();
    Show();

    wxSize sz = GetSize();
    sz.y += 16; // hack for lack of better way to determine tilebar
    GetFrameAuiManager()->GetPane(this).
        FloatingSize(sz);
    GetFrameAuiManager()->Update();
}

void ConsoleCanvas::UpdateFonts( void )
{
    pXTE->RefreshFonts();
    pBRG->RefreshFonts();
    pRNG->RefreshFonts();
    pTTG->RefreshFonts();
    pVMG->RefreshFonts();
    pETA->RefreshFonts();
    pTRNG->RefreshFonts();
    pTTTG->RefreshFonts();

    m_pitemBoxSizerLeg->SetSizeHints( this );
    Layout();
    Fit();

    Refresh();
}

//------------------------------------------------------------------------------
//    AnnunText Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(AnnunText, wxWindow)
EVT_PAINT(AnnunText::OnPaint)
EVT_MOUSE_EVENTS ( AnnunText::MouseEvent )
END_EVENT_TABLE()

AnnunText::AnnunText( wxWindow *parent, wxWindowID id, const wxString& LegendElement,
        const wxString& ValueElement ) :
        wxWindow( parent, id, wxDefaultPosition, wxDefaultSize, wxNO_BORDER )
{
    m_label = _T("Label");
    m_value = _T("-----");

    m_labelFont = wxFont(14, wxDEFAULT, wxNORMAL, wxNORMAL);
    m_valueFont = wxFont(24, wxDEFAULT, wxNORMAL, wxNORMAL);

    m_LegendTextElement = LegendElement;
    m_ValueTextElement = ValueElement;

    RefreshFonts();
}

AnnunText::~AnnunText()
{
}
void AnnunText::MouseEvent( wxMouseEvent& event )
{
    if( event.RightDown() ) {
        wxContextMenuEvent cevt;
        cevt.SetPosition( event.GetPosition());
        
        ConsoleCanvas *ccp = dynamic_cast<ConsoleCanvas*>(GetParent());
        if(ccp)
            ccp->OnContextMenu( cevt );
        
    }
    else if( event.LeftDown() ) {
        ConsoleCanvas *ccp = dynamic_cast<ConsoleCanvas*>(GetParent());
        if(ccp){
            ccp->page = !ccp->page;
            ccp->ShowWithFreshFonts();
        }
    }
    
}

void AnnunText::CalculateMinSize( void )
{
    //    Calculate the minimum required size of the window based on text size

    int wl = 50;            // reasonable defaults?
    int hl = 20;
    int wv = 50;
    int hv = 20;

    GetTextExtent( _T("1234"), &wl, &hl, NULL, NULL, &m_labelFont );
    GetTextExtent( _T("123.456"), &wv, &hv, NULL, NULL, &m_valueFont );

    wxSize min;
    min.x = wl + wv;
    
    // Space is tight on Android....
#ifdef __OCPN__ANDROID__
    min.x = wv * 1.2; 
#endif    
    
    min.y = (int) ( ( hl + hv ) * 1.2 );

    SetMinSize( min );
}

void AnnunText::SetColorScheme( PI_ColorScheme cs )
{
    m_backBrush = *wxTheBrushList->FindOrCreateBrush( *wxBLACK, wxBRUSHSTYLE_SOLID );

    m_default_text_color = *wxBLACK;
    RefreshFonts();
}

void AnnunText::RefreshFonts()
{
//    m_plabelFont = FontMgr::Get().GetFont( m_LegendTextElement );
//    m_pvalueFont = FontMgr::Get().GetFont( m_ValueTextElement );

//    m_legend_color = FontMgr::Get().GetFontColor( _("Console Legend") );
//    m_val_color = FontMgr::Get().GetFontColor( _("Console Value") );
    m_legend_color = *wxWHITE;
    m_val_color = *wxWHITE;
    
    CalculateMinSize();
    
    // Make sure that the background color and the text colors are not too close, for contrast
    if(m_backBrush.IsOk()){
        wxColour back_color = m_backBrush.GetColour();
    
        wxColour legend_color = m_legend_color;
        if( (abs(legend_color.Red() - back_color.Red()) < 5) &&
                (abs(legend_color.Green() - back_color.Blue()) < 5) &&
                (abs(legend_color.Blue() - back_color.Blue()) < 5))
            m_legend_color = m_default_text_color;
            
        wxColour value_color = m_val_color;
        if( (abs(value_color.Red() - back_color.Red()) < 5) &&
            (abs(value_color.Green() - back_color.Blue()) < 5) &&
            (abs(value_color.Blue() - back_color.Blue()) < 5))
            m_val_color = m_default_text_color;
            
    }
}

void AnnunText::SetLegendElement( const wxString &element )
{
    m_LegendTextElement = element;
}

void AnnunText::SetValueElement( const wxString &element )
{
    m_ValueTextElement = element;
}

void AnnunText::SetALabel( const wxString &l )
{
    m_label = l;
}

void AnnunText::SetAValue( const wxString &v )
{
    m_value = v;
}

void AnnunText::OnPaint( wxPaintEvent& event )
{
    int sx, sy;
    GetClientSize( &sx, &sy );

    //    Do the drawing on an off-screen memory DC, and blit into place
    //    to avoid objectionable flashing
    wxMemoryDC mdc;

    wxBitmap m_bitmap( sx, sy, -1 );
    mdc.SelectObject( m_bitmap );
    mdc.SetBackground( m_backBrush );
    mdc.Clear();

//    if( style->consoleTextBackground.IsOk() ) mdc.DrawBitmap( style->consoleTextBackground, 0, 0 );

    mdc.SetTextForeground( m_default_text_color );

    mdc.SetFont( m_labelFont );
    mdc.SetTextForeground( m_legend_color );
    mdc.DrawText( m_label, 5, 2 );

    mdc.SetFont( m_valueFont );
    mdc.SetTextForeground( m_val_color );

    int w, h;
    mdc.GetTextExtent( m_value, &w, &h );
    int cw, ch;
    mdc.GetSize( &cw, &ch );

    mdc.DrawText( m_value, cw - w - 2, ch - h - 2 );

    wxPaintDC dc( this );
    dc.Blit( 0, 0, sx, sy, &mdc, 0, 0 );
    
}
//------------------------------------------------------------------------------
//    CDI Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(CDI, wxWindow)
EVT_PAINT(CDI::OnPaint)
EVT_MOUSE_EVENTS ( CDI::MouseEvent )
END_EVENT_TABLE()

CDI::CDI( wxWindow *parent, wxWindowID id, long style, const wxString& name ) :
        wxWindow( parent, id, wxDefaultPosition, wxDefaultSize, style, name )

{
    SetMinSize( wxSize( 10, 150 ) );
}

void CDI::MouseEvent( wxMouseEvent& event )
{
    if( event.RightDown() ) {
#ifdef    __OCPN__ANDROID__
        qDebug() << "right down";
         
        wxContextMenuEvent cevt;
        cevt.SetPosition( event.GetPosition());
        
        ConsoleCanvas *ccp = dynamic_cast<ConsoleCanvas*>(GetParent());
        if(ccp)
            ccp->OnContextMenu( cevt );
        
#endif    
    }
    else if( event.LeftDown() ) {
        ConsoleCanvas *ccp = dynamic_cast<ConsoleCanvas*>(GetParent());
        if(ccp){
            ccp->page = !ccp->page;
            ccp->ShowWithFreshFonts();
        }
    }
}

void CDI::SetColorScheme( PI_ColorScheme cs )
{
    wxColour c;
    GetGlobalColor( _T("DILG2"), &c );
    m_pbackBrush = wxTheBrushList->FindOrCreateBrush( c, wxBRUSHSTYLE_SOLID );
    GetGlobalColor( _T("DILG1"), &c );
    m_proadBrush = wxTheBrushList->FindOrCreateBrush( c, wxBRUSHSTYLE_SOLID );
    GetGlobalColor( _T("CHBLK"), &c );
    m_proadPen = wxThePenList->FindOrCreatePen( c, 1, wxPENSTYLE_SOLID );
}

void CDI::OnPaint( wxPaintEvent& event )
{
    
}

