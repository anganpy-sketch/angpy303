#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{

    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }
    printf("server_fd= %d ; Socket created successfully!\n", server_fd);

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    printf("The memory adress of server_addr is %p \n ", &server_addr);

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        return 1;
    }
    printf("Waiting for connection...\n");
    int client_fd;
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
    {
        perror("accept");
        return 1;
    }
    printf("Connected sucessfully !! \n");

    char buffer[1024];

    while (1)
    {
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received < 0)
        {
            perror("recv");
            break;
        }

        if (bytes_received == 0)
        {
            printf("Client disconnected\n");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Client: %s\n", buffer);

        char serverreply[1024];
        printf("you : ");
        fgets(serverreply, sizeof(serverreply), stdin);
        serverreply[strcspn(serverreply, "\n")] = '\0';
        if ((strcmp(serverreply, "exit")) == 0)
        {
            break;
        }
        int bytes_send = send(client_fd, serverreply, strlen(serverreply), 0);
        if (bytes_send < 0)
        {
            perror("send");
            break;
        }

        printf("bytes send is %d.\n", bytes_send);
    }

    close(client_fd);
    close(server_fd);

    return 0;
}