/*
 * svcevent.hpp
 *
 *  Created on: 23-06-2011
 *      Author: przemek
 */

#ifndef SVCEVENT_HPP_
#define SVCEVENT_HPP_

#include <wx/string.h>
#include <wx/list.h>

class SVCEvent {
public:
	typedef enum {
		EVENT_UNKNOWN,
		EVENT_MUTED,
		EVENT_DEAFEN,
		EVENT_SERVER,
		EVENT_CONNECTION_STATE,
		EVENT_PEER_ADD,
		EVENT_PEER_DEL,
		EVENT_PEER_SET,
		EVENT_SERVER_SET,
		EVENT_I_AM
	} EventType;

	EventType eventType;
	bool flag;
	wxString host;
	wxString address;
	unsigned int port;
	int peerId;
	int connectionState;
	wxString key;
	wxString value;

	SVCEvent() {
		flag = false;
		port = 0;
		peerId = -1;
		connectionState = 0;
		eventType = EVENT_UNKNOWN;
	}

	static SVCEvent* createMuted(bool flag) {
		SVCEvent* e = new SVCEvent();
		e->eventType = EVENT_MUTED;
		e->flag = flag;
		return e;
	}

	static SVCEvent* createDefean(bool flag) {
		SVCEvent* e = new SVCEvent();
		e->eventType = EVENT_DEAFEN;
		e->flag = flag;
		return e;
	}

	static SVCEvent* createServer(const wxString& host, const wxString& address, unsigned int port) {
		SVCEvent* e = new SVCEvent();
		e->eventType = EVENT_SERVER;
		e->host = host;
		e->address = address;
		e->port = port;
		return e;
	}

	static SVCEvent* createConnectionState(int connectionState) {
		SVCEvent* e = new SVCEvent();
		e->eventType = EVENT_CONNECTION_STATE;
		e->connectionState = connectionState;
		return e;
	}

	static SVCEvent* createPeerAdd(int peerId) {
		SVCEvent* e = new SVCEvent();
		e->eventType = EVENT_PEER_ADD;
		e->peerId = peerId;
		return e;
	}

	static SVCEvent* createPeerDel(int peerId) {
		SVCEvent* e = new SVCEvent();
		e->eventType = EVENT_PEER_DEL;
		e->peerId = peerId;
		return e;
	}

	static SVCEvent* createPeerSet(int peerId, const wxString& key, const wxString& value) {
		SVCEvent* e = new SVCEvent();
		e->eventType = EVENT_PEER_SET;
		e->peerId = peerId;
		e->key = key;
		e->value = value;
		return e;
	}

	static SVCEvent* createServerSet(const wxString& key, const wxString& value) {
		SVCEvent* e = new SVCEvent();
		e->eventType = EVENT_SERVER_SET;
		e->key = key;
		e->value = value;
		return e;
	}

	static SVCEvent* createIAm(int peerId) {
		SVCEvent* e = new SVCEvent();
		e->eventType = EVENT_I_AM;
		e->peerId = peerId;
		return e;
	}


};

WX_DECLARE_LIST(SVCEvent, SVCEventList);


#endif /* SVCEVENT_HPP_ */
