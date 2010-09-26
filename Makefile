.SECONDEXPANSION:
.PHONY: clean
LIBS=-lconfig -lenet -lportaudio -lpthread
CFLAGS+=-Isrc/common -Isrc/libsvc -Isrc/libsvc/include -std=c89

# FIXME: this shit doesn't work
libsvc_OBJS= \
	src/libsvc/audio_api_portaudio.o src/common/thread.o

test_threads_OBJS := src/tests/test_threads.o src/common/thread.o

test_audio_api_OBJS := src/tests/test_audio_api.o src/libsvc/audio_api_portaudio.o

test_audio_packet_cage_OBJS := src/tests/test_audio_packet_cage.o  src/libsvc/audio.o src/libsvc/audio_api_portaudio.o src/libsvc/packet_cage.o src/common/thread.o

test_audio_cage_queue_OBJS := src/tests/test_audio_cage_queue.o  src/libsvc/packet_queue.o src/libsvc/audio.o src/libsvc/audio_api_portaudio.o src/libsvc/packet_cage.o src/common/thread.o


-include $(OBJS:.o=.d)

%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

all: libsvc tests

libsvc test_threads test_audio_api test_audio_cage_queue test_audio_packet_cage: $$($$@_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

tests: test_threads test_audio_api test_audio_cage_queue test_audio_packet_cage

clean:
	rm -f src/*/*.o src/*/*.d tests/*
