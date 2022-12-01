#
# Makefile for chat
#
# LIBS	= -lsocket -lnsl
LIBS	= -lcrypto -lssl -lgnutls
CFLAGS	= -g -ggdb -std=c99

all:	tls

# Add "directoryServer5" to lines 11 and 13 if implemented
tls:	chatServer5 chatClient5

client5.o server5.o: inet.h

chatServer5: server5.o
	gcc $(CFLAGS) -o $@ chatServer5.c $(LIBS)

chatClient5: client5.o
	gcc $(CFLAGS) -o $@ chatClient5.c $(LIBS)

# directoryServer5: directory5.o
# 	gcc $(CFLAGS) -o $@ directoryServer5.c $(LIBS)

#
# Clean up the mess we made
# Add "directoryServer5" if you add directoryServer
clean:
	rm *.o \
	chatServer5 chatClient5 2>/dev/null
