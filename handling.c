#include <stdio.h>
#include <process.h>
#include "handling.h"

HANDLE hMutexSock = NULL;
HANDLE hMutexFlag = NULL;
HANDLE hMutexThread = NULL;
HANDLE hMutexMsg = NULL;
HANDLE hMutexSigns[10] = {NULL};
HANDLE hMutexSign = NULL;
HANDLE hMutexList = NULL;


HANDLE cThread[10] = {0};
unsigned cThreadID[10] = {0};
int threadUsed[10] = {0};
int needClose[10] = {0};
char *answer[10];
int ansLen[10] = {0};
int ansID[10] = {0};
int releaseM[10] = {0};

SOCKET sock = 0;
SOCKADDR_IN serverAddr = {0};
SOCKADDR_IN dnsAddr = {0};
const int port = 53;

ArrayList list = NULL;
ArrayList listTmp = NULL;

char *fileName = "list.txt";
char *dnsServerIP = "8.8.8.8";
int debug = 0;

clock_t start_time = 0;

unsigned __stdcall keyListener(void *flag) {
    char buffer[5];
    WaitForSingleObject(hMutexFlag, INFINITE);
    while (*(int *) flag) {
        ReleaseMutex(hMutexFlag);
        fgets(buffer, 5, stdin);
        if (strcmp(buffer, "exit") == 0)
            *(int *) flag = 0;
        WaitForSingleObject(hMutexFlag, INFINITE);
    }
    ReleaseMutex(hMutexFlag);
    _endthreadex(0);
}

unsigned handlingClient(void *pArguments) {
    Client *client = (Client *) pArguments;

    char *recvBuff = client->recvBuffer;
    unsigned short originID = getID(recvBuff);
    unsigned short newID = (unsigned short) client->threadNum;
    setID(newID, recvBuff);

    printf("%lf Thread[%d] gets Query. ID: %hd\n", get_time(), client->threadNum, originID);

    char name[200];
    int count;
    int AAFlag = 1;
    int flagGetName = getName(name, recvBuff, client->n);
    if (flagGetName == 0) {
        if (debug)
            printf("%lf Thread[%d] gets Query: %s from %s\n", get_time(), client->threadNum, name,
                   inet_ntoa(client->clientAddr.sin_addr));
        WaitForSingleObject(hMutexList, INFINITE);
        NodePtr found = findNode(list, name);
        ReleaseMutex(hMutexList);
        if (found == NULL) {
            AAFlag = 0;
            WaitForSingleObject(hMutexList, INFINITE);
            found = findNode(listTmp, name);
            ReleaseMutex(hMutexList);
        }
        if (found != NULL) {
            setID(originID, recvBuff);
            setResponse(recvBuff);
            if (strcmp(found->ip, "0.0.0.0") == 0) {
                setNotFound(recvBuff);
                count = client->n;
            } else {
                count = setAns(recvBuff, client->n, found->ip);
                if (AAFlag)
                    setAA(recvBuff);
            }
            printf("%lf Thread[%d] sends Answer.\n", get_time(), client->threadNum);
            sendto(sock, recvBuff, count, 0, (SOCKADDR *) &client->clientAddr, sizeof(client->clientAddr));
        } else {
            WaitForSingleObject(hMutexMsg, INFINITE);
            ansID[client->threadNum] = newID;
            ReleaseMutex(hMutexMsg);

            WaitForSingleObject(hMutexSock, INFINITE);
            sendto(sock, client->recvBuffer, client->n, 0, (SOCKADDR *) &dnsAddr, sizeof(dnsAddr));
            ReleaseMutex(hMutexSock);

            DWORD result = WaitForSingleObject(hMutexSigns[client->threadNum], 2000);
            if (result == WAIT_OBJECT_0) {
                WaitForSingleObject(hMutexSock, INFINITE);
                WaitForSingleObject(hMutexMsg, INFINITE);
                setID(originID, answer[client->threadNum]);
                sendto(sock, answer[client->threadNum], ansLen[client->threadNum], 0, (SOCKADDR *) &client->clientAddr,
                       sizeof(client->clientAddr));
                char IP[200];
                if (isSavable(answer[client->threadNum], client->n)) {
                    getIP(answer[client->threadNum], client->n, IP);
                    WaitForSingleObject(hMutexList, INFINITE);
                    addNodeToList(listTmp, name, IP);
                    if (debug)
                        printf("%lf Thread[%d] gets tmp ip: %s %s\n", get_time(), client->threadNum, name, IP);
                }
                ReleaseMutex(hMutexList);
                ReleaseMutex(hMutexMsg);
                printf("%lf Thread[%d] sends Answer.\n", get_time(), client->threadNum);
                ReleaseMutex(hMutexSock);
                releaseM[client->threadNum] = 1;
                ReleaseMutex(hMutexSigns[client->threadNum]);
            } else {
                printf("%lf Thread[%d] timeout!!!\n", get_time(), client->threadNum);
            }
        }
    } else {
        printf("%lf Thread[%d] ERROR!!!\n", get_time(), client->threadNum);
    }

    WaitForSingleObject(hMutexThread, INFINITE);
    needClose[client->threadNum] = 1;
    ReleaseMutex(hMutexThread);

    free(client);
    _endthreadex(0);
}

int findEmpty(const int a[]) {
    for (int i = 0; i < 10; ++i) {
        if (a[i] == 0)
            return i;
    }
    return -1;
}

void closeThread() {
    for (int i = 0; i < 10; ++i) {
        if (needClose[i] == 1) {
            CloseHandle(cThread[i]);
            printf("%lf Thread[%d] is closed.\n", get_time(), i);
            needClose[i] = 0;
            threadUsed[i] = 0;
        }
    }
}

void releaseSign() {
    for (int i = 0; i < 10; ++i) {
        if (releaseM[i] == 1) {
            WaitForSingleObject(hMutexSigns[i], INFINITE);
            releaseM[i] = 0;
        }
    }
}

int findAns(short ID) {
    for (int i = 0; i < 10; ++i) {
        if (ID == ansID[i])
            return i;
    }
    return -1;
}

int classifyMsg(const char *msg, int n) {
    if (n < 12)
        return ERR;
    int QR = (msg[3] & 0b10000000) >> 7;
    if (QR == 1)
        return ANS;
    else if (QR == 0)
        return QUE;
}

unsigned short getID(char *msg) {
    char buffer[2];
    memcpy(buffer, msg + 1, 1);
    memcpy(buffer + 1, msg, 1);
    return *(unsigned short *) buffer;
}

void setID(unsigned short ID, char *buffer) {
    char *id = (char *) &ID;
    memcpy(buffer, id + 1, 1);
    memcpy(buffer + 1, id, 1);
}

void setNotFound(char *buffer) {
    buffer[3] = (char) (buffer[3] | 0b00000011);
}

void setResponse(char *buffer) {
    buffer[2] = (char) (buffer[2] | 0b10000000);
}

int getName(char *destiny, const char *buffer, int n) {
    if (n <= 12)
        return -1;
    int i = 0;
    int count = 0;
    int total = 0;
    int k = 12;
    int flag = 0;
    while (buffer[k] != 0) {
        if (flag == 0) {
            flag = 1;
            total = (unsigned char) buffer[k];
        } else if (count != total) {
            count++;
            destiny[i] = buffer[k];
            i++;
        } else if (count == total) {
            flag = 0;
            k--;
            destiny[i] = '.';
            i++;
            count = 0;
        }
        k++;
    }
    destiny[i] = 0;
    return 0;
}

int setAns(char *buffer, int n, char *ip) {
    unsigned short *ANCOUNT = (unsigned short *) (buffer + 6);
    *ANCOUNT = htons(1);
    n = setName(buffer, n);
    unsigned long *TTL = (unsigned long *) (buffer + n);
    *TTL = htonl(100);
    n += 4;
    unsigned short *RDLENGTH = (unsigned short *) (buffer + n);
    *RDLENGTH = htons(4);
    n += 2;
    unsigned long *RDATA = (unsigned long *) (buffer + n);
    *RDATA = inet_addr(ip);
    n += 4;
    return n;
}

int setName(char *buffer, int n) {
    memcpy(buffer + n, buffer + 12, n - 12);
    unsigned short *QTYPE = (unsigned short *) (buffer + n * 2 - 12 - 4);
    *QTYPE = htons(1);
    return n * 2 - 12;
}

void setAA(char *buffer) {
    buffer[2] = (char) (buffer[2] | 0b00000100);
}

void getIP(char *buffer, int qLen, char *ip) {
    SOCKADDR_IN address;
    char *type = skipName(buffer + qLen);
    long *RDATA = (long *) (type + 10);
    address.sin_addr.s_addr = *RDATA;
    strncpy(ip, inet_ntoa(address.sin_addr), 200);
}

int isSavable(char *buffer, int qLen) {
    unsigned short *ANCOUNT = (unsigned short *) (buffer + 6);
    unsigned short *NSCOUNT = (unsigned short *) (buffer + 8);
    int ansCount = ntohs(*ANCOUNT);
    int nsCount = ntohs(*NSCOUNT);
    if (ansCount == 0 && nsCount == 0)
        return 0;
    return judgeA(buffer, qLen);
}


int judgeA(char *buffer, int qLen) {
    char *type = skipName(buffer + qLen);
    unsigned short TYPE = ntohs(*(unsigned short *) type);
    return TYPE == 1;
}

char *skipName(char *buffer) {
    while (1) {
        if (judgePointer(buffer)) {
            buffer += 2;
            return buffer;
        }
        if (*buffer == 0)
            return buffer;
        buffer += 1;
    }
}

int judgePointer(const char *buffer) {
    int a = (*buffer & 0b11000000) >> 6;
    return a == 3;
}

void getArgs(int argc, char **argv) {
    if (argc != 4) {
        printf("Get bad arguments. Progress will use default configuration.\n");
        return;
    }
    if (strcmp(argv[1], "-d") == 0)
        debug = 1;
    if (strcmp(argv[2], "0") != 0)
        fileName = argv[2];
    if (strcmp(argv[3], "0") != 0)
        dnsServerIP = argv[3];
}

double get_time(void) {
    clock_t end_time = clock();
    return (double) (end_time - start_time) / CLOCKS_PER_SEC;
}