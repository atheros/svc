/*
 * svcstate.cpp
 *
 *  Created on: 22-06-2011
 *      Author: przemek
 */


#include "svcstate.hpp"
#include "svcparser.hpp"


SVCState::SVCState() {
	// process related variables
	svccPath = wxT("svcc");
	stdoutThread = NULL;
	stderrThread = NULL;
	svc = NULL;
	svcPid = 0;

	// SVC state variables
	stateChanged = true;
	locallyMuted = false;
	locallyDeafen = false;
	connectionState = SVCCON_DISCONNECTED;

}

SVCState::~SVCState() {
	killSVC();
}

void SVCState::parseCommand(wxString& command) {
	SVCParserError res;
	wxArrayString cmd;

	res = SVCParser::parse(command, cmd);
	if (res == SVCPARSER_ESCAPE) {
		fprintf(stderr, "SVC command has an unterminated escape sequence\n");
		return;
	} else if (res == SVCPARSER_STRING) {
		fprintf(stderr, "SVC command has an unterminated string\n");
	}

	if (cmd.IsEmpty()) {
		return;
	}

	if (cmd[0][0] == ':') {
		// handle state
		if (cmd[0] == wxT(":STATE")) {
			// connection state
			handleStateConnection(cmd);
		} else if (cmd[0] == wxT(":MUTED")) {
			// locally muted
			handleStateMuted(cmd);
		} else if (cmd[0] == wxT(":DEAFEN")) {
			// locally deafen
			handleStateDeafen(cmd);
		} else if (cmd[0] == wxT(":SERVER")) {
			// server address info
			handleStateServer(cmd);
		}
	} else {
		// some command from server
	}
}

void SVCState::handleStateMuted(wxArrayString& cmd) {
	if (cmd.size() != 2) {
		stateLog.Add(wxT("error: Invalid number of arguments in :MUTED"));
		return;
	}

	if (cmd[1] == wxT("0") && locallyMuted == true) {
		locallyMuted = false;
		stateChanged = true;
	} else if (cmd[1] == wxT("1") && locallyMuted == false) {
		locallyMuted = true;
		stateChanged = true;
	}
}

void SVCState::handleStateDeafen(wxArrayString& cmd) {
	if (cmd.size() != 2) {
		stateLog.Add(wxT("error: Invalid number of arguments in :DEAFEN"));
		return;
	}

	if (cmd[1] == wxT("0") && locallyDeafen == true) {
		locallyDeafen = false;
		stateChanged = true;
	} else if (cmd[1] == wxT("1") && locallyDeafen == false) {
		locallyDeafen = true;
		stateChanged = true;
	}
}

void SVCState::handleStateConnection(wxArrayString& cmd) {
	if (cmd.size() != 2) {
		stateLog.Add(wxT("error: Invalid number of arguments in :DEAFEN"));
		return;
	}

	if (cmd[1] == wxT("connecting") && connectionState != SVCCON_CONNECTING) {
		connectionState = SVCCON_CONNECTING;
		stateChanged = true;
	} else if (cmd[1] == wxT("connected") && connectionState != SVCCON_CONNECTED) {
		connectionState = SVCCON_CONNECTED;
		stateChanged = true;
	} else if (cmd[1] == wxT("disconnected") && connectionState != SVCCON_DISCONNECTED) {
		connectionState = SVCCON_DISCONNECTED;
		stateChanged = true;
	} else {
		stateLog.Add(wxT("error: Invalid connection state"));
	}
}

void SVCState::handleStateServer(wxArrayString& cmd) {
	unsigned long port;
	if (cmd.size() != 4) {
		stateLog.Add(wxT("error: Invalid number of arguments in :SERVER"));
		return;
	}

	serverHost = cmd[1];
	serverAddress = cmd[2];
	cmd[3].ToULong(&port, 10);
	serverPort = port;
	stateChanged = true;
}



void SVCState::processIO() {
	if (!svc) {
		return;
	}

	if (!wxProcess::Exists(svcPid)) {
		killSVC();
	} else {
		wxString line, cmdName;
		wxArrayString cmd;



		while (1) {
			stdLock.Lock();
			if (stdoutList.IsEmpty()) {
				stdLock.Unlock();
				break;
			} else {
				line = stdoutList.GetFirst()->GetData();
				svcOutput.Add(line);
				stdoutList.DeleteNode(stdoutList.GetFirst());
				stdLock.Unlock();
			}

			parseCommand(line);
		}

		{
			wxMutexLocker locker(stdLock);
			while (!stderrList.empty()) {
				svcError.Add(stderrList.GetFirst()->GetData());
				stderrList.DeleteNode(stderrList.GetFirst());
			}
		}
	}
}

void SVCState::openSVC() {
	stateLog.Add(wxT("Starting svcc..."));
	svc = wxProcess::Open(svccPath, wxEXEC_ASYNC);
	if (!svc) {
		stateLog.Add(wxString(wxT("error: failed to start ")) + svccPath);
		return;
	} else {
		stateLog.Add(wxT("svcc running"));
	}
	svcPid = svc->GetPid();
	stateChanged = true;

	stdoutThread = new ReaderThread(svc->GetInputStream(), &stdoutList,
			&stdLock);
	stderrThread = new ReaderThread(svc->GetErrorStream(), &stderrList,
			&stdLock);

	stdoutThread->Create();
	stderrThread->Create();

	stdoutThread->Run();
	stderrThread->Run();
}

void SVCState::killSVC() {
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

bool SVCState::isReady() {
	if (svc && wxProcess::Exists(svcPid)) {
		return true;
	} else {
		return false;
	}
}

void SVCState::sendCommand(const wxString& command) {
	wxString msg = command + wxT("\n");
	if (isReady()) {
		svc->GetOutputStream()->Write(msg.mb_str(), strlen(msg.mb_str()));
	}
}

bool SVCState::getSVCOutput(wxString& line) {
	if (svcOutput.IsEmpty()) {
		return false;
	} else {
		line = svcOutput[0];
		svcOutput.RemoveAt(0, 1);
		return true;
	}
}

bool SVCState::getSVCError(wxString& line) {
	if (svcError.IsEmpty()) {
		return false;
	} else {
		line = svcError[0];
		svcError.RemoveAt(0, 1);
		return true;
	}
}

bool SVCState::getLog(wxString& line) {
	if (stateLog.IsEmpty()) {
		return false;
	} else {
		line = stateLog[0];
		stateLog.RemoveAt(0, 1);
		return true;
	}
}

