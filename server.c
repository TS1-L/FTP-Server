#include "commands.h"

User users[MAX_USERS] = {
    {"user1", "pass1"},
    {"user2", "pass2"}
};

FileLog file_logs[MAX_FILES];
int file_count = 0;

void log_upload(const char* filename, const char* username) {
    if (file_count >= MAX_FILES) return;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    
    FileLog log;
    strncpy(log.filename, filename, sizeof(log.filename));
    strncpy(log.uploaded_by, username, sizeof(log.uploaded_by));
    strftime(log.upload_date, sizeof(log.upload_date), "%Y-%m-%d %H:%M:%S", t);

    file_logs[file_count++] = log;
    printf("Logged upload: %s by %s on %s\n", filename, username, log.upload_date);
}

int authenticate_user(const char* username, const char* password) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return 1;
        }
    }
    return 0;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    char username[50];

    // authentication for user
    recv(client_socket, buffer, sizeof(buffer), 0);
    strncpy(username, buffer, sizeof(username));
    
    recv(client_socket, buffer, sizeof(buffer), 0);
    if (!authenticate_user(username, buffer)) {
        send(client_socket, "Authentication failed\n", 22, 0);
        close(client_socket);
        return;
    }
    send(client_socket, "Authentication successful\n", 26, 0);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        recv(client_socket, buffer, sizeof(buffer), 0);

        if (strncmp(buffer, "LIST", 4) == 0) {
            list_files(client_socket);
        } else if (strncmp(buffer, "STOR", 4) == 0) {
            char* filename = buffer + 5;
            upload_file(client_socket, filename, username);
        } else if (strncmp(buffer, "RETR", 4) == 0) {
            char* filename = buffer + 5;
            download_file(client_socket, filename);
        } else if (strncmp(buffer, "QUIT", 4) == 0) {
            break;
        }
    }

    close(client_socket);
}

void list_files(int client_socket) {
    char response[BUFFER_SIZE] = "";
    for (int i = 0; i < file_count; i++) {
        char line[BUFFER_SIZE];
        snprintf(line, sizeof(line), "%s uploaded by %s on %s\n", file_logs[i].filename, file_logs[i].uploaded_by, file_logs[i].upload_date);
        strncat(response, line, sizeof(response) - strlen(response) - 1);
    }
    send(client_socket, response, strlen(response), 0);
}

void upload_file(int client_socket, const char* filename, const char* username) {
    FILE* file = fopen(filename, "wb");
    char buffer[BUFFER_SIZE];
    int bytes_read;

    if (!file) {
        send(client_socket, "Failed to open file\n", 21, 0);
        return;
    }

    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_read, file);
        if (bytes_read < BUFFER_SIZE) break;
    }

    fclose(file);
    log_upload(filename, username);
    send(client_socket, "File uploaded successfully\n", 28, 0);
}

void download_file(int client_socket, const char* filename) {
    FILE* file = fopen(filename, "rb");
    char buffer[BUFFER_SIZE];
    int bytes_read;

    if (!file) {
        send(client_socket, "File not found\n", 15, 0);
        return;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    fclose(file);
    send(client_socket, "File download complete\n", 25, 0);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(1);
    }

    listen(server_socket, 5);
    printf("Server is listening on port %d\n", PORT);

    while ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len))) {
        pthread_t thread;
        pthread_create(&thread, NULL, (void*)handle_client, (void*)(intptr_t)client_socket);
    }

    close(server_socket);
    return 0;
}
