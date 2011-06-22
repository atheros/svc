/*
 * svcparser.hpp
 *
 *  Created on: 21-06-2011
 *      Author: przemek
 */

#ifndef SVCPARSER_HPP_
#define SVCPARSER_HPP_

#include <wx/string.h>
#include <wx/arrstr.h>

/**
 * Errors returned by SVCParser::parse().
 */
typedef enum {
	SVCPARSER_OK = 0, // parse successful
	SVCPARSER_ESCAPE, // unterminated escape sequence
	SVCPARSER_STRING  // unterminated string
} SVCParserError;

/**
 * SVC command parser.
 */
class SVCParser {
public:
	/**
	 * Parse SVC command into an array of tokens.
	 */
	static SVCParserError parse(const wxString& line, wxArrayString& cmd);

	/**
	 * Escape a string token (part for a command) to send to SVC.
	 */
	static wxString escape(wxString& text);

};

#endif /* SVCPARSER_HPP_ */
