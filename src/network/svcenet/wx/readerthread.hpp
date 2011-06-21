/*
 * readerthread.hpp
 *
 *  Created on: 21-06-2011
 *      Author: przemek
 */

#ifndef READERTHREAD_HPP_
#define READERTHREAD_HPP_

#include <wx/thread.h>
#include <wx/stream.h>

/**
 * Helper class going around wxWidget's lack of non-blocking pipe reads.
 *
 * NOTE: This reader is only usefull to read lines, and this is what it
 *       reports to output wxStringList.
 */
class ReaderThread: public wxThread {
private:
	/**
	 * Line stream to read from.
	 */
	wxInputStream* stream;

	/**
	 * String list to write to (without the ending \n).
	 */
	wxStringList* output;

	/**
	 * Lock locking the output list.
	 */
	wxMutex* lock;

	/**
	 * Lock locking the end flag.
	 */
	wxMutex endLock;

	/**
	 * End request recieved.
	 */
	bool endReceived;

public:
	/**
	 * Constructor.
	 *
	 * NOTE: you still need to call Create() then Run().
	 */
	ReaderThread(wxInputStream* stream, wxStringList* output, wxMutex* lock);
	virtual ~ReaderThread();

	virtual void *Entry();

	/**
	 * Use this to signal the thread it should quit.
	 * Ofcourse it won't check that status until blocked read ends (new input
	 * or EOF after killing the attached process).
	 */
	void signalEnd();

	/**
	 * Check if this thread is suppose to exit it's loop.
	 */
	bool shouldEnd();

};

#endif /* READERTHREAD_HPP_ */
