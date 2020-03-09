tcp_socket_server: tcp_socket_server.c http_responses.h http_responses.c
	gcc -pthread -o tcp_socket_server tcp_socket_server.c http_responses.h http_responses.c
