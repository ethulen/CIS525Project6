/*-------------------------------------------------------------*/
/* ChatServer4.c - Server for a chat room using nonblocking I/O*/
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

typedef struct User
{
    char username[MAX];
    SSL *ssl;
    int socket;
    char to[MAX], fr[MAX];
    char *toiptr, *friptr;
} User;

int startServer();

char topic[MAX];
char hostaddr[MAX];
int newsockfd, hostSockfd;
unsigned int clilen;
struct sockaddr_in cli_addr, serv_addr;

int main(int argc, char **argv)
{
    struct User *users[MAX_CLIENTS] = {0};
    int connectedClients = 0, maxfd = 0;
    char message[MAX];
    int j, ret;
    int maxfdp1, val, stdineof;
    ssize_t n, nwritten;
    fd_set readset, writeset;
    stdineof = 0;

    SSL_library_init();
    // Initializes Server SSL State
    SSL_METHOD *method;
    SSL_CTX *ctx;
    SSL *ssl;
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    // TODO: Check SSLv2 or SSLv23
    method = SSLv2_server_method();
    ctx = SSL_CTX_new(method);

    // Load Certificate and Private Key Files
    // TODO: Create CertFile based on links in the assignment page
    // SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM);
    // TODO: Create KeyFile based on links in the assignment page
    // SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM);
    // if(!(SSL_CTX_check_private_key(ctx)))
    // {
    //     fprintf(stderr, "Key & certificate don't match");
    // }

    hostSockfd = 0;
    if ((hostSockfd = startServer()) < 1)
    {
        printf("Issue starting server. Closing server...\n");
        exit(0);
    }
    listen(hostSockfd, 5);
    for (;;)
    {
        FD_ZERO(&readset);
        FD_SET(hostSockfd, &readset);

        maxfd = hostSockfd;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (users[i] != 0)
            {
                FD_SET(users[i]->socket, &readset);
                if (users[i]->friptr < &(users[i]->fr[MAX]))
                {
                    FD_SET(users[i]->socket, &writeset);
                }
                if (users[i]->socket > maxfd)
                {
                    maxfd = users[i]->socket;
                }
            }
        }

        if ((j = select(maxfd + 1, &readset, &writeset, NULL, NULL)) > 0)
        {

            if (FD_ISSET(hostSockfd, &readset))
            {
                /* Accept a new connection request. */
                clilen = sizeof(cli_addr);
                newsockfd = accept(hostSockfd, (struct sockaddr *)&cli_addr, &clilen);
                if (newsockfd < 0)
                {
                    perror("server: accept error");
                    exit(1);
                }

                // Create SSL Session State based on context & SSL_accept
                ssl = SSL_new(ctx);
                SSL_set_fd(ssl, newsockfd);
                if (SSL_accept(ssl) == FAIL)
                {
                    ERR_print_errors_fp(stderr);
                }
                else
                {
                    val = fcntl(newsockfd, F_GETFL, 0);
                    fcntl(newsockfd, F_SETFL, val | O_NONBLOCK);

                    /* Read the username from the client. */
                    for (int i = 0; i < MAX_CLIENTS; i++)
                    {
                        if (users[i] != 0 && FD_ISSET(users[i]->socket, &readset))
                        {
                            if ((n = SSL_read(ssl, users[i]->toiptr, ((users[i]->to[MAX]) - users[i]->toiptr[MAX])) > 0))
                            {
                                users[i]->toiptr += n;
                                if (users[i]->toiptr == &(users[i]->to[MAX]))
                                {
                                    printf("Read: %s\n", users[i]->to);
                                    // TODO: check buffer for username
                                    if (users[i]->username[0] == 0)
                                    {
                                        bool dupName = false;
                                        for (int k = 0; k < MAX_CLIENTS; k++)
                                        {
                                            if (users[k] != 0 && (strncmp(users[i]->to, users[k]->username, MAX) == 0))
                                            {
                                                dupName = true;
                                                break;
                                            }
                                        }

                                        if (dupName)
                                        {

                                            snprintf(users[i]->fr, MAX, "%s", "Sorry, that user already exists. Please restart and try a different username!\n");
                                            connectedClients--;
                                            close(users[i]->socket);
                                            free(users[i]);
                                            users[i] = 0;
                                        }
                                        else
                                        {
                                            strncpy(users[i]->username, users[i]->fr, MAX);
                                            bzero(users[i]->username, MAX);
                                        }
                                    }
                                }

                                if (strstr(message, "/e") != NULL)
                                {
                                    connectedClients--;
                                    close(users[i]->socket);

                                    for (int h = 0; h < MAX_CLIENTS && users[h] != 0 && h != i; h++)
                                    {
                                        snprintf(users[h]->fr, MAX, "%s has left the chat\n", users[i]->username);
                                        users[h]->friptr = users[h]->fr;
                                    }
                                    free(users[i]);
                                    users[i] = 0;
                                }
                                else
                                {
                                    for (int h = 0; h < MAX_CLIENTS && users[h] != 0 && h != i; h++)
                                    {
                                        snprintf(users[h]->fr, MAX, "%s:%s", users[i]->username, users[i]->to);
                                        users[h]->friptr = users[h]->fr;
                                    }
                                    users[i]->toiptr = users[i]->to;
                                    // if fr buffer is not empty, will be overwritten
                                }
                            }

                            else if (n < 0)
                            {
                                if (errno != EWOULDBLOCK)
                                    perror("read error on socket");
                            }
                        }
                    }
                }
            }
            int index;
            for (index = 0; index < MAX_CLIENTS; index++)
            {
                if (users[index] == 0)
                {

                    struct User *newUser = calloc(sizeof(struct User), 1);
                    newUser->toiptr = newUser->to; /* initialize buffer pointers */
                    newUser->friptr = &(newUser->fr[MAX]);
                    bzero(newUser->username, MAX);
                    newUser->socket = newsockfd;
                    users[index] = newUser;
                    connectedClients++;
                }

                printf("%d\n", connectedClients);

                if (index == MAX_CLIENTS)
                {

                    snprintf(users[index]->to, MAX, "%s", "There are too many clients connected\n");
                    close(newsockfd);
                    break;
                }

                if (connectedClients == 1)
                {

                    snprintf(users[index]->fr, MAX, "%s", "You are the first user to join the chat!\n");
                }
                else
                {

                    for (int i = 0; i < MAX_CLIENTS && i != index && users[i] != 0; i++)
                    {
                        snprintf(users[i]->fr, MAX, "%s has joined the chat\n", users[index]->username); // FIXME
                    }
                }
            }
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (users[i] != 0 && FD_ISSET(users[i]->socket, &writeset))
                {
                    if ((n = SSL_read(ssl, users[i]->toiptr, ((users[i]->to[MAX]) - users[i]->toiptr[MAX])) > 0))
                    {

                        users[i]->friptr += n;
                        if (users[i]->friptr == &(users[i]->fr[MAX]))
                        {
                            printf("Written: %s\n", users[i]->fr);
                        }
                    }
                    else if (n < 0)
                    {
                        if (errno != EWOULDBLOCK)
                            perror("write error on socket");
                    }
                }
            }
        }
    }
}

int startServer()
{
    // Sets up hosting connection
    /* Create communication endpoint */
    if ((hostSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("server: can't open stream socket");
        exit(1);
    }

    /* Bind socket to local address */
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    if (bind(hostSockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("server: can't bind local address");
        exit(1);
    }

    return hostSockfd;
}