/*
 * idewindow.hpp
 *
 *  Created on: 19-06-2011
 *      Author: przemek
 */

#ifndef IDEWINDOW_HPP_
#define IDEWINDOW_HPP_

#include <wx/wx.h>
#include <wx/process.h>
#include <wx/treectrl.h>
#include <wx/aui/framemanager.h>
#include <wx/aui/auibook.h>
#include "svcstate.hpp"

class SVCApp;

class SVCWindow: public wxFrame {
	DECLARE_EVENT_TABLE()
private:
	SVCApp*				app;
	wxMenuBar*			menuBar;
	wxMenu*				menuFile;
	wxStatusBar*		statusBar;

	wxPanel*			logPanel;

	wxTextCtrl*			logOutput;
	wxTextCtrl*			input;
	wxTextCtrl*			ioOutput;

	wxTreeCtrl*			channels;

	wxAuiNotebook*		contentNotebook;
	wxAuiManager*		frameManager;


	wxTimer*			ioTimer;

	void OnIOTimer(wxTimerEvent& event);
	void OnCommand(wxCommandEvent& event);

	// state log
	SVCState::ConnectionState stateConnectionState;
	bool stateLocallyMuted;
	bool stateLocallyDeafen;
	bool firstStateChange;
public:
	SVCWindow(SVCApp* app);
	~SVCWindow();

	enum {
		ID_IOTIMER = wxID_HIGHEST + 1,
		ID_COMMAND_INPUT
	};

	enum {
		STATUS_SVCC = 0,
		__STATUS_COUNT
	};

	void ioStdout(const wxString& text);
	void ioStderr(const wxString& text);
	void ioStdin(const wxString& text);

	void log(const wxString& text);
	void logError(const wxString& text);
	void logSVCError(const wxString& text);
	void logCommand(const wxString& text);
	void logConnection(const wxString& text);

	void updateSVCState();
};

#endif /* IDEWINDOW_HPP_ */
