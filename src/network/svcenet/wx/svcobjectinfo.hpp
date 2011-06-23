/*
 * svcserverinfo.hpp
 *
 *  Created on: 23-06-2011
 *      Author: przemek
 */

#ifndef SVCOBJECTINFO_HPP_
#define SVCOBJECTINFO_HPP_

#include <wx/wx.h>
#include <wx/splitter.h>

WX_DECLARE_STRING_HASH_MAP(wxString, SVCObjectMap);

class SVCObjectInfo: public wxPanel {
private:
	SVCObjectMap values;

	wxListBox* keysBox;
	wxTextCtrl* valueBox;

	enum {
		ID_KEYS = wxID_HIGHEST + 10
	};

public:
	SVCObjectInfo(wxWindow* parent);
	~SVCObjectInfo();

	void clearOptions();
	void setOption(const wxString& name, const wxString& value);
};


#endif /* SVCOBJECTINFO_HPP_ */
