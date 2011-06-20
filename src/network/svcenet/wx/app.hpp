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

class SVCApp: public wxApp {
private:
	SVCWindow* window;

public:
	virtual bool OnInit();

};


#endif /* APP_HPP_ */
