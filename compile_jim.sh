#!/bin/sh



if ! [ -d jimtcl ]; then
	git clone git://repo.or.cz/jimtcl.git jimtcl
fi

if ! [ -f jimtcl/Makefile ]; then
	cd jimtcl
	./configure --enable-utf8
	cd ..
fi

if ! [ -f jimtcl/libjim.a ]; then
	cd jimtcl
	make
	cd ..
fi

if ! [ -f libjim.a ]; then
	cp jimtcl/libjim.a .
fi
	