.SECONDEXPANSION:
.PHONY: clean
LIBS=-lconfig -lenet -lportaudio -lpthread $(shell pkg-config --libs celt)
CFLAGS+=-Isrc/common -Isrc/libsvc -Isrc/libsvc/include -std=c89 -fPIC -L. -g

# FIXME: this shit doesn't work
libsvc.so_OBJS= \
	src/libsvc/packet_queue.o src/libsvc/audio.o src/libsvc/audio_api_portaudio.o \
	src/libsvc/packet_cage.o src/common/thread.o src/libsvc/libsvc.o \
	src/libsvc/libsvc_incoming.o src/libsvc/codec.o \
	src/libsvc/network_packet.o

test_threads_OBJS := src/tests/test_threads.o src/common/thread.o

test_audio_api_OBJS := src/tests/test_audio_api.o src/libsvc/audio_api_portaudio.o

test_audio_packet_cage_OBJS := src/tests/test_audio_packet_cage.o  src/libsvc/audio.o src/libsvc/audio_api_portaudio.o src/libsvc/packet_cage.o src/common/thread.o

test_audio_cage_queue_OBJS := src/tests/test_audio_cage_queue.o  src/libsvc/packet_queue.o src/libsvc/audio.o src/libsvc/audio_api_portaudio.o src/libsvc/packet_cage.o src/common/thread.o

test_libsvc_OBJS := src/tests/test_libsvc.o  src/libsvc/packet_queue.o src/libsvc/audio.o src/libsvc/audio_api_portaudio.o \
                    src/libsvc/packet_cage.o src/common/thread.o src/libsvc/libsvc.o src/libsvc/libsvc_incoming.o src/libsvc/codec.o \
                    src/libsvc/network_packet.o

svcclient_OBJS := src/network/svcd/client/main.o

svcserver_OBJS := src/network/svcd/server/main.o src/common/dstr.o


-include $(OBJS:.o=.d)

%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

all: libsvc.so tests svcclient svcserver

test_libsvc test_threads test_audio_api test_audio_cage_queue test_audio_packet_cage: $$($$@_OBJS)
	$(CC) $(LDFLAGS) $^ -o ./tests/$@ $(LIBS)

libsvc.so: $$($$@_OBJS)
	$(CC) -shared $(LDFLAGS) $^ -o $@ $(LIBS)

tests: test_threads test_audio_api test_audio_cage_queue test_audio_packet_cage test_libsvc


svcclient svcserver: $$($$@_OBJS) libsvc.so
	$(CC) $(LDFLAGS) libsvc.so $^ -o $@ $(LIBS)


clean:
	rm -f src/*/*.o src/*/*.d tests/* libsvc.so svcclient svcserver
