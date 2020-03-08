/*
 *  Library Includes
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 *  Non-library Includes
 */

#include "http_responses.h"


/*
 *  #defines
 */

#define MAX_HEADER_RESPONSE_LEN     8192    // 8KB max length for HTTP response headers
#define MAX_BUFF_LEN                1024    // 1KB buffer size

/*
 *  Helper Function Prototypes
 */

static int check_permission(char *filepath);
void print_content_type(int socket_fd, char *filename);
static void send_file_contents(int socket_fd, FILE *fp);

/*
 *  Functions
 */

void send_response(int socket_fd, char *request_headers) {
    char *temp = malloc(strlen(request_headers) + 1);
    strcpy(temp, request_headers);
    char *http_method = strtok(temp, " ");

    if (strcmp(http_method, "GET") == 0) {
        // Get the filename
        char *file = strtok(NULL, " ");
        handle_get_response(socket_fd, file);
    } else {
        send_400_bad_request(socket_fd);
    }
    free(temp);
}

void send_404_response(int socket_fd) {
    char header[MAX_HEADER_RESPONSE_LEN];
    snprintf(header, MAX_HEADER_RESPONSE_LEN, "HTTP/1.1 404 Not Found\n"
                                              "Content-Type: text/html\n"
                                              "Content-Length: 13\n\n"
                                              "404 Not Found\n");
    write(socket_fd, header, strlen(header));
}

void send_403_response(int socket_fd) {
    char header[MAX_HEADER_RESPONSE_LEN];
    snprintf(header, MAX_HEADER_RESPONSE_LEN, "HTTP/1.1 403 Forbidden\n"
                                              "Content-Type: text/html\n"
                                              "Content-Length: 13\n\n"
                                              "403 Forbidden\n");
    write(socket_fd, header, strlen(header));
}

void send_400_bad_request(int socket_fd) {
    char header[MAX_HEADER_RESPONSE_LEN];
    snprintf(header, MAX_HEADER_RESPONSE_LEN, "HTTP/1.1 400 Bad Request\n"
                                              "Content-Type: text/html\n"
                                              "Content-Length: 15\n\n"
                                              "400 Bad Request\n");
    write(socket_fd, header, strlen(header));
}

// NOTE: Content-Length header response must be very accurate otherwise
// nothing will show
void handle_get_response(int socket_fd, char *file) {
    if (strcmp(file, "/") == 0)
        file = "index.html";
    else
        // Get the first slash out of the filename
        file += 1;

    FILE *fp = fopen(file, "r");

    if (fp == NULL || check_permission(file) == -1) {
        // Return 404 status code if file not found or check_permission fails
        send_404_response(socket_fd);
        return;
    } else if (check_permission(file) == 0) {
        // Return 403 Forbidden if user has no permissions
        send_403_response(socket_fd);
        return;
    }

    print_content_type(socket_fd, file);
    send_file_contents(socket_fd, fp);
    fclose(fp);
}

/*
 *  Helper Functions
 */

// Returns 1 if the file has Read permissions for Others, 0 for no permission and
// -1 if stat fails
static int check_permission(char *filepath) {
    struct stat s;
    if (stat(filepath, &s) != 0) {
        return -1;
    }

    if ((s.st_mode & S_IRWXO) & S_IROTH) {
        return 1;
    }
    return 0;
}

// Prints the appropriate Content-Type header to the socket
void print_content_type(int socket_fd, char *filename) {
    char buffer[MAX_HEADER_RESPONSE_LEN];

    if (strstr(filename, ".html")) {
        snprintf(buffer, MAX_HEADER_RESPONSE_LEN, "HTTP/1.1 200 OK\n"
                                                  "Content-Type: text/html\n");
        write(socket_fd, buffer, strlen(buffer));
    } else if (strstr(filename, ".txt")) {
        snprintf(buffer, MAX_HEADER_RESPONSE_LEN, "HTTP/1.1 200 OK\n"
                                                  "Content-Type: text/plain\n");
        write(socket_fd, buffer, strlen(buffer));
    } else if (strstr(filename, ".jpg") || strstr(filename, ".jpeg")) {
        snprintf(buffer, MAX_HEADER_RESPONSE_LEN, "HTTP/1.1 200 OK\n"
                                                  "Content-Type: image/jpeg\n");
        write(socket_fd, buffer, strlen(buffer));
    } else if (strstr(filename, ".gif")) {
        snprintf(buffer, MAX_HEADER_RESPONSE_LEN, "HTTP/1.1 200 OK\n"
                                                  "Content-Type: image/gif\n");
        write(socket_fd, buffer, strlen(buffer));
    } else {
        snprintf(buffer, MAX_HEADER_RESPONSE_LEN, "HTTP/1.1 200 OK\n"
                                                  "Content-Type: application/octet-stream\n");
        write(socket_fd, buffer, strlen(buffer));
    }
}

// Reads and returns the contents of the file, returns NULL if failed or if file is empty
static void send_file_contents(int socket_fd, FILE *fp) {
    char buffer[MAX_BUFF_LEN];

    // Get the length of the file
    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    snprintf(buffer, MAX_BUFF_LEN, "Content-Length: %ld\n\n", length);
    write(socket_fd, buffer, strlen(buffer));

    if (length == 0) {
        // File is empty
        return;
    }

    // Read in the file in chunks and send it to the socket
    size_t bytes_read = 0;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        write(socket_fd, buffer, bytes_read);
    }
}
