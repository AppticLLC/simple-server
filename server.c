#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>

int server_fd;

void handle_client(int fd) {
    // Get our response ready.
    char *data = "Hello world.\n";
    int size = strlen(data);

    // Start the response.
    dprintf(fd, "HTTP/1.0 200 OK\r\n");

    // Set headers.
    // Double line break to indicate the end of headers.
    dprintf(fd, "Content-Type: text/html\r\n");
    dprintf(fd, "Content-Length: %d\r\n\r\n", size);

    // Loop until we're finished writing.
    ssize_t bytes_sent;
    while (size > 0) {
        bytes_sent = write(fd, data, size);
        if (bytes_sent < 0)
            return;
        size -= bytes_sent;
        data += bytes_sent;
    }
}

void start_server(int *server_socket) {
    // Specify the port.
    int port = 8000;

    // Set up the socket for the server.
    *server_socket = socket(PF_INET, SOCK_STREAM, 0);

    // Allow reuse of local addresses.
    int socket_option = 1;
    setsockopt(*server_socket, SOL_SOCKET, SO_REUSEADDR, &socket_option, 
        sizeof(socket_option));

    // Set up the server address struct.
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // Assign the address to the socket.
    bind(*server_socket, (struct sockaddr *) &server_address, 
        sizeof(server_address));

    // Start listening.
    listen(*server_socket, 1024);

    // Initialize the relevant variables to handle client sockets.
    struct sockaddr_in client_address;
    size_t client_address_length = sizeof(client_address);
    int client_socket;

    // Start the server loop.
    while (1) {
        // Accept the client socket.
        client_socket = accept(*server_socket,
            (struct sockaddr *) &client_address,
            (socklen_t *) &client_address_length);

        handle_client(client_socket);

        close(client_socket);
    }

    // Shut down the server.
    shutdown(*server_socket, SHUT_RDWR);
    close(*server_socket);
}

void shut_down_server_handler(int signal) {
    close(server_fd);
    exit(0);
}

int main(int argc, char **argv) {
    // Make sure we shut down the server gracefully.
    signal(SIGINT, shut_down_server_handler);

    start_server(&server_fd);
}