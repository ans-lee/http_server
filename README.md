# HTTP Server

A basic TCP HTTP server in C using sockets to communicate between clients, interpreting
requests and responding in an appropriate manner. This is just a small project used to
understand socket programming, HTTP status codes, request and response headers and
generally gaining some knowledge on how HTTP servers work.

## Features

* Runs a server on localhost with a port number of your choosing or a random one
* Parses GET requests in a general manner (the code is easily portable)
* Can intercept POST requests but currently does not do anything (POST requests varies between applications, some starting code is provided!)
* Sends appropriate status codes depending on the requests
* Concurrency

## Usage

1. Add some files (html files would be great) to the directory containing the repository

2. While on the terminal and in the directory with the repository, type this command:
```
make
```
3. Then, if you wish to run the server with a random port number, type:
```
./http_server
```
4. Otherwise if you want to specify a port number:
```
./http_server 8000
```
5. You should be able to access the server on a browser at http://localhost:XXXXX or http://127.0.0.1:XXXXX, replacing the X's with the port number

**NOTE:** Some port numbers around 1 to 1000 are reserved and numbers below 0 and above 66535 cannot be used.
A port number higher than 2000 should be fine.

## Issues

As this is a small project for educational purposes and not for commercial use, the server will have
several issues if used beyond its capabilities. Thus, I would not recommend anyone to use this
server outside of a developer environment.

## Support

Currently, I have only tested the server on Ubuntu 18.04.4, a Linux distribution and it should work
for any Linux operating system. It may work for Mac users, however Windows is most likely
incompatible. I don't plan making a version that works with Windows.
