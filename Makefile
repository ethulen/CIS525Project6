#
# Makefile for chat
#
# LIBS	= -lsocket -lnsl
LIBS	= -lcrypto -lssl -lgnutls
CFLAGS	= -g -ggdb -std=c99

all:	tls

tls:	chatServer5 chatClient5 directoryServer5

client5.o server5.o directory5.o: inet.h

chatServer5: server5.o
	gcc $(CFLAGS) -o $@ chatServer5.c $(LIBS)

chatClient5: client5.o
	gcc $(CFLAGS) -o $@ chatClient5.c $(LIBS)

directoryServer5: directory5.o
	gcc $(CFLAGS) -o $@ directoryServer5.c $(LIBS)

#
# Clean up the mess we made
#
clean:
	rm *.o \
	chatServer5 chatClient5 directoryServer5 2>/dev/null
