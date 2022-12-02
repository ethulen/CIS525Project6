/*-------------------------------------------------------------*/
/* directoryServer5.c - sample iterative time/date server.               */
/*-------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include "inet.h"
#include <sys/select.h>
typedef struct sock{
	server sockserver;
	int socknumber;
	int active;
}sock;
int main(int argc, char **argv)
{

    //printf("1\n");	
    int                 sockfd,clifd,newsockfd, childpid,n,curmax;
    unsigned int	clilen;
    struct sockaddr_in  cli_addr, serv_addr;
    struct tm           *timeptr;  /* pointer to time structure */
    time_t              clock;     /* clock value (in secs)     */
    char                s[MAX];
    char                request;
    sock               availablesocks[10]={0};
    int 		curServers = 0;
    fd_set readset;
    /* Create communication endpoint */
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server: can't open stream socket");
        exit(1);
    }
    curmax = sockfd;
    /* Bind socket to local address */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(SERV_TCP_PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("server: can't bind local address");
        exit(1);
    }

    if ( (clifd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server: can't open stream socket");
        exit(1);
    }
    curmax = clifd;
    /* Bind socket to local address */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(CLI_TCP_PORT);

    if (bind(clifd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("server: can't bind local address");
        exit(1);
    }
  
    //printf("sockcreated %d\n", sockfd);	
    listen(sockfd, 5);
    listen(clifd,5);

    for ( ; ; ) {
	    
   	 //	printf("2\n");	
	FD_ZERO(&readset);
	//FD_SET(STDIN_FILENO, &readset);
	FD_SET(sockfd, &readset);
	FD_SET(clifd, &readset);
	//printf("3\n");	
	
	for(int i = 0 ; i < curServers; i++){
		
		//printf("3\n");	
		if (availablesocks[i].active == 1){

			//printf("socket%d\n",people[i].sock);	
			FD_SET(availablesocks[i].socknumber,&readset);
		}

	}
	
	//printf("4\n");	
	if ((n=select(curmax +1,&readset, NULL, NULL, NULL))>0){
	  // printf("5a\n");			
	   if( FD_ISSET(sockfd,&readset)){//new server
		///printf("HERE\n");
		struct sockaddr_in socktosend;
		int socksize;
        	newsockfd = accept(sockfd, (struct sockaddr*)&socktosend,&socksize);

        	if (newsockfd < 0) {
            	   perror("server: accept error");
           	   exit(1);
		}
		//FD_SET(newsockfd, &readset);
		if(curmax < newsockfd){
			curmax = newsockfd;
		}
		availablesocks[curServers].active = 1;
		availablesocks[curServers].socknumber = newsockfd;
		//memcpy(availablesocks[curServers].sockserver.ip = socktosend .sin_addr.s_addr;
		//availablesocks[curServers].sockserver.port = socktosend .sin_port;
		curServers ++;

	   }
	   if(FD_ISSET(clifd,&readset)){ //client wants servers	
        	int newclifd = accept(clifd, NULL,NULL);
		for(int i = 0 ; i < curServers; i++){
			if(availablesocks[i].active ==1){
				//printf("SEND!\n");
				write(newclifd,&availablesocks[i].sockserver,sizeof(server));
			}
		}
		close(newclifd);
	   }
	   else{ //server is closing
		
		for(int i = 0 ; i < curServers; i++){
			if(FD_ISSET(availablesocks[i].socknumber,&readset)){
				//server tempserver4;
				if(read(availablesocks[i].socknumber,&availablesocks[i].sockserver,sizeof(server))<= 0){
					availablesocks[i].active = 0;

				
				}
				
				for(int j = 0 ; j < curServers; j++){
					if(i == j)continue;
					if(strncmp(availablesocks[i].sockserver.topic,availablesocks[j].sockserver.topic,sizeof(availablesocks[j].sockserver.topic))== 0)  availablesocks[i].active = 0;

				}
				//memcpy(availablesocks[i].sockserver,tempserver4.topic,sizeof(tempserver4.topic)); 

			}
				
		}
	   }
		
		

	   
	}
							//if(&people[i] != &people[j] && people[j].enrolled == 1){
       								// write(people[j].sock, &curmessage, sizeof(message));
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

