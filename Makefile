http_server: http_server.c http_responses.h http_responses.c
	gcc -pthread -o http_server http_server.c http_responses.h http_responses.c
