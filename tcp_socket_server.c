//
// Server side TCP socket program
//

/*
 *  Library Includes
 */

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

/*
 *  Non-library Includes
 */

#include "http_responses.h"

/*
 *  #defines
 */

#define PORT                        0       // Setting PORT to 0 will pick a random port
#define MAX_CONNS                   10
#define MAX_HEADER_REQUEST_LEN      8192    // 8KB max length for HTTP request headers

/*
 *  Function Prototypes
 */

void initialise_server(int *server_fd, struct sockaddr_in *address);
void serve_request(int *server_fd, struct sockaddr_in *address);

/*
 *  Main
 */

int main(void) {
    int server_fd;
    int new_socket;
    struct sockaddr_in address;

    initialise_server(&server_fd, &address);
    printf("\n****** Server is listening on port %d ******\n\n", ntohs(address.sin_port));

    while (1) {
        serve_request(&server_fd, &address);
    }

    return EXIT_SUCCESS;
}

/*
 *  Functions
 */

// Initialises the server
void initialise_server(int *server_fd, struct sockaddr_in *address) {
    // Create socket file descriptor
    if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Setup the socket
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);

    memset(address->sin_zero, '\0', sizeof address->sin_zero);

    // Assign address to the socket
    if (bind(*server_fd, (struct sockaddr*) address, sizeof(*address)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Tell socket to accept incoming connections
    if (listen(*server_fd, MAX_CONNS) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Retrieves the local name for the socket and store it in sockaddr
    socklen_t socklen = sizeof(*address);
    if (getsockname(*server_fd, (struct sockaddr *) address, &socklen) == -1) {
        perror("getsocketname");
        exit(EXIT_FAILURE);
    }
}

// Waits for a connection and serves the response given by the connection
void serve_request(int *server_fd, struct sockaddr_in *address) {
    int socket;
    int addrlen = sizeof(address);

    printf("====== Waiting for a connection ======\n\n");

    // Create a socket to get the response from the connection
    if ((socket = accept(*server_fd, (struct sockaddr*) address, (socklen_t*) &addrlen)) == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Read in the request
    char request[MAX_HEADER_REQUEST_LEN] = {0};
    long bytes_read = read(socket, request, MAX_HEADER_REQUEST_LEN);
    printf("++++++++++++++++ Reading Request ++++++++++++++++\n\n");
    printf("%s", request);
    printf("++++++++++++++++ Finished Reading Request ++++++++++++++++\n\n");

    // Prevent 0 byte reads from crashing the server
    if (bytes_read != 0) {
        send_response(socket, request);
    }

    printf("\n---------------- Sent a Response ----------------\n\n");

    // Destroy the socket
    close(socket);
}
