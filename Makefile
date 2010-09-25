LIBS=-lconfig -lenet
CFLAGS=-Isrc/common -Isrc/libsvc


COMMON_OBJS=	\
	src/common/thread.o	\

TESTS_OBJS=	\
	src/tests/test_threads.o	src/tests/test_audio_api.o src/tests/test_audio_packet_cage.o \

LIBSVC_OBJS= \
	src/libsvc/audio_api_portaudio.o

all: 


tests_base:
	if ! test -d tests; then mkdir tests; fi

tests: tests_base test_threads test_audio_api test_audio_packet_cage

run_tests: tests
	tests/test_threads


test_threads: src/tests/test_threads.o src/common/thread.o src/common/thread.h
	gcc -o tests/test_threads src/tests/test_threads.o src/common/thread.o -lpthread

test_audio_api: src/tests/test_audio_api.o src/libsvc/audio_api_portaudio.o
	gcc -o tests/test_audio_api src/tests/test_audio_api.o src/libsvc/audio_api_portaudio.o  -lportaudio 

test_audio_packet_cage: src/tests/test_audio_api.o src/libsvc/audio_api_portaudio.o src/libsvc/packet_cage.o
	gcc -o tests/test_audio_packet_cage src/tests/test_audio_api.o src/libsvc/audio_api_portaudio.o src/libsvc/packet_cage.o src/common/thread.o -lportaudio -lpthread



clean:
	rm -f $(COMMON_OBJS) $(TESTS_OBJS)
