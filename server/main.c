#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>	//inet_addr
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

//#define SEGMENT 4096
#define SEGMENT 1024

void error(char *exit_msg) {
    perror(exit_msg);
    exit(errno);
}

int file_exist (char *filename)
{
    struct stat   buffer;
    int k = stat (filename, &buffer);
    printf("size: %ld %d\n", buffer.st_size, k == 0);
    return (k == 0);
}

char *dir = NULL;

int send_segments(int sock, int segments, int file) {
    int read_size;
    char msg[SEGMENT * segments];
    if ((read_size = read(file, msg, segments * SEGMENT)) > 0) {
        write(sock, msg, read_size);
        if (read_size == 0 || read_size < segments * SEGMENT) {
            write(sock, "DONE", 4);
            if (close(file) != 0)
                error("Couldn't close file");
        }
    }
    return read_size;
}
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc, read_size, segments = 0, file = 0;
    char client_message[200], *states[4] = {"FILE", "CONF", "CONT", "EXIT"}, state = 0,
         *responses[2] = {"N", "Y"}, path[1000];

    //Receive a message from client
    while((read_size = (recv(sock , client_message , 200 , 0))) > 0 )
    {
        puts(client_message);
        if (strncmp(states[state], client_message, 4) == 0) {
            if (state == 0) {

                strcpy(path, dir);
                strcpy(path + strlen(path), client_message + 6);
                path[strlen(path) - 1] = '\0';
                puts(path);
                write(sock , responses[file_exist(path)] , strlen(path));

                state = 1;
            } else if (state == 1) {
                if ((read_size = recv(sock , client_message , 200 , 0)) < 0)
                    break;
                segments = atoi(client_message);
                if((file = open(path, O_RDONLY)) == -1) {
                    error("Could not open file");
                    break;
                }

;               read_size = send_segments(sock, segments, file);

                if (read_size == -1) {
                    perror("breaking 1");
                    break;
                }

                state = 2;
            } else {
                read_size = send_segments(sock, segments, file);

                if (read_size == -1) {
                    puts("breaking 2");
                    break;
                }

                if (read_size == 0 || read_size < segments * SEGMENT)
                    state = 0;
                else
                    state = 2;
            }
            memset(client_message, 0, 200);
        } else if (strncmp(states[3], client_message, read_size) == 0)
            break;
        else {
            strcpy(client_message, "Wrong protocol, exiting ...");
            write(sock , client_message , read_size);
            break;
        }
    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }

    //Free the socket pointer
    free(socket_desc);

    return 0;
}

int main(int argc, char *argv[]) {
    char *port = NULL;

    if (argc <= 1)
        error("The command had no arguments.\n");

    int opt = 0;
    while ((opt = getopt(argc, argv, "p:f:")) != -1) {
        switch (opt) {
            case 'p':
                port = optarg;
                printf("Input option value=%s\n", port);
                break;
            case 'f':
                dir = optarg;
                printf("Input option value=%s\n", dir);
                break;
            case '?':
                if (optopt == 'p')
                    error("Missing port number\n");
                else if (optopt == 'a')
                    error("Missing IP address\n");
                else
                    error("Invalid option\n");
            default:
                error("Something is wrong with the arguments. Please check\n");
        }
    }

    /*  Address Family - AF_INET (this is IP version 4)
    Type - SOCK_STREAM (this means connection oriented TCP protocol)
    Protocol - 0 [ or IPPROTO_IP This is IP protocol]*/
    int socket_desc, new_socket;
    struct sockaddr_in server, client;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
        error("Could not create socket\n");

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(port) );

    //Connect to remote server

    if (bind(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
        error("Bind error\n");

    puts("Bind done");

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    unsigned int c = sizeof(struct sockaddr_in);

    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");

        //Reply to the client

        pthread_t sniffer_thread;
        int *new_sock = malloc(1);
        *new_sock = new_socket;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
            error("Could not create thread");

        //Now join the thread , so that we dont terminate before the thread
        pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }

    new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (new_socket == -1)
        error("Accept failed");

    puts("Connection accepted");

    // Finish connection when ready
    close(socket_desc);
    return 0;
}
