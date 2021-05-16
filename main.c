#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <string.h>
#include "handling.h"


int main(int argc, char *argv[]) {
    getArgs(argc, argv);

    list = loadFromFile(fileName);
    listTmp = loadFromFile("listTmp.txt");

    if (debug) {
        printf("configuration file name: %s\n", fileName);
        printf("configuration dnsServer IP: %s\n", dnsServerIP);
        printf("AA IP:\n");
        printAll(list);
        printf("TMP IP:\n");
        printAll(listTmp);
    }

    WSADATA wsaData;
    WSAStartup(WINSOCK_VERSION, &wsaData);

    for (int i = 0; i < 10; ++i) {
        answer[i] = calloc(1, 512);
    }

    char recvBuffer[512];
    int result;

    int flag = 1;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    dnsAddr.sin_family = AF_INET;
    dnsAddr.sin_port = htons(port);
    dnsAddr.sin_addr.s_addr = inet_addr(dnsServerIP);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    result = bind(sock, (SOCKADDR *) &serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        printf("Socket Error!\n");
        closesocket(sock);
        exit(-1);
    }

    hMutexSock = CreateMutex(NULL, FALSE, NULL);
    hMutexFlag = CreateMutex(NULL, FALSE, NULL);
    hMutexThread = CreateMutex(NULL, FALSE, NULL);
    hMutexMsg = CreateMutex(NULL, FALSE, NULL);
    hMutexSign = CreateMutex(NULL, FALSE, NULL);
    hMutexList = CreateMutex(NULL, FALSE, NULL);
    for (int i = 0; i < 10; ++i) {
        hMutexSigns[i] = CreateMutex(NULL, TRUE, NULL);
    }
    HANDLE hThread;
    unsigned threadID;
    hThread = (HANDLE) _beginthreadex(NULL, 0, &keyListener, &flag, 0, &threadID);

    int timeout = 500;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));

    printf("Server is listening port %d......\n", port);

    WaitForSingleObject(hMutexFlag, INFINITE);
    while (flag) {
        ReleaseMutex(hMutexFlag);
        SOCKADDR_IN clientAddr;
        int clientLen = sizeof(clientAddr);
        WaitForSingleObject(hMutexSock, INFINITE);
        result = recvfrom(sock, recvBuffer, 512, 0, (SOCKADDR *) &clientAddr, &clientLen);
        ReleaseMutex(hMutexSock);
        if (result != -1 && result != 10060) {
            int msgType = classifyMsg(recvBuffer, result);
            if (msgType == QUE) {
                Client *client = calloc(1, sizeof(Client));
                client->clientAddr = clientAddr;
                memcpy(client->recvBuffer, recvBuffer, result);
                client->n = result;
                WaitForSingleObject(hMutexThread, INFINITE);
                int n = findEmpty(threadUsed);
                if (n == -1) {
                    printf("No thread for this query!\n");
                } else {
                    client->threadNum = n;
                    threadUsed[n] = 1;
                    cThread[n] = (HANDLE) _beginthreadex(NULL, 0, handlingClient, client, 0, cThreadID);
                }
                ReleaseMutex(hMutexThread);
            } else if (msgType == ANS) {
                short ID = getID(recvBuffer);
                WaitForSingleObject(hMutexMsg, INFINITE);
                int n = findAns(ID);
                if (n == -1) {
                    printf("ID not found!\n");
                } else {
                    memcpy(answer[n], recvBuffer, result);
                    ansLen[n] = result;
                    ReleaseMutex(hMutexSigns[n]);
                }
                ReleaseMutex(hMutexMsg);
            }
        }

        WaitForSingleObject(hMutexThread, INFINITE);
        closeThread();
        ReleaseMutex(hMutexThread);

        WaitForSingleObject(hMutexSign, INFINITE);
        releaseSign();
        ReleaseMutex(hMutexSign);
        WaitForSingleObject(hMutexFlag, INFINITE);
    }

    ReleaseMutex(hMutexFlag);
    CloseHandle(hThread);

    WaitForMultipleObjects(10, cThread, TRUE, INFINITE);

    closeThread();
    closesocket(sock);
    CloseHandle(hMutexThread);
    CloseHandle(hMutexSock);

    WSACleanup();

    for (int i = 0; i < 10; ++i) {
        free(answer[i]);
    }

    if (debug) {
        printf("TMP list now is:\n");
        printAll(listTmp);
    }

    writeToFile("listTmp.txt", listTmp);
    freeAll(list);
    freeAll(listTmp);

    printf("Server is closed.");

    return 0;
}





