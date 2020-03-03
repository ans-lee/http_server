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
 *  #defines
 */

#define MAX_BUFF_LEN    1024    // 1KB buffer size for reading files

/*
 *  Helper Function Prototypes
 */

static int check_permission(char *filepath);
static char *read_file_contents(FILE *fp, long *length);

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

    long length = 0;
    char *contents = read_file_contents(fp, &length);
    if (contents == NULL) {
        // Return 500 Internal Server Error if the file could not be obtained
        snprintf(response, BUFSIZ, "HTTP/1.1 500 Internal Server Error\n"
                                   "Content-Type: text/html\n"
                                   "Content-Length: 25\n\n"
                                   "500 Internal Server Error\n");
        return response;
    }
    fclose(fp);

    if (strstr(file, ".html") != NULL) {
        snprintf(response, BUFSIZ, "HTTP/1.1 200 OK\n"
                                   "Content-Type: text/html\n"
                                   "Content-Length: %ld\n\n"
                                   "%s", length, contents);
    } else if (strstr(file, ".jpg") != NULL || strstr(file, ".jpeg") != NULL) {
        snprintf(response, BUFSIZ, "HTTP/1.1 200 OK\n"
                                   "Content-Type: image/jpeg\n"
                                   "Content-Length: %ld\n\n"
                                   "%s", length, contents);
    }
    // TODO: else statement

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

// Reads and returns the contents of the file, returns NULL if failed or if file is empty
// TODO: for images there are null terminating chars, need to find another way
static char *read_file_contents(FILE *fp, long *length) {
    char buffer[MAX_BUFF_LEN];
    char *contents = NULL;

    // Get the length of the file
    fseek(fp, 0, SEEK_END);
    *length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (*length == 0) {
        return NULL;
    }

    // Allocate exact size if lower than 1 KB, otherwise allocate a starting
    // size of 1 KB
    if (*length < MAX_BUFF_LEN) 
        contents = malloc(*length);
    else
        contents = malloc(MAX_BUFF_LEN);

    // If malloc failed, stop this function
    if (contents == NULL)
        return NULL;

    // Read in the file into contents and get the exact size
    size_t bytes_read = 0;
    size_t total = 0;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (total < (total + bytes_read)) {
            char *new = realloc(contents, total + bytes_read);

            // If realloc fails, free contents and stop this function
            if (new == NULL) {
                free(contents);
                return NULL;
            }
            contents = new;
        }
        strncat(contents, buffer, bytes_read);
        total += bytes_read;
    }
    printf("contents = %s", contents);
    printf("strlen = %ld", strlen(contents));

    return contents;
}
