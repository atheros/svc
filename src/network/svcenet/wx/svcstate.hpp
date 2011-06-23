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
#include "svcreaderthread.hpp"
#include "svcevent.hpp"


WX_DECLARE_STRING_HASH_MAP(wxString, SVCPeerInfo);
WX_DECLARE_STRING_HASH_MAP(wxString, SVCServerInfo);
WX_DECLARE_HASH_MAP(int, SVCPeerInfo, wxIntegerHash, wxIntegerEqual, SVCPeerInfoMap);


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
	SVCReaderThread* stdoutThread;


	/**
	 * SVC stderr reader thread
	 * FIXME: will use 100% CPU time in 2.9.2
	 */
	SVCReaderThread* stderrThread;

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
	 * Server host name.
	 */
	wxString serverHost;

	/**
	 * Server IP address.
	 */
	wxString serverAddress;

	/**
	 * Server port.
	 */
	unsigned int serverPort;

	/**
	 * SVC connection state.
	 */
	ConnectionState connectionState;


	wxArrayString svcOutput;
	wxArrayString svcError;
	wxArrayString stateLog;

	SVCPeerInfoMap peers;
	SVCServerInfo serverInfo;

	SVCEventList eventList;

	void parseCommand(const wxString& command);

	void handleStateMuted(wxArrayString& cmd);
	void handleStateDeafen(wxArrayString& cmd);
	void handleStateConnection(wxArrayString& cmd);
	void handleStateServer(wxArrayString& cmd);

	void handleCommandSSET(wxArrayString& cmd);
	void handleCommandYARE(wxArrayString& cmd);
	void handleCommandPADD(wxArrayString& cmd);
	void handleCommandPDEL(wxArrayString& cmd);
	void handleCommandPSET(wxArrayString& cmd);
	void handleCommandMESG(wxArrayString& cmd);

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
	 * Returns server host name.
	 */
	const wxString& getServerHost() {
		return serverHost;
	}

	/**
	 * Returns server IP address.
	 */
	const wxString& getServerAddress() {
		return serverAddress;
	}

	/**
	 * Returns server port.
	 */
	unsigned int getServerPort() {
		return serverPort;
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

	/**
	 * Check if server info or peer info changed.
	 */
	bool hasEvents() {
		return eventList.size() ? true : false;
	}

	SVCEvent getNextEvent() {
		SVCEvent event;
		if (!eventList.empty()) {
			event = *eventList.GetFirst()->GetData();
			eventList.erase(eventList.begin());
		}
		return event;
	}
};

#endif /* SVCSTATE_HPP_ */
