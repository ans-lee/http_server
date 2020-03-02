/*
 *  Library Includes
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

/*
 *  Non-library Includes
 */

#include "http_responses.h"

/*
 *  Helper Function Prototypes
 */

static int check_permission(char *filepath);

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
    if (strcmp(file, "/") == 0)
        file = "index.html";
    else
        file += 1;

    FILE *fp = fopen(file, "r");
    char *response = malloc(BUFSIZ);

    if (!fp || (check_permission(file) == -1)) {
        // Return 404 status code if file not found or check_permission fails
        snprintf(response, BUFSIZ, "HTTP/1.1 404 Not Found\n"
                                   "Content-Type: text/html\n"
                                   "Content-Length: 13\n\n"
                                   "404 Not Found\n");
        return response;
    } else if (check_permission(file) == 0) {
        // Return 403 Forbidden if user has no permissions
        snprintf(response, BUFSIZ, "HTTP/1.1 403 Forbidden\n"
                                   "Content-Type: text/html\n"
                                   "Content-Length: 13\n\n"
                                   "403 Forbidden\n");
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
