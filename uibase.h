// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.
///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __uibase__
#define __uibase__

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/toolbar.h>
#include <wx/statusbr.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/listctrl.h>
#include <wx/frame.h>
#include <wx/html/htmlwin.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/scrolwin.h>
#include <wx/statbmp.h>

///////////////////////////////////////////////////////////////////////////

#define wxID_MAINFRAME 1000
#define wxID_VIEWSHOWGENERATED 1001
#define wxID_OPTIONSGENERATEBITCOINS 1002
#define wxID_MENUOPTIONSOPTIONS 1003
#define wxID_BUTTONSEND 1004
#define wxID_BUTTONRECEIVE 1005
#define wxID_TEXTCTRLADDRESS 1006
#define wxID_BUTTONCHANGE 1007
#define wxID_BUTTONCOPY 1008
#define wxID_TRANSACTIONFEE 1009
#define wxID_PROXYIP 1010
#define wxID_PROXYPORT 1011
#define wxID_TEXTCTRLPAYTO 1012
#define wxID_BUTTONPASTE 1013
#define wxID_BUTTONADDRESSBOOK 1014
#define wxID_TEXTCTRLAMOUNT 1015
#define wxID_CHOICETRANSFERTYPE 1016
#define wxID_LISTCTRL 1017
#define wxID_BUTTONRENAME 1018
#define wxID_BUTTONNEW 1019
#define wxID_BUTTONEDIT 1020
#define wxID_BUTTONDELETE 1021
#define wxID_TEXTCTRL 1022

///////////////////////////////////////////////////////////////////////////////
/// Class CMainFrameBase
///////////////////////////////////////////////////////////////////////////////
class CMainFrameBase : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* m_menubar;
		wxMenu* m_menuFile;
		wxMenu* m_menuView;
		wxMenu* m_menuHelp;
		wxToolBar* m_toolBar;
		wxStatusBar* m_statusBar;
		
		wxStaticText* m_staticText32;
		wxTextCtrl* m_textCtrlAddress;
		wxButton* m_buttonNew;
		wxButton* m_buttonCopy;
		
		wxPanel* m_panel14;
		wxStaticText* m_staticText41;
		wxStaticText* m_staticTextBalance;
		
		wxChoice* m_choiceFilter;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ){ event.Skip(); }
		virtual void OnIconize( wxIconizeEvent& event ){ event.Skip(); }
		virtual void OnIdle( wxIdleEvent& event ){ event.Skip(); }
		virtual void OnMouseEvents( wxMouseEvent& event ){ event.Skip(); }
		virtual void OnPaint( wxPaintEvent& event ){ event.Skip(); }
		virtual void OnMenuFileExit( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMenuViewShowGenerated( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnUpdateUIViewShowGenerated( wxUpdateUIEvent& event ){ event.Skip(); }
		virtual void OnMenuOptionsGenerate( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnUpdateUIOptionsGenerate( wxUpdateUIEvent& event ){ event.Skip(); }
		virtual void OnMenuOptionsChangeYourAddress( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMenuOptionsOptions( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMenuHelpAbout( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonSend( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonAddressBook( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnKeyDown( wxKeyEvent& event ){ event.Skip(); }
		virtual void OnMouseEventsAddress( wxMouseEvent& event ){ event.Skip(); }
		virtual void OnSetFocusAddress( wxFocusEvent& event ){ event.Skip(); }
		virtual void OnButtonChange( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonCopy( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnListColBeginDrag( wxListEvent& event ){ event.Skip(); }
		virtual void OnListItemActivated( wxListEvent& event ){ event.Skip(); }
		virtual void OnPaintListCtrl( wxPaintEvent& event ){ event.Skip(); }
		
	
	public:
		wxMenu* m_menuOptions;
		wxListCtrl* m_listCtrl;
		CMainFrameBase( wxWindow* parent, wxWindowID id = wxID_MAINFRAME, const wxString& title = _("Bitcoin"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 712,484 ), long style = wxDEFAULT_FRAME_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL );
		~CMainFrameBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CTxDetailsDialogBase
///////////////////////////////////////////////////////////////////////////////
class CTxDetailsDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxHtmlWindow* m_htmlWin;
		wxButton* m_buttonOK;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnButtonOK( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		CTxDetailsDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Transaction Details"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 620,450 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		~CTxDetailsDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class COptionsDialogBase
///////////////////////////////////////////////////////////////////////////////
class COptionsDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxListBox* m_listBox;
		wxScrolledWindow* m_scrolledWindow;
		wxPanel* m_panelMain;
		
		wxStaticText* m_staticText32;
		wxStaticText* m_staticText31;
		wxTextCtrl* m_textCtrlTransactionFee;
		wxCheckBox* m_checkBoxLimitProcessors;
		wxSpinCtrl* m_spinCtrlLimitProcessors;
		wxStaticText* m_staticText35;
		wxCheckBox* m_checkBoxStartOnSystemStartup;
		wxCheckBox* m_checkBoxMinimizeToTray;
		wxCheckBox* m_checkBoxMinimizeOnClose;
		wxCheckBox* m_checkBoxUseProxy;
		
		wxStaticText* m_staticTextProxyIP;
		wxTextCtrl* m_textCtrlProxyIP;
		wxStaticText* m_staticTextProxyPort;
		wxTextCtrl* m_textCtrlProxyPort;
		wxPanel* m_panelTest2;
		
		wxStaticText* m_staticText321;
		wxStaticText* m_staticText69;
		wxStaticText* m_staticText70;
		wxStaticText* m_staticText71;
		wxButton* m_buttonOK;
		wxButton* m_buttonCancel;
		wxButton* m_buttonApply;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnListBox( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnKillFocusTransactionFee( wxFocusEvent& event ){ event.Skip(); }
		virtual void OnCheckBoxLimitProcessors( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnCheckBoxMinimizeToTray( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnCheckBoxUseProxy( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnKillFocusProxy( wxFocusEvent& event ){ event.Skip(); }
		virtual void OnButtonOK( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonCancel( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonApply( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		COptionsDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Options"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 540,360 ), long style = wxDEFAULT_DIALOG_STYLE );
		~COptionsDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CAboutDialogBase
///////////////////////////////////////////////////////////////////////////////
class CAboutDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticBitmap* m_bitmap;
		
		wxStaticText* m_staticText40;
		
		wxStaticText* m_staticTextMain;
		
		
		wxButton* m_buttonOK;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnButtonOK( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		wxStaticText* m_staticTextVersion;
		CAboutDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("About Bitcoin"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 532,329 ), long style = wxDEFAULT_DIALOG_STYLE );
		~CAboutDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CSendDialogBase
///////////////////////////////////////////////////////////////////////////////
class CSendDialogBase : public wxDialog 
{
	private:
	
	protected:
		
		
		wxStaticText* m_staticTextInstructions;
		
		wxStaticBitmap* m_bitmapCheckMark;
		wxStaticText* m_staticText36;
		wxTextCtrl* m_textCtrlAddress;
		wxButton* m_buttonPaste;
		wxButton* m_buttonAddress;
		wxStaticText* m_staticText19;
		wxTextCtrl* m_textCtrlAmount;
		wxStaticText* m_staticText20;
		wxChoice* m_choiceTransferType;
		
		
		wxStaticText* m_staticTextFrom;
		wxTextCtrl* m_textCtrlFrom;
		wxStaticText* m_staticTextMessage;
		wxTextCtrl* m_textCtrlMessage;
		
		wxButton* m_buttonSend;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnKeyDown( wxKeyEvent& event ){ event.Skip(); }
		virtual void OnTextAddress( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonPaste( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonAddressBook( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnKillFocusAmount( wxFocusEvent& event ){ event.Skip(); }
		virtual void OnButtonSend( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonCancel( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		CSendDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Send Coins"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 675,312 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		~CSendDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CSendingDialogBase
///////////////////////////////////////////////////////////////////////////////
class CSendingDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticTextSending;
		wxTextCtrl* m_textCtrlStatus;
		
		wxButton* m_buttonOK;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ){ event.Skip(); }
		virtual void OnPaint( wxPaintEvent& event ){ event.Skip(); }
		virtual void OnButtonOK( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonCancel( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		CSendingDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Sending..."), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 442,151 ), long style = wxDEFAULT_DIALOG_STYLE );
		~CSendingDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CYourAddressDialogBase
///////////////////////////////////////////////////////////////////////////////
class CYourAddressDialogBase : public wxDialog 
{
	private:
	
	protected:
		
		wxStaticText* m_staticText45;
		wxListCtrl* m_listCtrl;
		
		wxButton* m_buttonRename;
		wxButton* m_buttonNew;
		wxButton* m_buttonCopy;
		wxButton* m_buttonOK;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ){ event.Skip(); }
		virtual void OnListEndLabelEdit( wxListEvent& event ){ event.Skip(); }
		virtual void OnListItemActivated( wxListEvent& event ){ event.Skip(); }
		virtual void OnListItemSelected( wxListEvent& event ){ event.Skip(); }
		virtual void OnButtonRename( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonNew( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonCopy( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonOK( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonCancel( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		CYourAddressDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Your Bitcoin Addresses"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 610,390 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		~CYourAddressDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CAddressBookDialogBase
///////////////////////////////////////////////////////////////////////////////
class CAddressBookDialogBase : public wxDialog 
{
	private:
	
	protected:
		
		wxStaticText* m_staticText55;
		wxListCtrl* m_listCtrl;
		
		wxButton* m_buttonEdit;
		wxButton* m_buttonNew;
		wxButton* m_buttonDelete;
		wxButton* m_buttonOK;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ){ event.Skip(); }
		virtual void OnListEndLabelEdit( wxListEvent& event ){ event.Skip(); }
		virtual void OnListItemActivated( wxListEvent& event ){ event.Skip(); }
		virtual void OnListItemSelected( wxListEvent& event ){ event.Skip(); }
		virtual void OnButtonEdit( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonNew( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonDelete( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonOK( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonCancel( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		wxButton* m_buttonCancel;
		CAddressBookDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Address Book"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 610,390 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		~CAddressBookDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class CGetTextFromUserDialogBase
///////////////////////////////////////////////////////////////////////////////
class CGetTextFromUserDialogBase : public wxDialog 
{
	private:
	
	protected:
		
		wxStaticText* m_staticTextMessage1;
		wxTextCtrl* m_textCtrl1;
		wxStaticText* m_staticTextMessage2;
		wxTextCtrl* m_textCtrl2;
		
		
		wxButton* m_buttonOK;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ){ event.Skip(); }
		virtual void OnKeyDown( wxKeyEvent& event ){ event.Skip(); }
		virtual void OnButtonOK( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonCancel( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		CGetTextFromUserDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 403,138 ), long style = wxDEFAULT_DIALOG_STYLE );
		~CGetTextFromUserDialogBase();
	
};

#endif //__uibase__
