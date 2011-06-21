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

class SVCReaderThread: public wxThread {
private:
	wxInputStream* stream;
	wxStringList* output;
	wxMutex* lock;
	wxMutex endLock;
	bool endReceived;

public:
	SVCReaderThread(wxInputStream* stream, wxStringList* output, wxMutex* lock);
	virtual ~SVCReaderThread();
	virtual void *Entry();

	void signalEnd();
	bool shouldEnd();
};

class SVCWindow: public wxFrame {
private:
	wxMenuBar*			menuBar;
	wxMenu*				menuFile;

	wxPanel*			logPanel;

	wxTextCtrl*			logOutput;
	wxTextCtrl*			input;
	wxTextCtrl*			ioOutput;

	wxTreeCtrl*			channels;

	wxAuiNotebook*		contentNotebook;
	wxAuiManager*		frameManager;

	wxProcess*			svc;
	SVCReaderThread*	stdoutThread;
	SVCReaderThread*	stderrThread;
	wxMutex				stdLock;
	wxStringList		stdoutList;
	wxStringList		stderrList;

	wxTimer*			ioTimer;

	DECLARE_EVENT_TABLE()

	void OnSVCTerminate(wxProcessEvent& event);
	void OnIOTimer(wxTimerEvent& event);
	void OnCommand(wxCommandEvent& event);

	void openSVC();
	void killSVC();
public:
	SVCWindow();
	~SVCWindow();

	enum {
		ID_SVC	= wxID_HIGHEST + 1,
		ID_IOTIMER,
		ID_COMMAND_INPUT
	};

	void ioStdout(const wxString& text);
	void ioStderr(const wxString& text);
	void ioStdin(const wxString& text);

	void log(const wxString& text);
	void logCommand(const wxString& text);
	void logConnection(const wxString& text);

	void processIO();
};

#endif /* IDEWINDOW_HPP_ */
