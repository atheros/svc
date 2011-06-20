/*
 * app.cpp
 *
 *  Created on: 20-06-2011
 *      Author: przemek
 */

#include "app.hpp"

IMPLEMENT_APP(SVCApp)

bool SVCApp::OnInit() {
	window = new SVCWindow();
	window->Show(true);
	return true;
}


