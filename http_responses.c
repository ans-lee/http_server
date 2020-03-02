#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http_responses.h"

char* create_response(char *request_headers) {
    char *http_method = strtok(request_headers, " ");

    if (strcmp(http_method, "GET") == 0) {
        // Get the filename without the first slash
        char *file = strtok(NULL, " ");
        return create_get_response(file);
    }

    return NULL;
}

// NOTE: Content-Length header response must be very accurate otherwise
// nothing will show
// TODO: check for permissions and handle folders and other file types like images
char *create_get_response(char *file) {
    if (strcmp(file, "/") == 0) {
        file = "index.html";
    } else {
        file += 1;
    }

    FILE *fp = fopen(file, "r");
    char *response = malloc(BUFSIZ);

    // Return 404 status code if file not found
    if (!fp) {
        snprintf(response, BUFSIZ, "HTTP/1.1 404 Not Found\n"
                                   "Content-Type: text/html\n"
                                   "Content-Length: 13\n\n"
                                   "404 Not Found\n");
        return response;
    }

    // Return the contents of the file
    char buffer[BUFSIZ];
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fread(buffer, 1, length, fp);
    fclose(fp);
    printf("%s", buffer);

    snprintf(response, BUFSIZ, "HTTP/1.1 200 OK\n"
                               "Content-Type: text/html\n"
                               "Content-Length: %ld\n\n"
                               "%s", strlen(buffer), buffer);

    return response;
}
