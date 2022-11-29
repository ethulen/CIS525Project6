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

typedef struct User
{
    char username[MAX];
    int socket;
    char to[MAX], fr[MAX];
    char *toiptr, *friptr;
} User;

int startServer();
void sighandler(int signo);
int connectToDirectory();

char topic[MAX];
char hostaddr[MAX];
int port;
int sockfd, newsockfd, hostSockfd;
unsigned int clilen;
struct sockaddr_in cli_addr, serv_addr;

int main(int argc, char **argv)
{
    struct User *users[MAX_CLIENTS] = {0};
    int connectedClients = 0, maxfd = 0;
    char s[MAX], dmsg[MAX];
    char message[MAX];
    int j, ret;
    int maxfdp1, val, stdineof;
    ssize_t n, nwritten;
    fd_set readset, writeset;

    stdineof = 0;
    signal(SIGINT, sighandler);

    if (argc == 3)
    {
        // Checks that the port is valid
        if ((sscanf(argv[2], "%d") != 1) || (atoi(argv[2]) > 65535 || atoi(argv[2]) < 1024))
        {
            printf("Please choose a valid port number.\n");
            exit(0);
        }
        port = atoi(argv[2]);
        snprintf(topic, MAX, "%s", argv[1]);
    }
    else
    {
        printf("Please try to reinput your information as the following: \"<topic>\" <port number>. Thank you.\n");
        exit(0);
    }

    /* Starts connection to the directory */
    int serv_start = connectToDirectory();
    if (serv_start != 1)
    {
        printf("Server unable to start. Please try again!\n");
        exit(0);
    }
    snprintf(s, MAX, "cs%s %d", topic, port);
    write(sockfd, s, MAX);

    /* Starts the server hosting */
    hostSockfd = 0;
    // if ((hostSockfd = startServer()) < 1)
    // {
    //     printf("Issue starting server. Closing server...\n");
    //     exit(0);
    // }
    // listen(hostSockfd, 5);

    for (;;)
    {
        FD_ZERO(&readset);
        FD_SET(sockfd, &readset);
        maxfd = sockfd;
        if (hostSockfd != 0)
        {
            FD_SET(hostSockfd, &readset);
            if (hostSockfd > maxfd)
            {
                maxfd = hostSockfd;
            }
        }
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
            fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
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

                fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
                val = fcntl(newsockfd, F_GETFL, 0);
                fcntl(newsockfd, F_SETFL, val | O_NONBLOCK);

                /* Read the username from the client. */
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (users[i] != 0 && FD_ISSET(users[i]->socket, &readset))
                    {
                        if ((n = read(users[i]->socket, users[i]->toiptr, &(users[i]->to[MAX]) - users[i]->toiptr)) > 0)
                        {
                            fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
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
                                        fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
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
                                fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
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
            int index;
            for (index = 0; index < MAX_CLIENTS; index++)
            {
                if (users[index] == 0)
                {
                    fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
                    struct User *newUser = calloc(sizeof(struct User), 1);
                    newUser->toiptr = newUser->to; /* initialize buffer pointers */
                    newUser->friptr = &(newUser->fr[MAX]);
                    bzero(newUser->username, MAX);
                    // strncpy(newUser->username, newUser->fr, MAX);
                    newUser->socket = newsockfd;
                    users[index] = newUser;
                    connectedClients++;
                }

                printf("%d\n", connectedClients);

                if (index == MAX_CLIENTS)
                {
                    fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
                    snprintf(users[index]->to, MAX, "%s", "There are too many clients connected\n");
                    close(newsockfd);
                    break;
                }

                if (connectedClients == 1)
                {
                    fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
                    snprintf(users[index]->fr, MAX, "%s", "You are the first user to join the chat!\n");
                }
                else
                {
                    fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);

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
                    if ((n = write(users[i]->socket, users[i]->friptr, &(users[i]->fr[MAX]) - (users[i]->friptr))) > 0)
                    {
                        fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
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
            if (FD_ISSET(sockfd, &readset))
            {
                bzero(s, MAX);
                bzero(dmsg, MAX);
                if (read(sockfd, &dmsg, MAX) > 0)
                {
                    if (strstr(dmsg, "Sorry, that server name already exists") != NULL)
                    {
                        close(sockfd);
                        printf("%s", dmsg);
                        printf("Exiting now...\n");
                        exit(0);
                    }
                    if (strstr(dmsg, "Please choose a port that is not already occupied") != NULL)
                    {
                        close(sockfd);
                        printf("%s", dmsg);
                        printf("Exiting now...\n");
                        exit(0);
                    }

                    if (sscanf(dmsg, "%s", hostaddr) != 1)
                    {
                        printf("Issue reading address from directory\n");
                    }
                    else
                    {
                        printf("%s Server has been registered with the directory.\n", topic);
                        if ((hostSockfd = startServer()) != 1)
                        {
                            printf("Issue starting server. Closing server...\n");
                            exit(0);
                        }
                    }
                }
            }
        }
    }
}

int connectToDirectory()
{
    /* ALL DIRECTORY CONNECTION HANDLED HERE */
    // Set up the address of the server to be contacted.
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("client: can't open stream socket");
        exit(1);
    }

    /* Connect to the server. */
    if (connect(sockfd, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        perror("client: can't connect to server");
        exit(1);
    }

    return 1;
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
    serv_addr.sin_port = htons(port);

    if (bind(hostSockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("server: can't bind local address");
        exit(1);
    }

    printf("Server started with topic \"%s\"\n", topic);
    listen(hostSockfd, 5);
    return hostSockfd;
}

void sighandler(int signo)
{
    write(sockfd, "cs/e", MAX);
    printf("Server with topic \"%s\" closing...\n", topic);
    exit(0);
}