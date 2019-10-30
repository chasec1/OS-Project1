#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct mailbox{
    unsigned int numMessages;
    struct mail *head;
    struct mail *tail;
} mailbox;

typedef struct mail{
  unsigned int size;
  struct mail * next;
  unsigned char* message;
} mail;

typedef struct skipListNode{
    unsigned long id;
    unsigned int numPtrs;
    // Pointer to an array of next node pointers
    struct skipListNode **next;
    struct mailbox *mailbox;
} skipListNode;

static unsigned int next_random = 9001;

static unsigned int generate_random_int(void) {
    next_random = next_random * 1103515245 + 12345;
    return (next_random / 65536) % 32768;
}

static void seed_random(unsigned int seed) {
    next_random = seed;
}

// GLOBAL VARIABLES
struct skipListNode *HEAD = NULL;
struct skipListNode *TAIL = NULL;
unsigned int TOTAL_NODES = 0;
unsigned int ACTIVE_LEVELS = 0;
unsigned int TOTAL_LEVELS = 0;
unsigned int PROBABILITY = 0;
bool INITIALIZED = false;

long init(unsigned int ptrs, unsigned int prob);
long addNode(unsigned long id);
long removeNode(unsigned long id);
skipListNode* search(unsigned long id);
void display();
void cleanUp();


long send(unsigned long id, const unsigned char *msg, long len);
long recv(unsigned long id, unsigned char *msg, long len);


long init(unsigned int ptrs, unsigned int prob){
    // already INITIALIZED
    if(INITIALIZED){
        return -1;
    }
    else {
        INITIALIZED = true;
        TOTAL_LEVELS = ptrs;
        PROBABILITY = prob;
        HEAD = malloc(sizeof(skipListNode));
        HEAD->id = -1;
        HEAD->numPtrs = ptrs;
        HEAD->next = malloc(TOTAL_LEVELS * sizeof(skipListNode));
        TAIL = malloc(sizeof(skipListNode));
        TAIL->id = -1;
        TAIL->numPtrs = 0;
        TAIL->next = malloc(sizeof(skipListNode));
        TAIL->next[0] = NULL;
        for (unsigned int i = 0; i < TOTAL_LEVELS; i++) {
            HEAD->next[i] = TAIL;
        }
        return 0;
    }
}


long addNode(unsigned long id){
    // Bad ID
    if(id < 0){
        return -1;
    }
    unsigned int currLevel = TOTAL_LEVELS - 1;
    skipListNode *temp = HEAD;
    skipListNode **reassignment = malloc(TOTAL_LEVELS * sizeof(skipListNode *));
    for(unsigned int i = 0; i < TOTAL_LEVELS; i++){
        reassignment[i] = NULL;
    }

    // loop segfaults if i is made unsigned
    for(int i = TOTAL_LEVELS - 1; i >= 0; i--) {
        // while temps next is less than id and temps next is not tail
        while (temp->next[currLevel] != TAIL && temp->next[currLevel]->id < id) {
            temp = temp->next[currLevel];
        }
        reassignment[i] = temp;
        if(currLevel > 0){
            currLevel--;
        }
    }
    // mailbox already exists
    if(temp->next[0]->id == id){
        free(reassignment);
        return -1;
    }
    // creates new node
    skipListNode *newNode = malloc(sizeof(skipListNode));
    newNode->id = id;
    newNode->mailbox = malloc(sizeof(mailbox));
    newNode->mailbox->head = malloc(sizeof(mail));
    newNode->mailbox->tail = malloc(sizeof(mail));
    newNode->mailbox->head->size = 0;
    newNode->mailbox->head->message = NULL;
    newNode->mailbox->tail->size = 0;
    newNode->mailbox->tail->message = NULL;
    newNode->mailbox->head->next = newNode->mailbox->tail;
    newNode->mailbox->tail->next = newNode->mailbox->head;
    /*
    // 4 is arbitrary size that can be doubled later if necessary
    newNode->mailbox->bufferSize = 4;
    newNode->mailbox->head = 0;
    */
    newNode->mailbox->numMessages = 0;

    //newNode->mailbox->messages = malloc(newNode->mailbox->bufferSize * sizeof(char *));

    // flip coin
    unsigned int val = generate_random_int();
    unsigned int success = 1;
    while (success < TOTAL_LEVELS && val % PROBABILITY == 0) {
        success++;
        val = generate_random_int();
    }
    if(success -1 > ACTIVE_LEVELS)
        ACTIVE_LEVELS = success - 1;
    newNode->next = malloc(success * sizeof(skipListNode *));
    newNode->numPtrs = success;
    //reassignment
    for(unsigned int i = 0; i <= success - 1; i ++){
        newNode->next[i] = reassignment[i]->next[i];
        reassignment[i]->next[i] = newNode;
    }
    TOTAL_NODES++;
    free(reassignment);/*
        for(int i = 0; i < temp->mailbox->numMessages; i++){
            free(temp->mailbox->messages[temp->mailbox->head]);
            temp->mailbox->head += 1;

            // loops back around if needed
            if(temp->mailbox->head > temp->mailbox->bufferSize)
                temp->mailbox->head = 0;
        }
        free(temp->mailbox->messages);
         */
    return 0;
}

long removeNode(unsigned long id) {
    //bad ID
    if (id < 0)
        return -1;
    unsigned int currLevel = ACTIVE_LEVELS;
    skipListNode *temp = HEAD;
    skipListNode **reassignment = malloc(ACTIVE_LEVELS * sizeof(skipListNode *));
    // loop moves down
    // segfaults if i is made unsigned
    for (int i = ACTIVE_LEVELS; i >= 0; i--) {
        // loop moves right
        while (temp->next[currLevel] != TAIL && temp->next[currLevel]->id < id) {
            temp = temp->next[currLevel];
        }
        reassignment[i] = temp;
        if (currLevel > 0)
            currLevel--;
    }
    temp = temp->next[currLevel];
    // mailbox found
    if (temp->id == id){
        for(unsigned int i = 0; i < temp->numPtrs; i++){
            reassignment[i]->next[i] = temp->next[i];
        }
        TOTAL_NODES--;
        printf("FREEING ");
        printf("%ld\n", temp->id);
        free(temp->next);
        //free(temp->mailbox->messages);
        /*
        // NEED TO ITERATE OVER MAIL LINKED LIST AND DELETE ALL
        mail *mailPtr = temp->mailbox->head->next;
        while(mailPtr->next != temp->mailbox->tail){
            mailPtr = mailPtr->next;
            free(temp->mailbox->head->next);
            temp->mailbox->head->next = mailPtr;
        }
        free(temp->mailbox->head);
        free(temp->mailbox->tail);
         */
        free(temp->mailbox);
        free(temp);
        free(reassignment);
        return 0;
    }
    // mailbox not found
    else {
        free(reassignment);
        return -1;
    }
}

skipListNode* search(unsigned long id){
    //bad ID
    if(id < 0) {
        // throw error
        // return -1;
    }
    unsigned int currLevel = ACTIVE_LEVELS;
    skipListNode *temp = HEAD;
    // loop moves down
    for(int i = ACTIVE_LEVELS; i >= 0; i--) {
        // loop moves right
        while (temp->next[currLevel] != TAIL && temp->next[currLevel]->id < id) {
            temp = temp->next[currLevel];
        }
        // mailbox found
        if(temp->next[currLevel]->id == id) {
            temp = temp->next[currLevel];
            return temp;
        }
        if(currLevel > 0) {
            currLevel--;
        }
    }
    // mailbox not found
    //return -1;
}

void display(){
    skipListNode *temp = HEAD;
    unsigned long currLevel = ACTIVE_LEVELS;
    // segfaults if i is made unsigned
    for(int i = ACTIVE_LEVELS; i >= 0; i--) {
        // while temps next is less than id and temps next is not tail
        while (temp->next[currLevel] != TAIL) {
            temp = temp->next[currLevel];
            printf("%ld", temp->id);
        }
        if(currLevel > 0){
            printf("\n");
            currLevel--;
            temp = HEAD;
        }
    }
    printf("\n");
}

void cleanUp(){
    skipListNode *temp = HEAD->next[0];
for(unsigned int i = 0; i < TOTAL_NODES; i++){
        HEAD->next[0] = temp->next[0];
        printf("FREEING ");
        printf("%ld\n", temp->id);
        free(temp->next);

        /*
        mail *mailPtr = temp->mailbox->head->next;
        while(mailPtr != temp->mailbox->tail){
            mailPtr = mailPtr->next;
            free(temp->mailbox->head->next);
            temp->mailbox->head->next = mailPtr;
        }
        free(temp->mailbox->head);
        free(temp->mailbox->tail);
        */
        free(temp->mailbox);
        free(temp);
        temp = HEAD->next[0];
    }
    free(HEAD->next);
    free(HEAD);
    free(TAIL->next);
    free(TAIL);
}

// ============= MAILBOX STUFF =============

long send(unsigned long id, const unsigned char *msg, long len){
    // bad id
    if(id < 0){
        return -1;
    }
    skipListNode *currBox = search(id);
    mail *newMail = malloc(sizeof(mail));
    newMail->size = len;
    newMail->message = malloc(sizeof(msg));
    memcpy(newMail->message,msg,len);
    currBox->mailbox->tail->next->next = newMail;
    newMail->next = currBox->mailbox->tail;
    currBox->mailbox->tail = newMail;
    currBox->mailbox->numMessages += 1;
    printf("%s\n", currBox->mailbox->head->next->message);
    return 0;

}

long recv(unsigned long id, unsigned char *msg, long len){
    // bad id
    if(id < 0){
        return -1;
    }

    skipListNode *currBox = search(id);
    // mailbox empty
    if(currBox->mailbox->numMessages == 0){
        return -1;
    }
    memcpy(msg, currBox->mailbox->head->next->message, len);
    mail *temp = currBox->mailbox->head->next->next;
    free(currBox->mailbox->head->next);
    currBox->mailbox->head->next = temp;
    printf("%s\n", msg);
    return 0;
}