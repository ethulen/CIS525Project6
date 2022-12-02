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
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

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
   
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if(!ctx){
        fprintf(stderr, "SSL_CTX_new() failed./n");
        exit(1);
    }
    /*if (!SSL_set_tlsext_host_name(ssl, hostname)){
        fprintf(stderr,"SSL_set_tlsext_host_name() failed. \n");
        ERR_print_errors_fp(stderr);
        exit(1);
    }
    */
    
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
    scanf( " %s",name);
    message idmessage;
    idmessage.type = 'n';
    //bug for concat
    idmessage.length = strlen(name);    
    memcpy(idmessage.value,name,40);
    if (!ctx){
        fprintf(stderr, "SSL_ctx_new() failed.\n");
        exit(1);
    }
    SSL *ssl = SSL_new(ctx);
    if (!ssl){
        fprintf(stderr, "SSL_new() failed.\n");
        exit(1);
    }
    SSL_set_fd(ssl, sockfd);
    if(SSL_connect(ssl) == -1){
        fprintf(stderr, "SSL_connect() failed.\n");
        ERR_print_errors_fp(stderr);
        exit(1);
    }
    
    // Gets certificate issuer
    // if(tmp = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0))
    // {
    //     OPENSSL_free(tmp);
    // }
    
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
                    int readcheck = SSL_read(ssl, &curMessage, sizeof(message));
                    if(readcheck <= 0){
                        printf("error with write\n");
                        exit(0);
                    }
                }
                    //add fail check for read
                    if(curMessage.type == 'm'){
                        curMessage.sender[strlen(curMessage.sender)-1]='\0';
                        printf("%s: %s",curMessage.sender,curMessage.value);
                    }
                if(curMessage.type == 'n'){

                    curMessage.value[strlen(curMessage.value)-1]='\0';
                    printf("%s has joined the chat!\n",curMessage.value);
                }
                // printf("Read so far: "); puts(msg); printf("\n");
            }
        if(FD_ISSET(STDIN_FILENO , &readset)){
            message messageToSend;
            memset(&messageToSend,0,sizeof(message));
            parseOutput(&messageToSend);
            int writecheck=SSL_write(ssl, &messageToSend, sizeof(message));
            if(writecheck <= 0){
                printf("error with write\n");
                exit(0);
            }
            //add error check
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


