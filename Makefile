#
# Makefile for chat
#
# LIBS	= -lsocket -lnsl
LIBS	=
CFLAGS	= -g -ggdb -std=c99

all:	nonblock

nonblock:	chatServer4 chatClient4 

client4.o server4.o: inet.h

chatServer4: server4.o
	gcc $(CFLAGS) -o $@ chatServer4.c $(LIBS)

chatClient4: client4.o
	gcc $(CFLAGS) -o $@ chatClient4.c $(LIBS)

#
# Clean up the mess we made
#
clean:
	rm *.o \
	chatServer4 chatClient4 directoryServer4 2>/dev/null
