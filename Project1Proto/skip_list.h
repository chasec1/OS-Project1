#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

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
long cleanUp();


long send(unsigned long id, const unsigned char *msg, long len);
long recv(unsigned long id, unsigned char *msg, long len);


// Checks if skip list has already been initialized, if not sets global variables and allocates appropriate memory
long init(unsigned int ptrs, unsigned int prob){
    // already INITIALIZED
    if(INITIALIZED){
        return -EEXIST;
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
    //uninitialized skip list
    if(!INITIALIZED)
        return -ENODEV;
    // Bad ID
    if(id < 0){
        return -ENOENT;
    }
    unsigned int currLevel = TOTAL_LEVELS - 1;
    skipListNode *temp = HEAD;
    // reassignment will be used to save nodes where temp traverses down and may need to be reassigned later
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
        return -EEXIST;
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
    newNode->mailbox->numMessages = 0;

    // flip coin
    unsigned int val = generate_random_int();
    unsigned int success = 1;
    while (success < TOTAL_LEVELS && val % PROBABILITY == 0) {
        success++;
        val = generate_random_int();
    }
    if(success -1 > ACTIVE_LEVELS)
        ACTIVE_LEVELS = success - 1;
    // allocates the new nodes array of next pointers
    newNode->next = malloc(success * sizeof(skipListNode *));
    newNode->numPtrs = success;

    //reassignment
    for(unsigned int i = 0; i <= success - 1; i ++){
        newNode->next[i] = reassignment[i]->next[i];
        reassignment[i]->next[i] = newNode;
    }
    TOTAL_NODES++;
    free(reassignment);
    return 0;
}

long removeNode(unsigned long id) {
    //uninitialized skip list
    if(!INITIALIZED)
        return -ENODEV;

    //bad ID
    if (id < 0)
        return -ENOENT;

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
    // the two loops will iterate until temp is pointing to the node prior to the one we want
    temp = temp->next[currLevel];
    // mailbox found
    if (temp->id == id){
        for(unsigned int i = 0; i < temp->numPtrs; i++){
            reassignment[i]->next[i] = temp->next[i];
        }
        TOTAL_NODES--;
        free(temp->next);

        // frees up the mail linked list within the mailbox
        mail *mailPtr = temp->mailbox->head->next;
        for(unsigned int j = 0; j < temp->mailbox->numMessages; j++){
            temp->mailbox->head = mailPtr->next;
            free(mailPtr);
            mailPtr = temp->mailbox->head->next;
        }
        free(temp->mailbox->head);
        free(temp->mailbox->tail);
        free(temp->mailbox);
        free(temp);
        free(reassignment);
        return 0;
    }
    // mailbox not found
    else {
        free(reassignment);
        return -ENOENT;
    }
}

// will return a pointer to the node with a matching ID
skipListNode* search(unsigned long id){
    /*
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
*/
}

void display(){
    skipListNode *temp = HEAD;
    unsigned long currLevel = ACTIVE_LEVELS;
    // segfaults if i is made unsigned
    for(int i = ACTIVE_LEVELS; i >= 0; i--) {
        // while temps next is less than id and temps next is not tail
        while (temp->next[currLevel] != TAIL) {
            temp = temp->next[currLevel];
        }
        if(currLevel > 0){
            currLevel--;
            temp = HEAD;
        }
    }
}

long cleanUp(){
    //uninitialized skip list
    if(!INITIALIZED)
        return -ENODEV;

    skipListNode *temp = HEAD->next[0];
    for(unsigned int i = 0; i < TOTAL_NODES; i++){
        HEAD->next[0] = temp->next[0];
        free(temp->next);

        mail *mailPtr = temp->mailbox->head->next;
        for(unsigned int j = 0; j < temp->mailbox->numMessages; j++){
            temp->mailbox->head = mailPtr->next;
            free(mailPtr);
            mailPtr = temp->mailbox->head->next;
        }
        free(temp->mailbox->head);
        free(temp->mailbox->tail);
        free(temp->mailbox);
        free(temp);
        temp = HEAD->next[0];
    }
    free(HEAD->next);
    free(HEAD);
    free(TAIL->next);
    free(TAIL);
    return 0;
}

long send(unsigned long id, const unsigned char *msg, long len){

    /*
    // bad id
    if(id < 0){
        return -1;
    }
     skipListNode *currBox = search(id);


    */

    //uninitialized skip list
    if(!INITIALIZED)
        return -ENODEV;

    //bad ID
    if(id < 0) {
        return -ENOENT;
    }
    skipListNode *currBox = NULL;
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
            currBox = temp;
        }
        if(currLevel > 0) {
            currLevel--;
        }
    }
    // mailbox not found
    if(currBox == NULL){
        return -ENOENT;
    }

    // without +16 I was getting a weird error and this corrected it, I am not sure why
    mail *newMail = malloc(sizeof(mail) +16 );
    newMail->size = len;
    newMail->message = malloc(sizeof(char)*len);
    memcpy(newMail->message,msg,len);
    currBox->mailbox->tail->next->next = newMail;
    newMail->next = currBox->mailbox->tail;
    currBox->mailbox->tail = newMail;
    currBox->mailbox->numMessages += 1;
    return 0;

}

long recv(unsigned long id, unsigned char *msg, long len){
    /*
    // bad id
    if(id < 0){
        return -1;
    }

    skipListNode *currBox = search(id);
    */
    //uninitialized skip list
    if(!INITIALIZED)
        return -ENODEV;

    //bad ID
    if(id < 0) {
        return -ENOENT;
    }
    skipListNode *currBox = NULL;
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
            currBox = temp;
        }
        if(currLevel > 0) {
            currLevel--;
        }
    }
    // mailbox not found
    if(currBox == NULL){
        return -ENOENT;
    }

    // mailbox empty
    if(currBox->mailbox->numMessages == 0){
        return -ESRCH;
    }

    // copies items in kernel memory to user memory
    memcpy(msg, currBox->mailbox->head->next->message, len);

    // manages the mail linked list
    mail *mailTmp = currBox->mailbox->head->next->next;
    free(currBox->mailbox->head->next->message);
    free(currBox->mailbox->head->next);
    currBox->mailbox->head->next = mailTmp;
    currBox->mailbox->numMessages -= 1;
    if(currBox->mailbox->numMessages == 0){
        currBox->mailbox->tail = currBox->mailbox->head;
        currBox->mailbox->head = currBox->mailbox->tail;
    }
    return 0;
}