/*------------------------------------------------------*/
/* chatClient5.c - UI for a chat room                   */
/*------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "inet.h"

#define MAX 100

void parseOutput(message* curmessage);

char msg[MAX];
int main()
{
    int i, n;
    char c;
    int nread;
    struct timeval wait_time; 
    struct sockaddr_in  serv_addr;
    int sockfd,servfd;
    fd_set readset;

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    serv_addr.sin_port        = htons(CLI_TCP_PORT);
    

    ///printf("a\n");
    /* Create a socket (an endpoint for communication). */
    if ( (servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client: can't open datagram socket");
        exit(1);
    }
    //printf("b\n");
    /* Bind the client's socket to the client's address */
    if (connect(servfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("client: can't bind local address");
        exit(1);
    }

    server options[10];
    int iter = 0;
    while(read(servfd,&options[iter],sizeof(server))>0){
	printf("Server:%d ->Topic:%s\n",iter,options[iter].topic);
	iter ++;

    }
    printf("please choose a server number(0-9) listed above: ");
    int choice = getchar()-'0';
    //printf("%d\n",options[choice].ip); 	
    //printf("%d\n",options[choice].port); 	
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = options[choice].ip;
    serv_addr.sin_port = options[choice].port;

    //serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    //serv_addr.sin_port        = htons(23621);
    
    //printf("test1\n");
    /* Create a socket (an endpoint for communication). */
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client: can't open datagram socket");
        exit(1);
    }

    //printf("test2\n");

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("client: can't bind local address");
        exit(1);
    }
    /* Bind the client's socket to the client's address */
    
    
    char name [MAX];
    printf("please enter a nickname\n");
    read( STDIN_FILENO,&name,MAX);
    message idmessage;
    idmessage.type = 'n';
    idmessage.length = strlen(name);    
    memcpy(idmessage.value,name,40);
    write(sockfd, &idmessage, sizeof(message));

	
    
    // standard input (descriptor 0), and pipe input descriptor.
    for(;;)
    {
	FD_ZERO(&readset);
	FD_SET(STDIN_FILENO, &readset);
	FD_SET(sockfd, &readset);
	message curMessage;

	// Block until data is available to read or 5 seconds have elapsed.

        if ((n=select(sockfd +1,&readset, NULL, NULL,NULL)) > 0){
	    if (FD_ISSET(sockfd,&readset))
            {	    
		memset(&curMessage,0,sizeof(message));
		read(sockfd, &curMessage, sizeof(message));
		if(curMessage.type == 'm'){
			curMessage.sender[strlen(curMessage.sender)-1]='\0';
			printf("%s: %s",curMessage.sender,curMessage.value);
		}
		if(curMessage.type == 'n'){

			curMessage.value[strlen(curMessage.value)-1]='\0';
			printf("%s has joined the chat!\n",curMessage.value);
		}
               // printf("Read so far: "); puts(msg); printf("\n");
	    }if(FD_ISSET(STDIN_FILENO , &readset)){
		message messageToSend;
		memset(&messageToSend,0,sizeof(message));
		parseOutput(&messageToSend);
        	write(sockfd, &messageToSend, sizeof(message));
	    }
        }
        else
        {
            printf("Type faster dude.\n");
        }
    }
}
void parseOutput(message* curmessage ){
	if(fgets(msg,(MAX),stdin) == msg)
	{
	   curmessage->type = 'm';
	   curmessage->length = strlen(msg);
	     
	   memcpy(curmessage->value,msg,MAX);
	   
	}
	else{
	   char temp = 'o';
	   while(temp != '\n' && temp != EOF){
		temp = getchar();
	   }
	}
}


