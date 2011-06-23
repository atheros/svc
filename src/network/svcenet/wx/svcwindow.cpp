/*
 * svcwindow.cpp
 *
 *  Created on: 20-06-2011
 *      Author: przemek
 */

#include "svcwindow.hpp"
#include "app.hpp"


BEGIN_EVENT_TABLE(SVCWindow, wxFrame)
	EVT_TIMER(ID_IOTIMER, SVCWindow::OnIOTimer) EVT_COMMAND(ID_COMMAND_INPUT, wxEVT_COMMAND_TEXT_ENTER, SVCWindow::OnCommand)
END_EVENT_TABLE()

SVCWindow::SVCWindow(SVCApp* app) :
	wxFrame(NULL, wxID_ANY, wxT("wxSVC"), wxDefaultPosition, wxSize(800, 600)) {

	this->app = app;

	frameManager = new wxAuiManager(this);

	// status bar setup
	int widths[__STATUS_COUNT] = {-1};
	statusBar = new wxStatusBar(this, wxID_ANY);
	statusBar->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	statusBar->SetFieldsCount(__STATUS_COUNT, widths);
	statusBar->SetStatusText(wxT("svcc not running"), STATUS_SVCC);
	SetStatusBar(statusBar);


	// content notebook
	contentNotebook = new wxAuiNotebook(
			this,
			wxID_ANY,
			wxDefaultPosition,
			wxDefaultSize,
			wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE
					| wxAUI_NB_SCROLL_BUTTONS);

	// human readable log panel and command input
	logPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	logOutput = new wxTextCtrl(logPanel, wxID_ANY, wxT(""), wxDefaultPosition,
			wxDefaultSize,
			wxTE_RICH | wxTE_MULTILINE | wxTE_NOHIDESEL | wxTE_READONLY);
	logOutput->SetFont(
			wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
					wxFONTWEIGHT_NORMAL));

	wxStaticText* commandLabel = new wxStaticText(logPanel, wxID_ANY,
			wxT("  Command: "));
	input = new wxTextCtrl(logPanel, ID_COMMAND_INPUT, wxT(""),
			wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	input->SetFont(
			wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
					wxFONTWEIGHT_NORMAL));

	wxBoxSizer *sizer1, *sizer2;
	sizer1 = new wxBoxSizer(wxVERTICAL);
	sizer1->Add(logOutput, 1, wxEXPAND);

	sizer2 = new wxBoxSizer(wxHORIZONTAL);
	sizer2->Add(commandLabel, 0, wxALIGN_CENTER_VERTICAL);
	sizer2->Add(input, 1, wxEXPAND);

	sizer1->Add(sizer2, 0, wxEXPAND);
	logPanel->SetSizer(sizer1);

	// raw SVC I/O panel
	ioOutput = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition,
			wxDefaultSize,
			wxTE_RICH | wxTE_MULTILINE | wxTE_NOHIDESEL | wxTE_READONLY);
	ioOutput->SetFont(
			wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
					wxFONTWEIGHT_NORMAL));

	// peer info panel
	peerInfo = new SVCObjectInfo(this);

	// server info panel
	serverInfo = new SVCObjectInfo(this);

	// add content panels
	contentNotebook->AddPage(logPanel, wxT("Console"), true);
	contentNotebook->AddPage(ioOutput, wxT("SVCC log"), false);
	contentNotebook->AddPage(peerInfo, wxT("Peer info"), false);
	contentNotebook->AddPage(serverInfo, wxT("Server info"), false);

	// Channels and peers tree
	channels = new wxTreeCtrl(this, wxID_ANY);


	wxAuiPaneInfo contentInfo, channelsInfo;
	// content AUI setup
	contentInfo.Name(wxT("content"));
	contentInfo.Caption(wxT("Content"));
	contentInfo.MinSize(wxSize(300, 200));
	contentInfo.CentrePane();
	// channels AUI setup
	channelsInfo.Name(wxT("channels"));
	channelsInfo.Caption(wxT("Channels"));
	channelsInfo.MinSize(wxSize(180, 200));
	channelsInfo.Right();

	// make AUI
	frameManager->AddPane(contentNotebook, contentInfo);
	frameManager->AddPane(channels, channelsInfo);
	frameManager->Update();

	// initialize state
	stateConnectionState = SVCState::SVCCON_DISCONNECTED;

	// processIO update timer.
	firstStateChange = true;
	ioTimer = new wxTimer(this, ID_IOTIMER);
	ioTimer->Start(100, false);
}

SVCWindow::~SVCWindow() {
	frameManager->UnInit();
	delete frameManager;
}

void SVCWindow::OnIOTimer(wxTimerEvent& event) {
	wxString line, rest;
	app->getState()->processIO();

	while (app->getState()->getSVCOutput(line)) {
		ioStdout(line);
	}

	while (app->getState()->getSVCError(line)) {
		ioStderr(line);
	}

	while (app->getState()->getLog(line)) {
		if (line.StartsWith(wxT("error: "), &rest)) {
			logError(rest);
		} else {
			log(line);
		}
	}

	updateSVCState();
}

void SVCWindow::OnCommand(wxCommandEvent& event) {
	app->sendUserCommand(input->GetValue());
	input->Clear();
}

void SVCWindow::ioStdout(const wxString& text) {
	ioOutput->SetForegroundColour(wxColour(0, 0, 0, 255));
	ioOutput->AppendText(text + wxT("\n"));
}

void SVCWindow::ioStderr(const wxString& text) {
	ioOutput->SetForegroundColour(wxColour(255, 0, 0, 255));
	ioOutput->AppendText(text + wxT("\n"));
}

void SVCWindow::ioStdin(const wxString& text) {
	ioOutput->SetForegroundColour(wxColour(0, 0, 255, 255));
	ioOutput->AppendText(text + wxT("\n"));
}

void SVCWindow::log(const wxString& text) {
	logOutput->SetForegroundColour(wxColour(0, 0, 0, 255));
	logOutput->AppendText(text + wxT("\n"));
}

void SVCWindow::logError(const wxString& text) {
	logOutput->SetForegroundColour(wxColour(255, 0, 0, 255));
	logOutput->AppendText(wxT("error: ") + text + wxT("\n"));
}

void SVCWindow::logSVCError(const wxString& text) {
	logOutput->SetForegroundColour(wxColour(255, 0, 0, 255));
	logOutput->AppendText(wxT("svcc error: ") + text + wxT("\n"));
}

void SVCWindow::logCommand(const wxString& text) {
	logOutput->SetForegroundColour(wxColour(0, 200, 0, 255));
	logOutput->AppendText(wxT("excuting "));
	logOutput->SetForegroundColour(wxColour(0, 0, 255, 255));
	logOutput->AppendText(text + wxT("\n"));
}

void SVCWindow::logConnection(const wxString& text) {
	logOutput->SetForegroundColour(wxColour(255, 160, 0, 255));
	logOutput->AppendText(text + wxT("\n"));
}

void SVCWindow::handleConnectionState(const SVCEvent& event) {
	// update connection and server status
	SVCState::ConnectionState oldState = stateConnectionState;
	stateConnectionState = (SVCState::ConnectionState)event.connectionState;

	wxString status, message;

	// construct status line
	if (stateConnectionState == SVCState::SVCCON_CONNECTED) {
		status
			<< wxT("svcc running - connected to ")
			<< stateServerHost
			<< wxT(" (")
			<< stateServerAddress
			<< wxT(":")
			<< stateServerPort
			<< wxT(")");
		message
			<< wxT("You are now connected to ")
			<< stateServerHost
			<< wxT(" (")
			<< stateServerAddress
			<< wxT(":")
			<< stateServerPort
			<< wxT(")");
	} else if (stateConnectionState == SVCState::SVCCON_CONNECTING) {
		status
			<< wxT("svcc running - connecting to ")
			<< stateServerHost
			<< wxT(" (")
			<< stateServerAddress
			<< wxT(":")
			<< stateServerPort
			<< wxT(")");
		message
			<< wxT("Connecting to ")
			<< stateServerHost
			<< wxT(" (")
			<< stateServerAddress
			<< wxT(":")
			<< stateServerPort
			<< wxT(")");
	} else if (stateConnectionState == SVCState::SVCCON_DISCONNECTED) {
		status = wxT("svcc running - disconnected");
		if (oldState == SVCState::SVCCON_CONNECTED) {
			message = wxT("Disconnected");
			serverInfo->clearOptions();
			peerInfo->clearOptions();
		} else if (oldState == SVCState::SVCCON_CONNECTING) {
			message = wxT("Connection failed");
		}
	}
	// update status
	statusBar->SetStatusText(status, 0);
	if (!message.IsEmpty()) {
		logConnection(message);
	}
}

void SVCWindow::updateSVCState() {
	SVCState* state = app->getState();
	if (!state->isReady()) {
		statusBar->SetStatusText(wxT("svcc not running"));
		while (state->hasEvents()) {
			// drain events.
			state->getNextEvent();
		}
		return;
	}

	if (firstStateChange) {
		statusBar->SetStatusText(wxT("svcc running - disconnected"), 0);
		firstStateChange = false;
	}

	while(state->hasEvents()) {
		SVCEvent event = state->getNextEvent();

		switch (event.eventType) {
		case SVCEvent::EVENT_CONNECTION_STATE:
			handleConnectionState(event);
			break;

		case SVCEvent::EVENT_MUTED:
			if (event.flag) {
				log(wxT("You are now locally muted"));
			} else {
				log(wxT("You are no longer locally muted"));
			}
			break;

		case SVCEvent::EVENT_DEAFEN:
			if (event.flag) {
				log(wxT("You are now locally deafen"));
			} else {
				log(wxT("You are no longer locally deafen"));
			}
			break;

		case SVCEvent::EVENT_SERVER:
			stateServerAddress = event.address;
			stateServerHost = event.host;
			stateServerPort = event.port;
			break;

		case SVCEvent::EVENT_SERVER_SET:
			serverInfo->setOption(event.key, event.value);
			break;

		default:
			break;
		}
	}
}
