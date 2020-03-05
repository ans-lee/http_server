//
//  Contains functions that handle HTTP responses
//

/*
 *  Functions
 */

// Sends an appropriate response based on the request headers
void send_response(int socket_fd, char *request_headers);

// Generates and sends a GET response, with status code 200 OK and the
// requested file/folder/etc if it exists and the user has permission,
// otherwise returns a 404 Not Found or 401 Unauthorized
// depending on the condition of the requested data
void send_get_response(int socket_fd, char *file);
