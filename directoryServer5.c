/*-------------------------------------------------------------*/
/* server.c - sample iterative time/date server.               */
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

typedef struct Server
{
    char topic[MAX];
    char ip[MAX];
    int port;
    int socket;
} Server;

bool determineIsServer(char target[], char message[]);
int hostDirectory();

struct sockaddr_in cli_addr, serv_addr;
int sockfd, newsockfd;
unsigned int clilen;

int main(int argc, char **argv)
{
    int maxInArray;
    int curServers = 0;
    char message[MAX];
    char s[MAX];
    struct Server servers[MAX_SERVERS];
    fd_set readset;
    int j, ret;
    bool isServer;

    for (int z = 0; z < MAX_SERVERS; z++)
    {
        struct Server newServer;
        strcpy(newServer.ip, "");
        newServer.port = 0;
        strcpy(newServer.topic, "");
        newServer.socket = 0;
        servers[z] = newServer;
    }

    int directoryStart = hostDirectory();
    if (directoryStart != 1)
    {
        printf("Directory unable to start. Please try again!\n");
        exit(0);
    }

    for (;;)
    {
        FD_ZERO(&readset);
        FD_SET(sockfd, &readset);
        maxInArray = sockfd;

        for (int z = 0; z < MAX_SERVERS; z++)
        {
            if (servers[z].socket != 0)
            {
                FD_SET(servers[z].socket, &readset);
                if (servers[z].socket > maxInArray)
                {
                    maxInArray = servers[z].socket;
                }
            }
        }

        if ((j = select(maxInArray + 1, &readset, NULL, NULL, NULL)) > 0)
        {
            for (int i = 0; i < MAX_SERVERS; i++)
            {
                if (FD_ISSET(servers[i].socket, &readset))
                {
                    bzero(message, MAX);
                    bzero(s, MAX);
                    if (((ret = read(servers[i].socket, &message, MAX)) > 0))
                    {
                        isServer = determineIsServer(s, message);
                        if ((isServer) && (strncmp(s, "/e", MAX) == 0))
                        {
                            snprintf(s, MAX, "%s", "Server has been taken off the list of available servers! Have a nice day!\n");
                            write(servers[i].socket, s, MAX);
                            close(servers[i].socket);
                            strcpy(servers[i].ip, "");
                            servers[i].port = 0;
                            strcpy(servers[i].topic, "");
                            servers[i].socket = 0;
                        }
                    }
                }
            }
            if (FD_ISSET(sockfd, &readset))
            {
                /* Accept a new connection request. */
                clilen = sizeof(cli_addr);
                newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
                if (newsockfd < 0)
                {
                    perror("server: accept error");
                    exit(1);
                }

                // Resetting variables
                bzero(message, MAX);
                bzero(s, MAX);
                isServer = false;

                /* Read the message from the client. */
                read(newsockfd, &message, MAX);
                isServer = determineIsServer(s, message);

                // Handles if a client or a server
                if (isServer) // Handles a server request
                {
                    struct Server newServer;

                    strcpy(newServer.ip, inet_ntoa(cli_addr.sin_addr));
                    if (sscanf(s, "%s %d", &newServer.topic, &newServer.port) != 2)
                    {
                        printf("Issue reading message from server.\n");
                    }

                    bool dupName = false;
                    bool dupPort = false;
                    for (int z = 0; z < MAX_SERVERS; z++)
                    {
                        if (strncmp(newServer.topic, servers[z].topic, MAX) == 0)
                        {
                            dupName = true;
                        }
                        if ((newServer.port == servers[z].port) && (strncmp(newServer.ip, servers[z].ip, MAX) == 0))
                        {
                            dupPort = true;
                        }
                    }
                    if (dupName)
                    {
                        snprintf(s, MAX, "%s", "Sorry, that server name already exists. Please try again with a different server name.\n");
                        write(newsockfd, s, MAX);
                        close(newsockfd);
                    }
                    else if (dupPort)
                    {
                        snprintf(s, MAX, "%s", "Choose a port that is not already occupied and try again.\n");
                        write(newsockfd, s, MAX);
                        close(newsockfd);
                    }
                    else
                    {
                        bool added = false;
                        int index = 0;
                        newServer.socket = newsockfd;
                        while ((!added) && (index < MAX_SERVERS))
                        {
                            if (servers[index].port == 0)
                            {
                                servers[index] = newServer;
                                added = true;
                            }
                            else
                            {
                                index++;
                            }
                        }
                        curServers++;
                        snprintf(s, MAX, "%s", newServer.ip);
                        write(newsockfd, s, MAX);
                    }
                }
                else // Handles a client request
                {
                    if (curServers == 0)
                    {
                        snprintf(s, MAX, "ds%s", "No servers to connect to! Please try reconnecting when servers are available!\n");
                        write(newsockfd, s, MAX);
                        close(newsockfd);
                    }
                    else
                    {
                        for (int z = 0; z < MAX_SERVERS; z++)
                        {
                            if (servers[z].socket != 0)
                            {
                                bzero(s, MAX);
                                snprintf(s, MAX, "ds%s %s %d\n", servers[z].topic, servers[z].ip, servers[z].port);
                                write(newsockfd, s, MAX);
                            }
                        }
                        snprintf(s, MAX, "ds%s\n", "/e");
                        write(newsockfd, s, MAX);
                        close(newsockfd);
                    }
                }
            }
        }
        else
        {
            snprintf(s, MAX, "Nothing to read. \n");
        }
    }
}

int hostDirectory()
{
    // Sets up hosting connection
    /* Create communication endpoint */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("server: can't open stream socket");
        exit(1);
    }

    /* Bind socket to local address */
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("server: can't bind local address");
        exit(1);
    }

    listen(sockfd, 5);
    return 1;
}

bool determineIsServer(char target[MAX], char message[MAX])
{
    if (message[0] == 'c' && message[1] == 's')
    {
        for (int i = 2; i < MAX; i++)
        {
            target[i - 2] = message[i];
        }
        return true;
    }
    else
    {
        for (int i = 2; i < MAX; i++)
        {
            target[i - 2] = message[i];
        }
        return false;
    }
}
