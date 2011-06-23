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
	locallyMuted = false;
	locallyDeafen = false;
	connectionState = SVCCON_DISCONNECTED;

}

SVCState::~SVCState() {
	killSVC();
}

void SVCState::parseCommand(const wxString& command) {
	SVCParserError res;
	wxArrayString cmd;

	res = SVCParser::parse(command, cmd);
	if (res == SVCPARSER_ESCAPE) {
		stateLog.Add(wxT("error: SVC command has an unterminated escape sequence"));
		return;
	} else if (res == SVCPARSER_STRING) {
		stateLog.Add(wxT("error: SVC command has an unterminated string"));
		return;
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
		if (cmd[0] == wxT("SSET")) {
			// set server value
			handleCommandSSET(cmd);
		}
		// some command from server
	}
}

void SVCState::handleCommandSSET(wxArrayString& cmd) {
	if (cmd.size() != 3) {
		stateLog.Add(wxT("error: Invalid number of arguments in SSET"));
		return;
	}

	serverInfo[cmd[1]] = cmd[2];
	eventList.Append(SVCEvent::createServerSet(cmd[1], cmd[2]));
}

void SVCState::handleCommandYARE(wxArrayString& cmd) {

}

void SVCState::handleCommandPADD(wxArrayString& cmd) {

}

void SVCState::handleCommandPDEL(wxArrayString& cmd) {

}

void SVCState::handleCommandPSET(wxArrayString& cmd) {
	long int peerId = 0;
	if (cmd.size() != 4) {
		stateLog.Add(wxT("error: Invalid number of arguments in PSET"));
		return;
	}

	peers[peerId] = SVCPeerInfo();
	peers[peerId][cmd[2]] = cmd[3];

	eventList.Append(SVCEvent::createPeerSet(peerId, cmd[2], cmd[3]));

}

void SVCState::handleCommandMESG(wxArrayString& cmd) {

}

void SVCState::handleStateMuted(wxArrayString& cmd) {
	if (cmd.size() != 2) {
		stateLog.Add(wxT("error: Invalid number of arguments in :MUTED"));
		return;
	}

	if (cmd[1] == wxT("0") && locallyMuted == true) {
		locallyMuted = false;
		eventList.Append(SVCEvent::createMuted(false));
	} else if (cmd[1] == wxT("1") && locallyMuted == false) {
		locallyMuted = true;
		eventList.Append(SVCEvent::createMuted(true));
	}
}

void SVCState::handleStateDeafen(wxArrayString& cmd) {
	if (cmd.size() != 2) {
		stateLog.Add(wxT("error: Invalid number of arguments in :DEAFEN"));
		return;
	}

	if (cmd[1] == wxT("0") && locallyDeafen == true) {
		locallyDeafen = false;
		eventList.Append(SVCEvent::createDefean(false));
	} else if (cmd[1] == wxT("1") && locallyDeafen == false) {
		locallyDeafen = true;
		eventList.Append(SVCEvent::createDefean(true));
	}
}

void SVCState::handleStateConnection(wxArrayString& cmd) {
	if (cmd.size() != 2) {
		stateLog.Add(wxT("error: Invalid number of arguments in :DEAFEN"));
		return;
	}

	if (cmd[1] == wxT("connecting") && connectionState != SVCCON_CONNECTING) {
		connectionState = SVCCON_CONNECTING;
		eventList.Append(SVCEvent::createConnectionState(SVCCON_CONNECTING));
	} else if (cmd[1] == wxT("connected") && connectionState != SVCCON_CONNECTED) {
		connectionState = SVCCON_CONNECTED;
		eventList.Append(SVCEvent::createConnectionState(SVCCON_CONNECTED));
	} else if (cmd[1] == wxT("disconnected") && connectionState != SVCCON_DISCONNECTED) {
		connectionState = SVCCON_DISCONNECTED;
		eventList.Append(SVCEvent::createConnectionState(SVCCON_DISCONNECTED));
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


	eventList.Append(SVCEvent::createServer(serverHost, serverAddress, serverPort));
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

	stdoutThread = new SVCReaderThread(svc->GetInputStream(), &stdoutList,
			&stdLock);
	stderrThread = new SVCReaderThread(svc->GetErrorStream(), &stderrList,
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
	if (svc && svcPid && wxProcess::Exists(svcPid)) {
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

