/*
 * readerthread.cpp
 *
 *  Created on: 21-06-2011
 *      Author: przemek
 */

#include "readerthread.hpp"

ReaderThread::ReaderThread(wxInputStream* stream, wxStringList* output,
		wxMutex* lock) :
	wxThread(wxTHREAD_JOINABLE) {
	this->stream = stream;
	this->output = output;
	this->lock = lock;
	this->endReceived = false;
}

ReaderThread::~ReaderThread() {

}

void* ReaderThread::Entry() {
	wxString buffer;
	int c;
	while (!stream->Eof() && !shouldEnd()) {
		c = stream->GetC();
		if (c == '\n') {
			wxMutexLocker locker(*lock);
			output->Add(buffer);
			buffer.Clear();
		} else {
			buffer.Append(c, 1);
		}
	}
}

void ReaderThread::signalEnd() {
	wxMutexLocker locker(endLock);
	endReceived = true;
}

bool ReaderThread::shouldEnd() {
	wxMutexLocker locker(endLock);
	return endReceived;
}

