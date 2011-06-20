/*
 * idewindow.hpp
 *
 *  Created on: 19-06-2011
 *      Author: przemek
 */

#ifndef IDEWINDOW_HPP_
#define IDEWINDOW_HPP_

#include <wx/wx.h>

#include <wx/treectrl.h>
#include <wx/aui/framemanager.h>
#include <wx/aui/auibook.h>


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

	DECLARE_EVENT_TABLE()

public:
	SVCWindow();
	~SVCWindow();

};

#endif /* IDEWINDOW_HPP_ */
