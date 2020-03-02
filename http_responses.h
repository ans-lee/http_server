//
//  Contains functions that handle HTTP responses
//

/*
 *  Functions
 */

// Creates an appropriate response based on the request headers
char *create_response(char *request_headers);

// Generates a GET response, with status code 200 OK and the requested file/folder/etc if
// it exists and the user has permission, otherwise returns a 404 Not Found or 401 Unauthorized
// depending on the condition of the requested data
char *create_get_response(char *file);
