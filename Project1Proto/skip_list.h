#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct mailbox{
    unsigned int id;
};

typedef struct skipListNode{
    unsigned int id;
    unsigned int numPtrs;
    // Pointer to an array of next node pointers
    struct skipListNode **next;
    struct mailbox *mailbox;
};

static unsigned int next_random = 9001;

static unsigned int generate_random_int(void) {
    next_random = next_random * 1103515245 + 12345;
    return (next_random / 65536) % 32768;
}

static void seed_random(unsigned int seed) {
    next_random = seed;
}

struct skipListNode *head = NULL;
struct skipListNode *tail = NULL;
unsigned int activeLevels = 0;
unsigned int totalLevels = 0;
unsigned int probability = 0;

long init(unsigned int ptrs, unsigned int prob);
long insert(unsigned int id);
long removeNode(unsigned int id);
long search(unsigned int id);

long search(unsigned int id){
    unsigned int currLevel = activeLevels;
    struct skipListNode *temp = head->next[currLevel];
    // loop moves down
    while(currLevel >= 0){
        // loop moves right
        while(id < temp->id) {
            temp = temp->next[currLevel];
            if (temp->next[currLevel]->id == -1 && currLevel > 0) {
                currLevel--;
                temp = head->next[currLevel];
            }
        }
        currLevel--;
    }
    temp = temp->next[currLevel];
    if(temp->id == id)
        return 0;
    else
        return -1;
}

long removeNode(unsigned int id){
    unsigned int currLevel = activeLevels;
    struct skipListNode *temp = head->next[currLevel];
    // loop moves down
    while(currLevel >= 0){
        // loop moves right
        while(id < temp->id) {
            temp = temp->next[currLevel];
            if (temp->next[currLevel]->id == -1 && currLevel > 0) {
                currLevel--;
                temp = head->next[currLevel];
            }
        }
        currLevel--;
    }
    // node found
    if(temp->next[currLevel]->id == id) {
        // pointer reassignment
        for(int i = 0; i < temp->next[currLevel]->numPtrs; i++){
            temp->next[i] = temp->next[currLevel]->next[i];
        }
        return 0;
    }
    else {
        return -1;
    }
}

long insert(unsigned int id){
    unsigned int currLevel = activeLevels;
    struct skipListNode **reassignment = malloc(totalLevels * sizeof(struct skipListNode *));
    for(int i = 0; i < totalLevels; i++){
        reassignment[i] = NULL;
    }
    struct skipListNode *temp = head->next[currLevel];
    // loop moves down
    while(currLevel >= 0){
        // loop moves right
        while(id < temp->id){
            temp = temp->next[currLevel];
            if(temp->next[currLevel]->id == -1 && currLevel > 0){
                reassignment[currLevel] = temp;
                currLevel--;
                temp = head->next[currLevel];
            }
        }
        reassignment[currLevel] = temp;
        currLevel--;
    }
    // create new node
    struct skipListNode *newNode = malloc(sizeof(struct skipListNode));
    newNode->id = id;
    newNode->mailbox = malloc(sizeof(struct mailbox));
    // flip coin
    unsigned int val = generate_random_int();
    unsigned int success = 0;
    while(success <= totalLevels && val % probability == 0){
        success++;
    }
    newNode->next = malloc(success * sizeof(struct skipListNode *));
    // pointer reassignment
    for(int i = 0; i < success; i++){
        newNode->next[i] = reassignment[i];
        reassignment[i]->next = newNode;
    }
    // FREE UP MEMORY
    free(reassignment);
    free(newNode);
}

long init(unsigned int ptrs, unsigned int prob){
    totalLevels = ptrs;
    probability = prob;
    head = malloc(sizeof(struct skipListNode));
    head->id = -1;
    head->numPtrs = ptrs;
    head->next = malloc(totalLevels * sizeof(struct skipListNode));
    tail = malloc(sizeof(struct skipListNode));
    tail->id = -1;
    tail->numPtrs = 0;
    tail->next = malloc(sizeof(struct skipListNode));
    tail->next[0] = NULL;
    for(int i = 0; i < totalLevels; i++){
        head->next[i] = tail;
    }
}


/*
long mbx421_init(unsigned int ptrs, unsigned int prob);
long mbx421_create(unsigned long id);
long mbx421_destroy(unsigned long id);
long mbx421_read(unsigned int id);

typedef struct mailbox{
    int id;
};

typedef struct skip_list_node{
    struct skip_list_node* m_below;
    struct list_head m_link;
    int m_key;
    struct mailbox* m_mailbox;

};

static unsigned int next_random = 9001;

static unsigned int generate_random_int(void) {
    next_random = next_random * 1103515245 + 12345;
    return (next_random / 65536) % 32768;
}

static void seed_random(unsigned int seed) {
    next_random = seed;
}

// Globals
struct list_head *levelHeads = NULL;
unsigned int activeLevels = 0;
unsigned int totalLevels = 0;
unsigned int probability = 0;
unsigned int totalNodes = 0;

// INITIALIZE
long mbx421_init(unsigned int ptrs, unsigned int prob){
    if(prob < 0)
        return -1;
    else if(ptrs < 1)
        return -1;
    else{
        probability = prob;
        totalLevels = ptrs;
        levelHeads = malloc (ptrs * sizeof(struct list_head));
        for(int i = 0; i < ptrs; i++) {
            levelHeads[i].next = NULL;
            levelHeads[i].prev = NULL;
    }
        return 0;
    }
    }


// CREATE
long mbx421_create(unsigned long id){
    // initializes mailbox that is being added
    struct skip_list_node *temp = malloc(sizeof(struct skip_list_node));
    temp->m_below = NULL;
    temp->m_key = id;
    temp->m_link.next = NULL;
    temp->m_link.prev = NULL;
    temp->m_mailbox = malloc(sizeof(struct mailbox));
    int curLevel = 0;
    //First item in List
    if(levelHeads[0].next == NULL){
        list_add(temp, &levelHeads[0]);
        totalNodes++;
    }
    // nodes already exist before

    else{
        struct list_head pos = levelHeads[activeLevels];
        int posID = pos.next+ sizeof(int);
        while(id < posID){
            if(pos.next == NULL && activeLevels-- != 0){

            }
            pos = *pos.next;
            posID = pos.next + sizeof(int);
        }
        struct skip_list_node *below = &pos - sizeof(struct list_head);
        while(below != NULL){
            below = below->m_below;
        }
        posID = pos.next + sizeof(int);
        pos = *below->m_link.next;
        while(posID != id && posID < id){
            pos = *pos.next;
            posID = pos.next + sizeof(int);
        }
        list_add(temp, &pos);

    }

    // Handles if I need to add more levels
    unsigned int ranVal = 0;
    bool checkLevel = true;
    while(curLevel < totalLevels && checkLevel) {
        //Check if I need to move level
        seed_random((unsigned int) time(NULL));
        ranVal = generate_random_int();

        //moves up level
        if (ranVal % probability == 0) {
            curLevel++;

            // creates node for next level up that points to the same mailbox
            struct skip_list_node *newTemp = malloc(sizeof(struct skip_list_node));
            newTemp->m_below = temp;
            newTemp->m_key = temp->m_key;
            newTemp->m_link.next = NULL;
            newTemp->m_link.prev = NULL;
            newTemp->m_mailbox = temp->m_mailbox;

            //If first node on level
            if (curLevel > activeLevels) {
                activeLevels++;


                list_add(newTemp, &levelHeads[curLevel]);
                totalNodes++;

            }
            //COPY OF NORMAL INSERT
            else{
                struct list_head pos = levelHeads[activeLevels];
                int posID = pos.next+ sizeof(int);
                while(id < posID){
                    if(pos.next == NULL && activeLevels-- != 0){

                    }
                    pos = *pos.next;
                    posID = pos.next + sizeof(int);
                }
                struct skip_list_node *below = &pos - sizeof(struct list_head);
                while(below != NULL){
                    below = below->m_below;
                }
                posID = pos.next + sizeof(int);
                pos = *below->m_link.next;
                while(posID != id && posID < id){
                    pos = *pos.next;
                    posID = pos.next + sizeof(int);
                }
                list_add(temp, &pos);
            }


        }
        // stays on level
        else{
            checkLevel = false;
        }

        }
    }


// DESTROY
long mbx421_destroy(unsigned long id){
    unsigned int curLevel = activeLevels;
    struct list_head pos = levelHeads[activeLevels];
    unsigned int posID = pos.next + sizeof(int);
    while(id < posID) {
        if (pos.next == NULL && curLevel >= 0) {
            pos = levelHeads[activeLevels];
            curLevel--;
        } else {
            pos = *pos.next;
        }
        posID = pos.next + sizeof(int);
    }
    struct skip_list_node *below = &pos - sizeof(struct list_head);
    while(below != NULL){
        below = below->m_below;
    }
    if(posID == id) {
        __list_del(&pos, pos.next);
    }
    else {
        posID = pos.next + sizeof(int);
        pos = *below->m_link.next;
        while(posID != id && posID < id){
            pos = *pos.next;
            posID = pos.next + sizeof(int);
        }
        __list_del(&pos, pos.next);
    }
    //Mailbox not found
    if(curLevel < 0) {
        return -1;
    }


}
// READ
long mbx421_read(unsigned int id){
    unsigned int curLevel = activeLevels;
    struct list_head pos = levelHeads[activeLevels];
    unsigned int posID = pos.next + sizeof(int);
    while(id < posID) {
        if (pos.next == NULL && curLevel >= 0) {
            pos = levelHeads[activeLevels];
            curLevel--;
        } else {
            pos = *pos.next;
        }
        posID = pos.next + sizeof(int);
    }
    struct skip_list_node *below = &pos - sizeof(struct list_head);
    while(below != NULL){
        below = below->m_below;
    }
    if(posID == id) {
        // Found it
    }
    else {
        posID = pos.next + sizeof(int);
        pos = *below->m_link.next;
        while(posID != id && posID < id){
            pos = *pos.next;
            posID = pos.next + sizeof(int);
        }
        // Mailbox found
    }
        //Mailbox not found
        if(curLevel < 0) {
            return -1;
        }

}
 */