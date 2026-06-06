#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUF_SIZE 1024
#define MAX_CLNT 256

SOCKET clnt_socks[MAX_CLNT];
int clnt_cnt = 0;
CRITICAL_SECTION cs;

void ErrorHandling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

unsigned WINAPI HandleClient(void *arg) {
    SOCKET clnt_sock = *((SOCKET*)arg);
    int str_len = 0, i;
    char msg[BUF_SIZE];

    while ((str_len = recv(clnt_sock, msg, sizeof(msg), 0)) != 0) {
        if (str_len == -1) break; // Error or disconnection

        // Broadcast
        EnterCriticalSection(&cs);
        for (i = 0; i < clnt_cnt; i++) {
            send(clnt_socks[i], msg, str_len, 0);
        }
        LeaveCriticalSection(&cs);
    }

    // Remove disconnected client
    EnterCriticalSection(&cs);
    for (i = 0; i < clnt_cnt; i++) {
        if (clnt_sock == clnt_socks[i]) {
            while (i < clnt_cnt - 1) {
                clnt_socks[i] = clnt_socks[i + 1];
                i++;
            }
            break;
        }
    }
    clnt_cnt--;
    LeaveCriticalSection(&cs);

    closesocket(clnt_sock);
    return 0;
}

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    SOCKADDR_IN servAdr, clntAdr;
    int clntAdrSz;
    HANDLE hThread;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    InitializeCriticalSection(&cs);

    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&servAdr, 0, sizeof(servAdr));
    servAdr.sin_family = AF_INET;
    servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAdr.sin_port = htons(PORT);

    if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
        ErrorHandling("bind() error");

    if (listen(hServSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen() error");

    printf("Server started on port %d. Waiting for clients...\n", PORT);

    while (1) {
        clntAdrSz = sizeof(clntAdr);
        hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSz);

        EnterCriticalSection(&cs);
        clnt_socks[clnt_cnt++] = hClntSock;
        LeaveCriticalSection(&cs);

        hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)&hClntSock, 0, NULL);
        printf("Connected client IP: %s\n", inet_ntoa(clntAdr.sin_addr));
    }

    closesocket(hServSock);
    WSACleanup();
    DeleteCriticalSection(&cs);
    return 0;
}
