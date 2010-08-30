LIBS=-lconfig -lenet
CFLAGS=-Isrc/common -Isrc/svccd

SVCC_OBJS=	\
	src/svcc/main.o	\
	
SVCCD_OBJS=	\
	src/svccd/main.o src/svccd/audio_api_portaudio.o	\ 

SVCSERVER_OBJS=	\
	src/svcserver/main.o	\

COMMON_OBJS=	\
	src/common/thread.o	\

TESTS_OBJS=	\
	src/tests/test_threads.o	\



all: svcc svccd svcserver

svcc: $(SVCC_OBJS)
	gcc -o svcc $(SVCC_OBJS) $(LIBS)

svccd: $(SVCCD_OBJS)
	gcc -o svccd $(SVCCD_OBJS) $(LIBS)

svcserver: $(SVCSERVER_OBJS)
	gcc -o svcserver $(SVCSERVER_OBJS) $(LIBS)


tests_base:
	if ! test -d tests; then mkdir tests; fi

tests: tests_base test_threads test_audio_api

run_tests: tests
	tests/test_threads
	tests/test_audio_api


test_threads: src/tests/test_threads.o src/common/thread.o src/common/thread.h
	gcc -o tests/test_threads src/tests/test_threads.o src/common/thread.o -lpthread

test_audio_api: src/tests/test_audio_api.o src/svccd/audio_api_portaudio.o
	gcc -o tests/test_audio_api src/tests/test_audio_api.o src/svccd/audio_api_portaudio.o -lportaudio




clean:
	rm -f $(SVCC_OBJS) $(SVCCD_OBJS) $(SVCSERVER_OBJS) $(TESTS_OBJS)
