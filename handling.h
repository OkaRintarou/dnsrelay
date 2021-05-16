#ifndef DNSRELAY_HANDLING_H
#define DNSRELAY_HANDLING_H

#include <winsock2.h>
#include "arrayList.h"

#define ERR -1
#define QUE 0
#define ANS 1

extern HANDLE hMutexSock;//访问sock
extern HANDLE hMutexFlag;
extern HANDLE hMutexThread;//新建线程
extern HANDLE hMutexMsg;//访问ans数组
extern HANDLE hMutexSigns[10];//获取信息
extern HANDLE hMutexSign;//访问releaseM array
extern HANDLE hMutexList;

extern HANDLE cThread[10];
extern unsigned cThreadID[10];
extern int threadUsed[10];
extern int needClose[10];
extern char *answer[10];
extern int ansLen[10];
extern int ansID[10];
extern int releaseM[10];

extern SOCKET sock;
extern SOCKADDR_IN serverAddr;
extern SOCKADDR_IN dnsAddr;
extern const int port;

extern ArrayList list;
extern ArrayList listTmp;

extern char *fileName;
extern char *dnsServerIP;
extern int debug;

typedef struct Client {
    SOCKADDR_IN clientAddr;
    char recvBuffer[512];
    int n;
    int threadNum;
} Client;

unsigned __stdcall keyListener(void *flag);

unsigned __stdcall handlingClient(void *pArguments);

int findEmpty(const int a[]);

void closeThread();

int classifyMsg(const char *msg, int n);

int findAns(short ID);

void releaseSign();

short getID(char *msg);

void setID(short ID, char *buffer);

void setNotFound(char *buffer);

void setResponse(char *buffer);

int getName(char *destiny, const char *buffer, int n);

int setAns(char *buffer, int n, char *ip);

int setName(char *buffer, int n);

void setAA(char *buffer);

void getIP(char *buffer, int qLen, char *ip);

int judgeA(char *buffer, int qLen);

int isSavable(char *buffer, int qLen);

char *skipName(char *buffer);

int judgePointer(const char *buffer);

extern void getArgs(int argc, char *argv[]);


#endif //DNSRELAY_HANDLING_H

