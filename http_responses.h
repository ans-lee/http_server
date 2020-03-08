//
//  Contains functions that handle HTTP responses
//

/*
 *  Functions
 */

// Sends an appropriate response based on the request headers
void send_response(int socket_fd, char *request_headers);

// Sends a 404 Not Found Response
void send_404_response(int socket_fd);

// Sends a 403 Forbidden Response
void send_403_response(int socket_fd);

// Sends a 400 Bad Request Response
void send_400_response(int socket_fd);

// Generates and sends a response to a GET request, with
// status code 200 OK and the requested file/folder/etc if it exists
// and the user has permission, otherwise returns a 404 Not Found
// or 403 Forbidden depending on the condition of the requested data
void handle_get_response(int socket_fd, char *file);
