/*
 * app.cpp
 *
 *  Created on: 20-06-2011
 *      Author: przemek
 */

#include "app.hpp"
#include "svcparser.hpp"

IMPLEMENT_APP(SVCApp)


bool SVCApp::OnInit() {
	state = new SVCState();
	// FIXME: just for debugging purposes.
	state->setSVCCPath(wxT("./svcc"));

	window = new SVCWindow(this);
	window->Show(true);


	// open SVC
	state->openSVC();
	return true;
}

int SVCApp::OnExit() {
	delete state;
	return 0;
}



void SVCApp::sendCommand(const wxString& command) {
	if (!state->isReady()) {
		window->logError(wxT("Can't execute command, svcc not running"));
	} else {
		window->ioStdin(command);
		state->sendCommand(command);
	}
}

void SVCApp::sendUserCommand(const wxString& command) {
	if (!state->isReady()) {
		window->logError(wxT("Can't execute command, svcc not running"));
	} else {
		window->logCommand(command);
		window->ioStdin(command);
		state->sendCommand(command);
	}
}


