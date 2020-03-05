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

#define MAX_BUFF_LEN    1024    // 1KB buffer size for reading files

/*
 *  Helper Function Prototypes
 */

static int check_permission(char *filepath);
static void send_file_contents(int socket_fd, FILE *fp);

void send_response(int socket_fd, char *request_headers) {
    char *http_method = strtok(request_headers, " ");

    if (strcmp(http_method, "GET") == 0) {
        // Get the filename without the first slash
        char *file = strtok(NULL, " ");
        send_get_response(socket_fd, file);
    }

    return;
}

// NOTE: Content-Length header response must be very accurate otherwise
// nothing will show
void send_get_response(int socket_fd, char *file) {
    if (strcmp(file, "/") == 0)
        file = "index.html";
    else
        // Get the slash out of the filename
        file += 1;

    FILE *fp = fopen(file, "r");
    char *header = malloc(BUFSIZ);

    if (fp == NULL || check_permission(file) == -1) {
        // Return 404 status code if file not found or check_permission fails
        snprintf(header, BUFSIZ, "HTTP/1.1 404 Not Found\n"
                                   "Content-Type: text/html\n"
                                   "Content-Length: 13\n\n"
                                   "404 Not Found\n");
        write(socket_fd, header, strlen(header));
        return;
    } else if (check_permission(file) == 0) {
        // Return 403 Forbidden if user has no permissions
        snprintf(header, BUFSIZ, "HTTP/1.1 403 Forbidden\n"
                                   "Content-Type: text/html\n"
                                   "Content-Length: 13\n\n"
                                   "403 Forbidden\n");
        write(socket_fd, header, strlen(header));
        return;
    }

    /*
    long length = 0;
    char *contents = read_file_contents(fp, &length);
    if (contents == NULL) {
        // Return 500 Internal Server Error if the file could not be obtained
        snprintf(header, BUFSIZ, "HTTP/1.1 500 Internal Server Error\n"
                                   "Content-Type: text/html\n"
                                   "Content-Length: 25\n\n"
                                   "500 Internal Server Error\n");
        return header;
    }
    fclose(fp);
    */

    if (strstr(file, ".html") != NULL) {
        snprintf(header, BUFSIZ, "HTTP/1.1 200 OK\n"
                                 "Content-Type: text/html\n");
        write(socket_fd, header, strlen(header));
    } else if (strstr(file, ".jpg") != NULL || strstr(file, ".jpeg") != NULL) {
        snprintf(header, BUFSIZ, "HTTP/1.1 200 OK\n"
                                 "Content-Type: image/jpeg\n");
        write(socket_fd, header, strlen(header));
    }
    // TODO: else statement

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
