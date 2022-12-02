/*-------------------------------------------------------------*/
/* ChatServer5.c - Server for a chat room using nonblocking I/O*/
/*-------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include "inet.h"
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
typedef struct person{
	char name[100];
	int enrolled;
	int sock;
}person;
int main(int argc, char **argv)
{
	
    //printf("1\n");	
    int                 servfd,sockfd, newsockfd, childpid,n,curmax;
    unsigned int	clilen;
    struct sockaddr_in  cli_addr, serv_addr;
    struct tm           *timeptr;  /* pointer to time structure */
    time_t              clock;     /* clock value (in secs)     */
    char                s[MAX];
    char                request;
    person               people[10]={0};
    int 		curUsers = 0;
    unsigned short port;
    char        ip [20];
    char 	theme [99];	
    fd_set readset;
    snprintf(theme,99,"%s",argv[2]);
    memcpy(ip,SERV_HOST_ADDR,(sizeof(SERV_HOST_ADDR)+1));
    sscanf(argv[1],"%hd",&port);
    /* Create communication endpoint */
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server: can't open stream socket");
        exit(1);
    }
    curmax = sockfd;
    /* Bind socket to local address */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port        = htons(port);
    //printf("%d\n",inet_addr(ip));
    //printf("%d\n",htons(port));
    //printf("%d\n",htonl(*SERV_HOST_ADDR));
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("server: can't bind local address");
        exit(1);
    }
     memset((char *) &serv_addr, 0, sizeof(serv_addr));
     serv_addr.sin_family      = AF_INET;
     serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
     serv_addr.sin_port        = htons(SERV_TCP_PORT);

     
     //printf("b\n");		
     /* Create a socket (an endpoint for communication). */
     if ( (servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
             perror("client: can't open datagram socket");
             exit(1);
     }

         /* Bind the client's socket to the client's address */
         if (connect(servfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	         perror("client: can't bind local address");
	         exit(1);
	 }

    //printf("c\n");		
	 server sendingserver;
	 sendingserver.ip = inet_addr(ip);
	 sendingserver.port = htons(port);
	 memcpy(sendingserver.topic,theme,100);
	 write(servfd,&sendingserver,sizeof(server));
  
    //printf("sockcreated %d\n", sockfd);	
    listen(sockfd, 5);

    for ( ; ; ) {
	    
   	 //	printf("2\n");	
	FD_ZERO(&readset);
	//FD_SET(STDIN_FILENO, &readset);
	FD_SET(sockfd, &readset);
	//printf("3\n");	
	
	for(int i = 0 ; i < curUsers; i++){
		
		//printf("3\n");	
		person	tempperson = people[i];
		//printf("socket%d\n",people[i].sock);	
		FD_SET(people[i].sock,&readset);

	}
	
	//printf("4\n");	
	if ((n=select(curmax +1,&readset, NULL, NULL, NULL))>0){
	  // printf("5a\n");			
	   if( FD_ISSET(sockfd,&readset)){
		
        	clilen = sizeof(cli_addr);
        	newsockfd = accept(sockfd, NULL,NULL);

        	if (newsockfd < 0) {
            	   perror("server: accept error");
           	   exit(1);
		}
		//FD_SET(newsockfd, &readset);
		if(curmax < newsockfd){
			curmax = newsockfd;
		}
		//socks[curUsers] = newsockfd;
		//person tempperson = { 
		//		      . enrolled = 0,
		//		      . sock = newsockfd};
		
		//printf("%d\n",tempperson.sock);
		//printf("%d\n",newsockfd);	
		//memcpy(people[curUsers], tempperson, sizeof(person));
		people[curUsers].enrolled = 0;
		
		people[curUsers].sock = newsockfd;
		curUsers ++;
		
	//	close(sockfd);
	//	open(sockfd);
	   }
	   else{
		//printf("5\n");			
		for(int i = 0 ; i < curUsers; i++){
			//person	tempperson = people[i];
			if(FD_ISSET((people[i].sock),&readset)){
				message curmessage;
       				read((people[i].sock), &curmessage, sizeof(message));
				memcpy(curmessage.sender,people[i].name, sizeof(curmessage.sender));
				//printf("%s\n",curmessage.sender);

				//printf("%s\n",people[i].name);
				if(curmessage.type == 'n'){
				//	snprintf(curmessage.value,MAX,"%s has joined the chat!",curmessage.value);
					
					int namecheck = 0;
					for(int j = 0 ; j < curUsers; j++){
						//person	tempperson2 = people[i];
						if(strncmp(people[j].name,curmessage.value,MAX)== 0){
							namecheck = 1;
						}
					}
					if( namecheck == 0){
						memcpy(people[i].name,curmessage.value,MAX);
						people[i].enrolled = 1;
						
						for(int j = 0 ; j < curUsers; j++){	
							//person	tempperson3 = people[i];
							if(&people[i] != &people[j] && people[j].enrolled == 1){
       								 write(people[j].sock, &curmessage, sizeof(message));

							
							}
						}

					}
				}
				else if(curmessage.type == 'm'){
					if(people[i].enrolled == 1){
						for(int j = 0 ; j < curUsers; j++){	
							//person	tempperson2 = people[i];
							if(&people[j] != &people[i] && people[j].enrolled == 1){
       								 write(people[j].sock, &curmessage, sizeof(message));
							}
						}
					}
				}
			}
		}
	   }
	}
	else{
		perror("issue with select");
		exit(1);
	}
        /* Accept a new connection request. */
	
        /* Read the request from the client. */

        /* Generate an appropriate reply. */
	//close(newsockfd);
    }
}
