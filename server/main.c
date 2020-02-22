#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

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
                printf("\nInput option value=%s", port);
                break;
            case 'a':
                address = optarg;
                printf("\nInput option value=%s", address);
                break;
            case '?':
                if (optopt == 'p')
                    error("Missing port number");
                else if (optopt == 'a')
                    error("Missing IP address");
                else
                    error("Invalid option");
            default:
                error("Something is wrong with the arguments. Please check");
        }
    }

    return 0;
}
