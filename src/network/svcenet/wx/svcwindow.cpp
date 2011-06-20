/*
 * svcwindow.cpp
 *
 *  Created on: 20-06-2011
 *      Author: przemek
 */

#include "svcwindow.hpp"

SVCWindow::SVCWindow() :
			wxFrame(NULL, wxID_ANY, wxT("wxSVC"), wxDefaultPosition,
					wxSize(800, 600)) {
	frameManager = new wxAuiManager(this);

	// content notebook
	contentNotebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
			wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS);

	// human readable log panel and command input
	logPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	logOutput = new wxTextCtrl(logPanel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,
			wxTE_RICH
			| wxTE_MULTILINE
			| wxTE_NOHIDESEL
			| wxTE_READONLY);
	logOutput->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

	wxStaticText* commandLabel = new wxStaticText(logPanel, wxID_ANY, wxT("Command: "));
	input = new wxTextCtrl(logPanel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	input->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

	wxBoxSizer *sizer1, *sizer2;
	sizer1 = new wxBoxSizer(wxVERTICAL);
	sizer1->Add(logOutput, 1, wxEXPAND);

	sizer2 = new wxBoxSizer(wxHORIZONTAL);
	sizer2->Add(commandLabel, 0, wxALIGN_CENTER_VERTICAL);
	sizer2->Add(input, 1, wxEXPAND);

	sizer1->Add(sizer2, 0, wxEXPAND);
	logPanel->SetSizer(sizer1);


	// raw SVC I/O panel
	ioOutput = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,
			wxTE_RICH
			| wxTE_MULTILINE
			| wxTE_NOHIDESEL
			| wxTE_READONLY);
	ioOutput->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

	contentNotebook->AddPage(logPanel, wxT("Console"), true);
	contentNotebook->AddPage(ioOutput, wxT("SVCC log"), false);

	channels = new wxTreeCtrl(this, wxID_ANY);

	wxAuiPaneInfo contentInfo, channelsInfo;

	contentInfo.MinSize(wxSize(200, 200));

	contentInfo.Name(wxT("content"));
	contentInfo.Caption(wxT("Content"));
	contentInfo.CentrePane();

	channelsInfo.Name(wxT("channels"));
	channelsInfo.Caption(wxT("Channels"));
	channelsInfo.MinSize(wxSize(180, 200));
	channelsInfo.Right();

	frameManager->AddPane(contentNotebook, contentInfo);
	frameManager->AddPane(channels, channelsInfo);
	frameManager->Update();
}

SVCWindow::~SVCWindow() {
	frameManager->UnInit();
}

BEGIN_EVENT_TABLE(SVCWindow, wxFrame)
END_EVENT_TABLE()

