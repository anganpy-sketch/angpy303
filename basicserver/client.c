#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{

    int client_fd;
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        perror("socket");
        return 1;
    }
    printf("Client socket created!\n");

    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET,
              "127.0.0.1",
              &server_addr.sin_addr);

    if (connect(client_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        return 1;
    }

    printf("Connected to server!\n");

    while (1)
    {
        char msg[1024];
        printf("you : ");
        fgets(msg, sizeof(msg), stdin);
        msg[strcspn(msg, "\n")] = '\0';
        if ((strcmp(msg, "exit")) == 0)
        {
            break;
        }
        int bytes_send = send(client_fd, msg, strlen(msg), 0);
      if (bytes_send < 0)
    {
        perror("send");
        break;
    }

        printf("bytes send is %d.\n", bytes_send);
    }
    close(client_fd);
}