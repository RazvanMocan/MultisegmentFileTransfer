#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>	//inet_addr
#include <string.h>
#define MAXSIZE 1024

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
                printf("Input option value as=%s\n", port);
                break;
            case 'a':
                address = optarg;
                printf("Input option value ds=%s\n", address);
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
    char *token;
    int currPosition = 0;
    char transferFile[4096 * MAXSIZE];
    static const char filename[] = "/home/razvan/Facultate/4th_year/2nd sem/DP/1st Project/client/config.txt";
    FILE *file = fopen ( filename, "r" );
    char lineArray[3][128];// [0] - fileName, [1] - segmentSize, [2] - Servers separated by coma
    int i=0;
    if ( file != NULL )
    {
        char line [ 128 ]; /* or other suitable maximum line size */
        while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
        {
            strcpy(lineArray[i], line);
            i++;
            puts( line ); /* write the line */
            if(i==3) break;
        }
        fclose ( file );
    }
    else
    {
        perror ( filename ); /* why didn't the file open? */
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

    const char s[2] = " ";
    token = strtok(lineArray[2], s);
    while( token != NULL )
    {

        token = strtok(NULL, s);
        server.sin_port = htons( atoi(token) );

        //Connect to remote server
        if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
            error("Connect error\n");

        puts("Connected");

        char server_reply[2000];
        int reply_size;
        char commandName[] = "FILE\n";

        char commandSegSize[] = "CONF\n";

        char commandContinueSending[] = "CONT\n";

        char commandExit[] = "EXIT\n";


        if(send(socket_desc, commandName, sizeof(commandName), 0) < 0)
        {
            puts("Send failed");
            break;
        }
        if(send(socket_desc, lineArray[0], sizeof(lineArray[0]), 0) < 0)
        {
            puts("Send failed");
            break;
        }

        reply_size = recv(socket_desc, server_reply , 2000 , 0);
        if(server_reply[0] == 'N')
        {

            close(socket_desc);
            continue;

        }
        else if(server_reply[0] == 'Y')
        {



            if(send(socket_desc, commandSegSize, sizeof(commandSegSize), 0) < 0)
            {
                puts("Send failed");
                break;
            }
            if(send(socket_desc, lineArray[1], sizeof(lineArray[1]), 0) < 0)
            {
                puts("Send failed");
                break;
            }

            while((reply_size = recv(socket_desc, server_reply , 2000 , 0)) != 4 /*&& server_reply != "DONE"*/ && currPosition < 2 * 4096 * MAXSIZE)
            {

                memcpy((transferFile + currPosition), server_reply, reply_size);
                currPosition+=reply_size;
                printf("pos: %d %d\n", currPosition, reply_size);
                if(send(socket_desc, commandContinueSending, strlen(commandContinueSending), 0) < 0)
                {
                    puts("Send failed");
                    break;
                }
                if (reply_size < 2000)
                    break;

            }
        }

        //Mai trimitem EXIT?
        //send(socket_desc, commandName, sizeof(commandExit));
        //send(socket_desc, lineArray[0], sizeof(commandExit));

        // Finish connection when ready





        close(socket_desc);
        FILE *fp;
        fp=fopen(lineArray[0], "wb");
        fwrite(transferFile, 1, currPosition, fp);
        printf("Successful file transfer\n");
        break;
    }
    return 0;
}
