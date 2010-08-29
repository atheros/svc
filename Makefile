LIBS=-lconfig -lenet
CFLAGS=-Isrc/common

SVCC_OBJS=	\
	src/svcc/main.o	\
	
SVCCD_OBJS=	\
	src/svccd/main.o	\ 

SVCSERVER_OBJS=	\
	src/svcserver/main.o	\

all: svcc svccd svcserver

svcc: $(SVCC_OBJS)
	gcc -o svcc $(SVCC_OBJS) $(LIBS)

svccd: $(SVCCD_OBJS)
	gcc -o svccd $(SVCCD_OBJS) $(LIBS)

svcserver: $(SVCSERVER_OBJS)
	gcc -o svcserver $(SVCSERVER_OBJS) $(LIBS)

clean:
	rm -f $(SVCC_OBJS) $(SVCCD_OBJS) $(SVCSERVER_OBJS)
