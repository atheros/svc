/*
 * svcwindow.cpp
 *
 *  Created on: 20-06-2011
 *      Author: przemek
 */

#include "svcwindow.hpp"

SVCReaderThread::SVCReaderThread(wxInputStream* stream, wxStringList* output,
		wxMutex* lock)
: wxThread(wxTHREAD_JOINABLE)
{
	this->stream = stream;
	this->output = output;
	this->lock = lock;
	this->endReceived = false;
}

SVCReaderThread::~SVCReaderThread() {

}

void* SVCReaderThread::Entry() {
	wxString buffer;
	int c;
	while (!stream->Eof() && !shouldEnd()) {
		c = stream->GetC();
		if (c == '\n') {
			wxMutexLocker locker(*lock);
			output->Add(buffer);
			buffer.Clear();
		} else {
			buffer.Append(c, 1);
		}
	}
}

void SVCReaderThread::signalEnd() {
	wxMutexLocker locker(endLock);
	endReceived = true;
}

bool SVCReaderThread::shouldEnd() {
	wxMutexLocker locker(endLock);
	return endReceived;
}



BEGIN_EVENT_TABLE(SVCWindow, wxFrame)
//EVT_END_PROCESS(ID_SVC, SVCWindow::OnSVCTerminate)
				EVT_TIMER(ID_IOTIMER, SVCWindow::OnIOTimer) EVT_COMMAND(ID_COMMAND_INPUT, wxEVT_COMMAND_TEXT_ENTER, SVCWindow::OnCommand)
				END_EVENT_TABLE()

SVCWindow::SVCWindow() :
	wxFrame(NULL, wxID_ANY, wxT("wxSVC"), wxDefaultPosition, wxSize(800, 600)) {
	frameManager = new wxAuiManager(this);

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
			wxT("Command: "));
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

	openSVC();

	ioTimer = new wxTimer(this, ID_IOTIMER);
	ioTimer->Start(100, false);
}

SVCWindow::~SVCWindow() {
	frameManager->UnInit();
	killSVC();
}

void SVCWindow::processIO() {
	char buff[1024];
	size_t length;
	wxInputStream* in;

	if (!svc) {
		printf("svc == null\n");
		return;
	}

	if (!wxProcess::Exists(svc->GetPid())) {
		log(wxT("SVCC terminated with code\n"));
		killSVC();
		openSVC();
	} else {
		wxString line;

		wxMutexLocker locker(stdLock);

		while (!stdoutList.empty()) {
			ioStdout(stdoutList.GetFirst()->GetData());
			stdoutList.DeleteNode(stdoutList.GetFirst());
		}

		while (!stderrList.empty()) {
			ioStderr(stderrList.GetFirst()->GetData());
			stderrList.DeleteNode(stderrList.GetFirst());
		}
	}
}

void SVCWindow::openSVC() {
	svc = wxProcess::Open(wxT("./svcc"), wxEXEC_ASYNC);
	printf("IsInputOpened %i\n", svc->IsInputOpened() ? 1 : 0);

	stdoutThread = new SVCReaderThread(svc->GetInputStream(), &stdoutList,
			&stdLock);
	stderrThread = new SVCReaderThread(svc->GetErrorStream(), &stderrList,
			&stdLock);

	stdoutThread->Create();
	stderrThread->Create();

	stdoutThread->Run();
	stderrThread->Run();
}

void SVCWindow::killSVC() {
	if (!svc) {
		return;
	}

	if (wxProcess::Exists(svc->GetPid())) {
		wxProcess::Kill(svc->GetPid(), wxSIGTERM);
	}

	if (stdoutThread) {
		stdoutThread->signalEnd();
	}

	if (stderrThread) {
		stderrThread->signalEnd();
	}

	if (stdoutThread) {
		stdoutThread->Wait();
		delete stdoutThread;
	}

	if (stderrThread) {
		stderrThread->Wait();
		delete stderrThread;
	}

	delete svc;
}

void SVCWindow::OnSVCTerminate(wxProcessEvent& event) {
	processIO();
}

void SVCWindow::OnIOTimer(wxTimerEvent& event) {
	processIO();
}

void SVCWindow::OnCommand(wxCommandEvent& event) {
	printf("SVCWindow::OnCommand()\n");
	if (svc && input->GetNumberOfLines()) {
		wxOutputStream* out = svc->GetOutputStream();
		wxString msg = input->GetValue() + wxT("\n");
		if (out) {
			out->Write(msg.mb_str(),
					strlen(msg.mb_str()));
		}
	}

	logCommand(input->GetValue());
	ioStdin(input->GetValue());
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
	logOutput->AppendText(text);
}

void SVCWindow::logCommand(const wxString& text) {
	logOutput->SetForegroundColour(wxColour(0, 200, 0, 255));
	logOutput->AppendText(wxT("Excuting "));
	logOutput->SetForegroundColour(wxColour(0, 0, 255, 255));
	logOutput->AppendText(text);
	logOutput->AppendText(wxT("\n"));
}

void SVCWindow::logConnection(const wxString& text) {
	logOutput->SetForegroundColour(wxColour(255, 160, 0, 255));
	logOutput->AppendText(text);
}
