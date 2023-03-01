
#include <stdio.h>
#include <winsock2.h>
#include <time.h>

#define BUFFER_SIZE 1024

void append_to_log(char* message) {
    FILE *fp;
    fp = fopen("log.txt", "a");
    if (fp == NULL) {
        printf("Failed to open file for writing.\n");
        return;
    }

    fprintf(fp, "%s\n", message); // write message to file
    fclose(fp); // close file
}

int main() {
    WSADATA wsa;
    int PORT = 9000;
    SOCKET s, new_socket;
    struct sockaddr_in server, client;
    int c, recv_len;
    char buffer[BUFFER_SIZE];
    time_t current_time;
    struct tm* time_info;
    char time_buffer[80];

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Winsock initialized.\n");

    // Create socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Socket created.\n");

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed. Error code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Bind done.\n");

    // Listen to incoming connections
    listen(s, 3);

    printf("Waiting for incoming connections...\n");

    c = sizeof(struct sockaddr_in);

    // Accept an incoming connection
    if ((new_socket = accept(s, (struct sockaddr *)&client, &c)) == INVALID_SOCKET) {
        printf("Accept failed. Error code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Connection accepted.\n");

    // Receive data from client
    while ((recv_len = recv(new_socket, buffer, BUFFER_SIZE, 0)) != SOCKET_ERROR) {
        buffer[recv_len] = '\0';

        if (strstr(buffer, "PRE") != NULL) {

            time(&current_time);
            time_info = localtime(&current_time);
            strftime(time_buffer, 80, "%Y-%m-%d %H:%M:%S", time_info);


//            printf("[%s] %s\n", time_buffer, buffer + 3);

            char* str1 =  (char*)time_buffer;
            char* str2 =  (char*)buffer;
            int len1 = (int)strlen(str1);
            int len2 = (int)strlen(str2);

            char* result = (char*)calloc(len1 + len2 + 1, sizeof(char));

            if (result == NULL) {
                printf("Error: Failed to allocate memory.\n");
                return 1;
            }

            str2 = str2 + 3;
            strcpy(result, str1);

            strcat(result, " ");
            strcat(result, str2);

//            printf("%s\n", result);

            append_to_log(result);

            free(result);
        }
        else if (strstr(buffer, "QQQ") != NULL){
            printf("Exit code has entered(QQQ).\n");
            break;
        }
    }

    if (recv_len == SOCKET_ERROR) {
        printf("Recv failed. Error code: %d\n", WSAGetLastError());
        return 1;
    }

    closesocket(new_socket);
    WSACleanup();

    return 0;
}
