LIBS=-lconfig -lenet
CFLAGS=-Isrc/common -Isrc/libsvc


COMMON_OBJS=	\
	src/common/thread.o	\

TESTS_OBJS=	\
	src/tests/test_threads.o	src/tests/test_audio_api.o \

LIBSVC_OBJS= \
	src/libsvc/audio_api_portaudio.o

all: 


tests_base:
	if ! test -d tests; then mkdir tests; fi

tests: tests_base test_threads test_audio_api

run_tests: tests
	tests/test_threads


test_threads: src/tests/test_threads.o src/common/thread.o src/common/thread.h
	gcc -o tests/test_threads src/tests/test_threads.o src/common/thread.o -lpthread

test_audio_api: src/tests/test_audio_api.o src/libsvc/audio_api_portaudio.o
	gcc -o tests/test_audio_api src/tests/test_audio_api.o src/libsvc/audio_api_portaudio.o -lportaudio


clean:
	rm -f $(COMMON_OBJS) $(TESTS_OBJS)
