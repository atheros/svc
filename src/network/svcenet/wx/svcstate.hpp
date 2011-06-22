/*
 * svcstate.hpp
 *
 *  Created on: 22-06-2011
 *      Author: przemek
 */

#ifndef SVCSTATE_HPP_
#define SVCSTATE_HPP_

#include <wx/string.h>
#include <wx/hashmap.h>
#include <wx/process.h>
#include "readerthread.hpp"


class SVCState {
public:
	typedef enum {
		SVCCON_DISCONNECTED,
		SVCCON_CONNECTED,
		SVCCON_CONNECTING
	} ConnectionState;

private:

	/**
	 * SVC process.
	 */
	wxProcess* svc;

	/**
	 * SVC pid.
	 */
	long int svcPid;

	/**
	 * SVC stdout reader thread
	 * FIXME: will use 100% CPU time in 2.9.2
	 */
	ReaderThread* stdoutThread;


	/**
	 * SVC stderr reader thread
	 * FIXME: will use 100% CPU time in 2.9.2
	 */
	ReaderThread* stderrThread;

	/**
	 * stdoutList/stderrList lock.
	 */
	wxMutex stdLock;

	/**
	 * SVC output lines.
	 */
	wxStringList stdoutList;

	/**
	 * SVC error lines.
	 */
	wxStringList stderrList;


	/**
	 * SVCC path.
	 */
	wxString svccPath;

	/**
	 * Is SVC locally muted.
	 */
	bool locallyMuted;

	/**
	 * Is SVC locally deafen.
	 */
	bool locallyDeafen;

	/**
	 * SVC connection state.
	 */
	ConnectionState connectionState;

	void parseCommand(wxString& command);

	wxArrayString svcOutput;
	wxArrayString svcError;
	wxArrayString stateLog;

	bool stateChanged;

	void handleStateMuted(wxArrayString& cmd);
	void handleStateDeafen(wxArrayString& cmd);
	void handleStateConnection(wxArrayString& cmd);

public:

	SVCState();
	~SVCState();

	/**
	 * Sets SVCC binary path.
	 */
	void setSVCCPath(const wxString& svccPath) {
		this->svccPath = svccPath;
	}

	/**
	 * Returns SVCC binary path.
	 */
	const wxString& getSVCCPath();

	/**
	 * Open SVC process.
	 *
	 * NOTE: svc process should be closed before calling this function.
	 */
	void openSVC();

	/**
	 * Kill SVC process.
	 */
	void killSVC();

	/**
	 * Process SVC IO.
	 */
	void processIO();

	/**
	 * Check if SVC is ready to accept IO.
	 */
	bool isReady();

	/**
	 * Check if SVC is muted locally
	 */
	bool isLocallyMuted() {
		return locallyMuted;
	}

	/**
	 * Check if SVC is deafen locally
	 */
	bool isLocallyDeafen() {
		return locallyDeafen;
	}

	/**
	 * Returns SVC connection state.
	 */
	ConnectionState getConnectionState() {
		return connectionState;
	}

	/**
	 * Execute SVC command.
	 *
	 * Same as sendUserCommand except it won't log the command in console.
	 */
	void sendCommand(const wxString& command);

	bool getSVCOutput(wxString& line);
	bool getSVCError(wxString& line);
	bool getLog(wxString& line);

	bool isStateChanged() {
		bool r = stateChanged;
		stateChanged = false;
		return r;
	}
};

#endif /* SVCSTATE_HPP_ */
