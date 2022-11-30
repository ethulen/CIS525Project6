/*------------------------------------------------------*/
/* chatClient4.c - UI for a chat room                   */
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

int main()
{
    int i, n;
    int sockfd;
    struct sockaddr_in serv_addr;
    char username[MAX];
    char message[MAX];
    fd_set readset;
    unsigned int servlen; /* length of server addr*/

    // Initializes Client SSL State
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    method = SSLv2_client_method();
    ctx = SSL_CTX_new(method);

    // standard input (descriptor 0), and pipe input descriptor.
    i = 0;

    /* Set up the address of the server to be contacted. */
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    serv_addr.sin_port = htons(SERV_TCP_PORT);
    /* Create a socket (an endpoint for communication). */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("client: can't open stream socket");
        exit(1);
    }
    /* Connect to the server. */

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        if (errno != EINPROGRESS)
            exit(1);
        perror("client: can't connect to server");
        exit(1);
    }

    // Establishes SSL protocol and create encryption link
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sd);
    if (SSL_connect(ssl) == -1)
    {
        ERR_print_errors_fp(stderr);
    }

    printf("Please enter your username here: \n");

    if (scanf("%s", username) != 1)
    {
        printf("Invalid username\n");
        exit(1);
    }
    SSL_write(ssl, username, MAX);

    /* Users write responses and send it to the server. */
    for (;;)
    {
        FD_ZERO(&readset);
        FD_SET(STDIN_FILENO, &readset);
        FD_SET(sockfd, &readset);

        if ((n = select(sockfd + 1, &readset, NULL, NULL, NULL)) > 0)
        {

            if (FD_ISSET(STDIN_FILENO, &readset))
            {
                printf("Please enter your message here: \n");
                if (scanf("%s", message) != 1)
                {
                    printf("Invalid message\n");
                }
                else
                {
                    if (strncmp(message, "exit", MAX) != 0)
                    {
                        SSL_write(ssl, message, MAX);
                        bzero(message, MAX);
                    }
                    else
                    {
                        exit(1);
                    }
                }
            }
            if (FD_ISSET(sockfd, &readset))
            {

                char response[MAX] = {0}; /* server response */
                if (SSL_read(ssl, response, MAX) <= 0)
                {
                    fprintf(stderr, "Read error from server or closed connection\n");
                    exit(1);
                }
                printf("%s\n", response);
            }
        }
    }
}