#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arrayList.h"

NodePtr addNode(NodePtr currentPtr, char *name, char *ip) {
    currentPtr->next = calloc(1, sizeof(Node));
    currentPtr = currentPtr->next;
    currentPtr->next = NULL;
    strncpy(currentPtr->name, name, 200);
    strncpy(currentPtr->ip, ip, 200);
    return currentPtr;
}

void freeAll(NodePtr nodePtr) {
    NodePtr currentPtr = nodePtr;
    while (currentPtr != NULL) {
        NodePtr nextPtr = currentPtr->next;
        free(currentPtr);
        currentPtr = nextPtr;
    }
}

NodePtr findNode(NodePtr nodePtr, char *name) {
    NodePtr currentPtr = nodePtr->next;
    while (currentPtr != NULL) {
        if (strcmp(name, currentPtr->name) == 0)
            return currentPtr;
        currentPtr = currentPtr->next;
    }
    return NULL;
}

NodePtr loadFromFile(char *fileName) {
    NodePtr headPtr = calloc(1, sizeof(Node));
    NodePtr currentPtr = headPtr;
    char name[200];
    char ip[200];
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        printf("Open file error.\n");
        exit(-1);
    }
    while (!feof(fp)) {
        int n = fscanf(fp, "%s %s", name, ip);
        if (n == 2)
            currentPtr = addNode(currentPtr, name, ip);
    }
    fclose(fp);
    return headPtr;
}

void writeToFile(char *fileName, NodePtr arrayList) {
    FILE *fp = fopen(fileName, "w");
    NodePtr currentPtr = arrayList->next;
    while (currentPtr != NULL) {
        fprintf(fp, "%s %s\n", currentPtr->name, currentPtr->ip);
        currentPtr = currentPtr->next;
    }
    fclose(fp);
}

void printAll(NodePtr arrayList) {
    NodePtr currentPtr = arrayList->next;
    while (currentPtr != NULL) {
        printf("%s %s\n", currentPtr->name, currentPtr->ip);
        currentPtr = currentPtr->next;
    }
}

void addNodeToList(NodePtr headPtr, char *name, char *ip) {
    NodePtr endPtr = getEnd(headPtr);
    endPtr->next = calloc(1, sizeof(Node));
    endPtr = endPtr->next;
    endPtr->next = NULL;
    strncpy(endPtr->name, name, 200);
    strncpy(endPtr->ip, ip, 200);
}

NodePtr getEnd(NodePtr headPtr) {
    NodePtr currentPtr = headPtr;
    NodePtr nextPtr = currentPtr->next;
    while (nextPtr != NULL) {
        currentPtr = nextPtr;
        nextPtr = currentPtr->next;
    }
    return currentPtr;
}
