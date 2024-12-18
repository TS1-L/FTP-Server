#include "commands.h"

void interact_with_server(int server_socket) {
    char buffer[BUFFER_SIZE];

    printf("Enter username: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    send(server_socket, buffer, strlen(buffer), 0);

    printf("Enter password: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    send(server_socket, buffer, strlen(buffer), 0);

    recv(server_socket, buffer, sizeof(buffer), 0);
    printf("%s", buffer);
    if (strncmp(buffer, "Authentication failed", 20) == 0) {
        return;
    }

    while (1) {
        printf("ftp> ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        send(server_socket, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "LIST", 4) == 0) {
            memset(buffer, 0, sizeof(buffer));
            recv(server_socket, buffer, sizeof(buffer), 0);
            printf("%s", buffer);
        } else if (strncmp(buffer, "STOR", 4) == 0) {
            char* filename = buffer + 5;
            FILE* file = fopen(filename, "rb");
            if (!file) {
                printf("File not found\n");
                continue;
            }

            while (!feof(file)) {
                int bytes_read = fread(buffer, 1, sizeof(buffer), file);
                send(server_socket, buffer, bytes_read, 0);
            }

            fclose(file);
            memset(buffer, 0, sizeof(buffer));
            recv(server_socket, buffer, sizeof(buffer), 0);
            printf("%s", buffer);
        } else if (strncmp(buffer, "RETR", 4) == 0) {
            char* filename = buffer + 5;
            FILE* file = fopen(filename, "wb");
            if (!file) {
                printf("Failed to create file\n");
                continue;
            }

            int bytes_received;
            while ((bytes_received = recv(server_socket, buffer, sizeof(buffer), 0)) > 0) {
                fwrite(buffer, 1, bytes_received, file);
                if (bytes_received < BUFFER_SIZE) break;
            }

            fclose(file);
            printf("File downloaded\n");
        } else if (strncmp(buffer, "QUIT", 4) == 0) {
            break;
        }
    }
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(client_socket);
        exit(1);
    }

    interact_with_server(client_socket);

    close(client_socket);
    return 0;
}
