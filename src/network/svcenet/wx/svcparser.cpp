/*
 * svcparser.cpp
 *
 *  Created on: 21-06-2011
 *      Author: przemek
 */

#include "svcparser.hpp"

SVCParserError SVCParser::parse(const wxString& line, wxArrayString& cmd) {
	bool in_escape = 0;
	bool in_string = 0;
	wxString buffer;
	size_t i, len;
	int ch;

	cmd.Clear();

	len = line.size();
	for(i = 0; i < len; i++) {
		ch = line[i];

		if (in_escape) {
			switch (ch) {
			case 'n':
				buffer.Append('\n', 1);
				break;
			case 'r':
				buffer.Append('\r', 1);
				break;
			case '0':
				buffer.Append((wxChar)'\0', 1);
				break;
			case '"':
				buffer.Append('"', 1);
				break;
			case 't':
				buffer.Append('\t', 1);
				break;
			case '\\':
				buffer.Append('\\', 1);
				break;
			default:
				buffer.Append(ch, 1);
				break;
			}
			in_escape = false;
		} else if (in_string) {
			if (ch == '"') {
				cmd.Add(buffer);
				buffer.Clear();
				in_string = false;
			} else if (ch == '\\') {
				in_escape = true;
			} else {
				buffer.Append(ch, 1);
			}
		} else {
			if (ch == '"') {
				if (!buffer.IsEmpty()) {
					cmd.Add(buffer);
					buffer.Clear();
				}

				in_string = true;
			} else if (ch == '\\') {
				in_escape = true;
			} else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
				if (!buffer.IsEmpty()) {
					cmd.Add(buffer);
					buffer.Clear();
				}
			} else {
				buffer.Append(ch, 1);
			}
		}
	}

	if (in_escape) {
		return SVCPARSER_ESCAPE;
	} else if (in_string) {
		return SVCPARSER_STRING;
	} else if (!buffer.IsEmpty()) {
		cmd.Add(buffer);
	}

	return SVCPARSER_OK;
}


wxString SVCParser::escape(wxString& text) {
	size_t i, len;
	wxString o;
	int c;

	len = text.size();
	for(i = 0; i < len; i++) {
		c = text[i];
		switch (c) {
		case '\n':
			o.Append(wxT("\\n"));
			break;
		case '\r':
			o.Append(wxT("\\r"));
			break;
		case 0:
			o.Append(wxT("\\0"));
			break;
		case '\"':
			o.Append(wxT("\\\""));
			break;
		case '\t':
			o.Append(wxT("\\t"));
			break;
		case '\\':
			o.Append(wxT("\\\\"));
			break;
		default:
			o.Append(c, 1);
			break;
		}
	}

	return o;
}
