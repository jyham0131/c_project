#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define BUF_SIZE 1024
#define NAME_SIZE 20
#define PORT 8080

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

void ErrorHandling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

unsigned WINAPI SendMsg(void *arg) {
    SOCKET hSock = *((SOCKET*)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];

    while (1) {
        fgets(msg, BUF_SIZE, stdin);
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
            closesocket(hSock);
            exit(0);
        }
        sprintf(name_msg, "%s %s", name, msg);
        send(hSock, name_msg, strlen(name_msg), 0);
    }
    return 0;
}

unsigned WINAPI RecvMsg(void *arg) {
    int hSock = *((SOCKET*)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    int str_len;

    while (1) {
        str_len = recv(hSock, name_msg, NAME_SIZE + BUF_SIZE - 1, 0);
        if (str_len == -1) return -1;
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    SOCKET hSock;
    SOCKADDR_IN servAdr;
    HANDLE hSndThread, hRcvThread;

    // Default server IP is local host for testing
    char* server_ip = "127.0.0.1";

    printf("Enter your name: ");
    fgets(name, NAME_SIZE, stdin);
    // Remove newline character
    name[strcspn(name, "\n")] = 0;

    char formatted_name[NAME_SIZE+4];
    sprintf(formatted_name, "[%s]", name);
    strcpy(name, formatted_name);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    hSock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&servAdr, 0, sizeof(servAdr));
    servAdr.sin_family = AF_INET;
    servAdr.sin_addr.s_addr = inet_addr(server_ip);
    servAdr.sin_port = htons(PORT);

    if (connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
        ErrorHandling("connect() error");

    printf("Connected to chat server.\n");

    hSndThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);
    hRcvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSock, 0, NULL);

    WaitForSingleObject(hSndThread, INFINITE);
    WaitForSingleObject(hRcvThread, INFINITE);

    closesocket(hSock);
    WSACleanup();
    return 0;
}
