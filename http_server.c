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
#include <pthread.h>
#include <fcntl.h>
#include <assert.h>

/*
 *  Non-library Includes
 */

#include "http_responses.h"

/*
 *  #defines
 */

#define DEFAULT_PORT                0       // Setting PORT to 0 will pick a random port
#define MAX_CONNS                   10      // Max queing pending connections allowed
#define MAX_HEADER_REQUEST_LEN      8192    // 8KB max length for HTTP request headers

/*
 *  Function Prototypes
 */

void initialise_server(int *server_fd, struct sockaddr_in *address, int port_num);
void setup_for_connections(int *server_fd, struct sockaddr_in *address);
void *serve_request(void* socket_ptr);

/*
 *  Main
 */

int main(int argc, char *argv[]) {
    if (argc == 2) {
        if ((!atoi(argv[1]) && (strcmp(argv[1], "0") != 0)) || atoi(argv[1]) > 65535 || atoi(argv[1]) < 0) {
            fprintf(stderr, "Usage: %s <port number from 0-65535, 0 for random port>\n", argv[0]);
            fprintf(stderr, "Alternate Usage: %s\n", argv[0]);
            return EXIT_FAILURE;
        }
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s <port number from 0-65535, 0 for random port>\n", argv[0]);
        fprintf(stderr, "Alternate Usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    int server_fd;
    struct sockaddr_in address;

    int port_num;
    if (argc != 1)
        port_num = atoi(argv[1]);
    else
        port_num = DEFAULT_PORT;

    initialise_server(&server_fd, &address, port_num);
    printf("\n*************** Server is listening on port %d ****************\n", ntohs(address.sin_port));
    printf("Access your website thorugh this link: http://127.0.0.1:%d\n", ntohs(address.sin_port));
    printf("Or through this link: http://localhost:%d\n\n", ntohs(address.sin_port));

    while (1) {
        setup_for_connections(&server_fd, &address);
    }

    return EXIT_SUCCESS;
}

/*
 *  Functions
 */

// Initialises the server
void initialise_server(int *server_fd, struct sockaddr_in *address, int port_num) {
    // Create socket file descriptor
    if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Setup the socket
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(port_num);

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

// Setup the server to listen for connections and create threads to serve requests
void setup_for_connections(int *server_fd, struct sockaddr_in *address) {
    int socket;
    int addrlen = sizeof(address);

    printf("==================== Waiting for a connection ====================\n\n");

    // Create a socket to get the response from the connection
    if ((socket = accept(*server_fd, (struct sockaddr*) address, (socklen_t*) &addrlen)) == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Malloc socket fd for the thread
    int *socket_cpy = malloc(sizeof(int));
    assert(socket_cpy != NULL);
    *socket_cpy = socket;
    
    // Create a thread and let it handle serving this request
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, serve_request, socket_cpy) < 0) {
        fprintf(stderr, "pthread_create: could not create thread\n");
    }
}

// Threaded function to serve requests
void *serve_request(void* data) {
    // Stop this thread on return
    pthread_detach(pthread_self());

    int socket = *((int *) data);

    // Read in the request
    char request[MAX_HEADER_REQUEST_LEN] = {0};
    long bytes_read = read(socket, request, MAX_HEADER_REQUEST_LEN);
    printf("++++++++++++++++++++++++ Reading Request +++++++++++++++++++++++++\n\n");
    printf("%s", request);
    printf("\n++++++++++++++++++++ Finished Reading Request ++++++++++++++++++++\n\n");

    // Prevent 0 byte reads from crashing the server
    if (bytes_read != 0)
        send_response(socket, request);

    printf("\n------------------------ Sent a Response -------------------------\n\n");

    // Destroy the socket and stop thread
    close(socket);
    free(data);
    return NULL;
}
