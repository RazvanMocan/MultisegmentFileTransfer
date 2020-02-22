#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>	//inet_addr
#include <string.h>

void error(char *exit_msg) {
    perror(exit_msg);
    exit(errno);
}

int main(int argc, char *argv[]) {
    char *port = NULL, *address = NULL;

    if (argc <= 1)
        error("The command had no arguments.\n");

    int opt = 0;
    while ((opt = getopt(argc, argv, "p:a:")) != -1) {
        switch (opt) {
            case 'p':
                port = optarg;
                printf("Input option value=%s\n", port);
                break;
            case 'a':
                address = optarg;
                printf("Input option value=%s\n", address);
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

/*    Address Family - AF_INET (this is IP version 4)
    Type - SOCK_STREAM (this means connection oriented TCP protocol)
    Protocol - 0 [ or IPPROTO_IP This is IP protocol]*/
    int socket_desc;
    struct sockaddr_in server;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
        error("Could not create socket\n");

    server.sin_addr.s_addr = inet_addr(address);
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(port) );

    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
        error("Connect error\n");

    puts("Connected");

    char server_reply[2000];
    int reply_size;
    while ( (reply_size = recv(socket_desc, server_reply , 2000 , 0)) > 0)
    {
        server_reply[reply_size] = '\0';
        printf("Server reply: %s\n",  server_reply);

        char *line;
        size_t size;
        if ( getline(&line, &size, stdin) < 0)
            break;

        if( send(socket_desc , line , size , 0) < 0)
        {
            puts("Send failed");
            break;
        }
    }

    if (reply_size < 0)
        puts("recv failed");

    // Finish connection when ready
    close(socket_desc);
    return 0;
}
