/*
 * app.cpp
 *
 *  Created on: 20-06-2011
 *      Author: przemek
 */

#include "app.hpp"


IMPLEMENT_APP(SVCApp)


bool SVCApp::OnInit() {
	svccPath = wxT("./svcc");
	stdoutThread = NULL;
	stderrThread = NULL;
	svc = NULL;
	svcPid = 0;

	window = new SVCWindow(this);
	window->Show(true);
	openSVC();
	return true;
}

int SVCApp::OnExit() {
	killSVC();
	return 0;
}


void SVCApp::processIO() {
	if (!svc) {
		return;
	}

	if (!wxProcess::Exists(svcPid)) {
		window->log(wxT("SVCC terminated with code\n"));
		killSVC();
		openSVC();
	} else {
		wxString line;

		wxMutexLocker locker(stdLock);

		while (!stdoutList.empty()) {
			window->ioStdout(stdoutList.GetFirst()->GetData());
			stdoutList.DeleteNode(stdoutList.GetFirst());
		}

		while (!stderrList.empty()) {
			window->ioStderr(stderrList.GetFirst()->GetData());
			window->logSVCError(stderrList.GetFirst()->GetData());
			stderrList.DeleteNode(stderrList.GetFirst());
		}
	}
}

void SVCApp::openSVC() {
	window->log(wxString(wxT("Starting svcc: ")) + svccPath);
	svc = wxProcess::Open(svccPath, wxEXEC_ASYNC);
	if (!svc) {
		window->logError(wxT("failed to start svcc\n"));
		return;
	}
	svcPid = svc->GetPid();
	window->updateSVCState();

	stdoutThread = new ReaderThread(svc->GetInputStream(), &stdoutList,
			&stdLock);
	stderrThread = new ReaderThread(svc->GetErrorStream(), &stderrList,
			&stdLock);

	stdoutThread->Create();
	stderrThread->Create();

	stdoutThread->Run();
	stderrThread->Run();
}

void SVCApp::killSVC() {
	if (!svc) {
		return;
	}

	// kill the process (so it can send EOF to it's output/error streams
	if (wxProcess::Exists(svcPid)) {
		wxProcess::Kill(svcPid, wxSIGTERM);
	}

	// signal stdout reader to end
	if (stdoutThread) {
		stdoutThread->signalEnd();
	}

	// signal stderr reader to end
	if (stderrThread) {
		stderrThread->signalEnd();
	}

	// wait for stdout reader
	if (stdoutThread) {
		stdoutThread->Wait();
		delete stdoutThread;
	}

	// wait for stdout reader
	if (stderrThread) {
		stderrThread->Wait();
		delete stderrThread;
	}

	// delete SVC process object
	svc = NULL;
}

bool SVCApp::isSVCReady() {
	if (svc && wxProcess::Exists(svcPid)) {
		return true;
	} else {
		return false;
	}
}


void SVCApp::sendCommand(const wxString& command) {
	wxString msg = command + wxT("\n");
	if (!isSVCReady()) {
		window->logError(wxT("Can't execute command, svcc not running"));
	} else {
		window->ioStdin(command);
		svc->GetOutputStream()->Write(msg.mb_str(), strlen(msg.mb_str()));
	}
}

void SVCApp::sendUserCommand(const wxString& command) {
	wxString msg = command + wxT("\n");
	if (!isSVCReady()) {
		window->logError(wxT("Can't execute command, svcc not running"));
	} else {
		window->logCommand(command);
		window->ioStdin(command);
		svc->GetOutputStream()->Write(msg.mb_str(), strlen(msg.mb_str()));
	}
}


