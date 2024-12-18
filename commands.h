#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#define MAX_USERS 5
#define MAX_FILES 10
#define BUFFER_SIZE 1024
#define PORT 21

typedef struct {
    char username[50];
    char password[50];
} User;

typedef struct {
    char filename[100];
    char uploaded_by[50];
    char upload_date[20];
} FileLog;

extern User users[MAX_USERS];
extern FileLog file_logs[MAX_FILES];
extern int file_count;

int authenticate_user(const char* username, const char* password);
void log_upload(const char* filename, const char* username);
void list_files(int client_socket);
void download_file(int client_socket, const char* filename);
void upload_file(int client_socket, const char* filename, const char* username);
void handle_client(int client_socket);

#endif
