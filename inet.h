#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 100
#define MAX_CLIENTS 5
#define MAX_THREADS 5

#define SERV_TCP_PORT 17235
#define CLI_TCP_PORT 28686

typedef struct server{
	int active;
	char topic[MAX];
	unsigned int ip;
	unsigned short port;
	
}server;

/* Change the following to be your host addr: 129.130.10.43 for viper and 129.130.10.39 for cougar */
#define SERV_HOST_ADDR "129.130.10.43"
