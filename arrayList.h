#ifndef DNSRELAY_ARRAYLIST_H

typedef struct Node {
    char name[200];
    char ip[200];
    struct Node *next;
} Node;

typedef Node *NodePtr;

typedef NodePtr ArrayList;

NodePtr addNode(NodePtr nodePtr, char *name, char *ip);

void freeAll(NodePtr nodePtr);

NodePtr findNode(NodePtr nodePtr, char *name);

NodePtr loadFromFile(char *fileName);

void writeToFile(char *fileName, NodePtr arrayList);

void printAll(NodePtr arrayList);

void addNodeToList(NodePtr headPtr, char *name, char *ip);

NodePtr getEnd(NodePtr headPtr);

#define DNSRELAY_ARRAYLIST_H

#endif //DNSRELAY_ARRAYLIST_H
