#include <stdio.h>
#include <winsock2.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"


#define BUFFER_SIZE 1024


int read_config_file(const char *filename, int *port, char *p1, char *p2) {
    FILE *fp;
    char line[1000];
    char *key, *value;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Failed to open file: %s\n", filename);
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        key = strtok(line, " =\n");
        value = strtok(NULL, " =\n");

        if (key != NULL && value != NULL) {
            if (strcmp(key, "port") == 0) {
                *port = strtol(value, NULL, 10);
            } else if (strcmp(key, "p1") == 0) {
                strcpy(p1, value);
            } else if (strcmp(key, "p2") == 0) {
                strcpy(p2, value);
            }
        }
    }

    printf("Port: %d\n", port);
    printf("p1: %s\n", p1);
    printf("p2: %s\n", p2);

    fclose(fp);
    return 0;
}

int add_to_db(char* text, int type){
    //Connect to db
    sqlite3* db;
    char* err_msg = 0;

    int rc = sqlite3_open("mydb", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }else
        printf("%s", "the database is opened:\n");

    char *sql = "INSERT INTO Messages (String, Type) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    } else {
        rc = sqlite3_bind_text(stmt, 2, text, -1, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to bind v1: %s\n", sqlite3_errmsg(db));
        }
        rc = sqlite3_bind_int(stmt, 1, type);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to bind v2: %s\n", sqlite3_errmsg(db));
        }
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
    }
}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    printf("%s\n", argv[1] ? argv[1] : "NULL");
    return 0;
}

void routine(){
    sqlite3* db;
    char* errmsg = 0;
    int rc = sqlite3_open("mydb", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }else
        printf("%s", "the database is opened:\n");

    char *sql = "SELECT * FROM Messages WHERE Type = 1 ORDER BY String ASC;";
    rc = sqlite3_exec(db, sql, callback, 0, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
    }

    sqlite3_close(db);
}

void append_to_log(char* message) {
    //The messages are added in Database too and this function is optionally added.
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
    int PORT;
    char p1[100];
    char p2[100];
    read_config_file("config.ini", &PORT, p1, p2);

    WSADATA wsa;
    SOCKET s, new_socket;
    struct sockaddr_in server, client;
    int c, recv_len;
    char buffer[BUFFER_SIZE];

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

    time_t last_time = time(NULL);
    time_t current_time;
    int interval = 120;

    // Receive data from client
    while ((recv_len = recv(new_socket, buffer, BUFFER_SIZE, 0)) != SOCKET_ERROR) {
        buffer[recv_len] = '\0';
        time(&current_time);
        time_info = localtime(&current_time);

        if (strstr(buffer, p1) != NULL ) {//If the prefix was P1
            //Create the message (int)
            strftime(time_buffer, 80, "%Y-%m-%d %H:%M:%S", time_info);
            char* str1 =  (char*)time_buffer;
            char* str2 =  (char*)buffer;
            int len1 = (int)strlen(str1);
            int len2 = (int)strlen(str2);
            char* result = (char*)calloc(len1 + len2 + 1, sizeof(char));
            if (result == NULL) {
                printf("Error: Failed to allocate memory.\n");
                return 1;
            }
            int len = (int)strlen(p1);
            str2 = str2 + len;
            strcpy(result, str1);
            strcat(result, " ");
            strcat(result, str2);

            //Append the message into `log.txt` file
            append_to_log(result);

            //Append the message in to `mydb` database
            add_to_db(result, 1);// type 1 = int

            free(result);
        }
        else if (strstr(buffer, p2) != NULL) {//If the prefix was P1
            //Create the message (string)
            strftime(time_buffer, 80, "%Y-%m-%d %H:%M:%S", time_info);
            char* str1 =  (char*)time_buffer;
            char* str2 =  (char*)buffer;
            int len1 = (int)strlen(str1);
            int len2 = (int)strlen(str2);

            char* result = (char*)calloc(len1 + len2 + 1, sizeof(char));

            if (result == NULL) {
                printf("Error: Failed to allocate memory.\n");
                return 1;
            }
            int len = strlen(p2);
            str2 = str2 + len;
            strcpy(result, str1);

            strcat(result, " ");
            strcat(result, str2);

            append_to_log(result);
            add_to_db(result, 2);//type 2 = string
            free(result);
        }
        else if (strstr(buffer, "QQQ") != NULL){
            //As an option to close the app I considered string 'QQQ' as a command to close the app
            printf("Exit code has entered(QQQ).\n");
            break;
        }

        if (current_time - last_time >= interval) {
            printf("Sorted value of type P1:\n");
            last_time = current_time;
            routine();
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
