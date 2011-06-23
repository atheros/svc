/*
 * svcobjectinfo.cpp
 *
 *  Created on: 23-06-2011
 *      Author: przemek
 */

#include "svcobjectinfo.hpp"

SVCObjectInfo::SVCObjectInfo(wxWindow* parent)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	keysBox = new wxListBox(splitter, ID_KEYS, wxDefaultPosition, wxDefaultSize, wxArrayString(),
			wxLB_SINGLE
			| wxLB_SORT
			| wxLB_NEEDED_SB
	);

	valueBox = new wxTextCtrl(splitter, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,
			wxTE_MULTILINE
			| wxTE_READONLY
			| wxTE_RICH
			| wxTE_DONTWRAP
	);


	splitter->SplitVertically(keysBox, valueBox, 200);
	splitter->SetSashGravity(0.0);
	splitter->SetSashPosition(200);
	splitter->SetMinimumPaneSize(200);

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(splitter, 1, wxEXPAND);
	SetSizer(sizer);

}

SVCObjectInfo::~SVCObjectInfo() {

}

void SVCObjectInfo::clearOptions() {
	keysBox->Clear();
	values.clear();
}

void SVCObjectInfo::setOption(const wxString& name, const wxString& value) {
	if (values.find(name) == values.end()) {
		// new value
		keysBox->Insert(name, -1);
	}
	values[name] = value;
}

