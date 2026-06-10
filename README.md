# Basic Chat Server in C

A simple client-server chat application built in C using TCP socket programming.

This project was created to understand how networking works at a low level, including socket creation, binding, listening, accepting connections, connecting clients, and exchanging messages between a server and a client.

## Features

* TCP-based client-server communication
* Server listens for incoming client connections
* Client connects to the server using IP address and port
* Message exchange between client and server
* Basic terminal-based chat interface
* Built using C socket programming

## Technologies Used

* C
* Linux / WSL
* TCP Sockets
* GCC
* Terminal

## Project Structure

```text
BasicChatServer/
├── server.c
├── client.c
└── README.md
```

## How It Works

The project contains two programs:

### Server

The server:

1. Creates a socket
2. Binds it to a port
3. Listens for incoming connections
4. Accepts a client connection
5. Sends and receives messages

### Client

The client:

1. Creates a socket
2. Connects to the server IP and port
3. Sends messages to the server
4. Receives responses from the server

## Compilation

Compile the server:

```bash
gcc server.c -o server
```

Compile the client:

```bash
gcc client.c -o client
```

## Usage

Start the server first:

```bash
./server
```

Then open another terminal and start the client:

```bash
./client
```

## Example

```text
Server started...
Waiting for client...
Client connected!

Client: Hello server
Server: Hello client
```

## Learning Objectives

This project helped me practice:

* Socket programming in C
* TCP client-server architecture
* Linux terminal usage
* `bind()`, `listen()`, `accept()`, and `connect()`
* Sending and receiving data using sockets
* Understanding how network communication works under the hood

## Future Improvements

* Support multiple clients
* Add usernames
* Add message timestamps
* Improve error handling
* Add chat history logging
* Add threading for simultaneous send/receive
* Add command support like `/exit`

## Author

Angan Ghosh

Computer Science Student exploring C, Linux, Networking, Cybersecurity, System Programming, and AI fundamentals.
