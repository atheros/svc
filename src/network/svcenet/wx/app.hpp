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
#include "svcstate.hpp"

class SVCApp: public wxApp {
public:

private:
	/**
	 * wxSVC main window.
	 */
	SVCWindow* window;

	/**
	 * SVC state.
	 */
	SVCState* state;

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
	 * Returns SVC state.
	 */
	SVCState* getState() {
		return state;
	}

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
