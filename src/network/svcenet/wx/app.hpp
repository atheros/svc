/*
 * app.hpp
 *
 *  Created on: 20-06-2011
 *      Author: przemek
 */

#ifndef APP_HPP_
#define APP_HPP_

#include <wx/wx.h>
#include "svcwindow.hpp"
#include "readerthread.hpp"

class SVCApp: public wxApp {
private:
	/**
	 * wxSVC main window.
	 */
	SVCWindow* window;

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

public:

	/**
	 * Initialize application.
	 */
	virtual bool OnInit();

	/**
	 * Terminate application.
	 */
	virtual int OnExit();

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
	 * Returns SVC process instance.
	 */
	wxProcess* getSVC() {
		return svc;
	}

	/**
	 * Check if SVC is ready to accept IO.
	 */
	bool isSVCReady();

	/**
	 * Execute SVC command.
	 *
	 * Same as sendUserCommand except it won't log the command in console.
	 */
	void sendCommand(const wxString& command);

	/**
	 * Execute command supplied by user.
	 */
	void sendUserCommand(const wxString& command);


};


#endif /* APP_HPP_ */
