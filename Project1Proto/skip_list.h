#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct mailbox{
    unsigned int id;
} mailbox;

typedef struct skipListNode{
    int id;
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
unsigned int ACTIVE_LEVELS = 0;
unsigned int TOTAL_LEVELS = 0;
unsigned int PROBABILITY = 0;
bool INITIALIZED = false;

long init(unsigned int ptrs, unsigned int prob);
long insert(int id);
long removeNode(unsigned int id);
long search(unsigned int id);
void display();

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

long insert(int id){
    int currLevel = TOTAL_LEVELS;
    skipListNode **reassignment = malloc(TOTAL_LEVELS * sizeof(skipListNode *));
    for(unsigned int i = 0; i < TOTAL_LEVELS; i++){
        reassignment[i] = NULL;
    }
    // first node in list
    if(HEAD->next[0] == TAIL){
        printf("\t First node");
        skipListNode *newNode = malloc(sizeof(skipListNode));
        newNode->id = id;
        newNode->mailbox = malloc(sizeof(mailbox));
        newNode->mailbox->id = id;
        // flip coin
        unsigned int val = generate_random_int();
        unsigned int success = 1;
        while (success < TOTAL_LEVELS && val % PROBABILITY == 0) {
            success++;
        }
        newNode->next = malloc(success * sizeof(skipListNode *));
        newNode->numPtrs = success;
        // pointer reassignment
        for (unsigned int i = 0; i < success; i++) {
            newNode->next[i] = TAIL;
            HEAD->next[i] = newNode;
        }


        return 0;
    }
    // nromal insert
    else {
        printf("\t normal insert ");
        skipListNode *temp = HEAD;
        // breaks loop
        bool forward = true;
        // loop moves down
        while (currLevel >= 0) {
            // loop moves right
            while (id > temp->id && forward) {
                if(id > temp->next[currLevel]->id) {
                    temp = temp->next[currLevel];
                } else if (temp->next[currLevel] == TAIL && currLevel > 0) {
                    reassignment[currLevel] = temp;
                    currLevel--;
                    temp = temp->next[currLevel];
                } else {
                    forward = false;
                }

            }
            reassignment[currLevel] = temp;
            currLevel--;
        }


        // mailbox already exists
        if (temp->id == id) {
            // FREE UP MEMORY
            free(reassignment);
            return -1;
        } else {
            // create new node
            skipListNode *newNode = malloc(sizeof(skipListNode));
            newNode->id = id;
            newNode->mailbox = malloc(sizeof(mailbox));
            newNode->mailbox->id = id;
            // flip coin
            unsigned int val = generate_random_int();
            unsigned int success = 1;
            while (success <= TOTAL_LEVELS && val % PROBABILITY == 0) {
                success++;

            }
            // adjusts number of active levels
            if(success -1 > ACTIVE_LEVELS)
                ACTIVE_LEVELS = success - 1;
            newNode->next = malloc(success * sizeof(skipListNode *));
            newNode->numPtrs = success;
            // pointer reassignment
            for (unsigned int i = 0; i < success; i++) {
                newNode->next[i] = reassignment[i];
                reassignment[i]->next = &newNode;
            }
            // FREE UP MEMORY
            free(reassignment);
            return 0;
        }
    }

}

long addNode(int id){
    int currLevel = TOTAL_LEVELS - 1;
    skipListNode *temp = HEAD;
    skipListNode **reassignment = malloc(TOTAL_LEVELS * sizeof(skipListNode *));
    for(int i = 0; i < TOTAL_LEVELS; i++){
        reassignment[i] = NULL;
    }
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
    // creates new node
    skipListNode *newNode = malloc(sizeof(skipListNode));
    newNode->id = id;
    newNode->mailbox = malloc(sizeof(mailbox));
    newNode->mailbox->id = id;
    // flip coin
    unsigned int val = generate_random_int();
    unsigned int success = 1;
    while (success <= TOTAL_LEVELS && val % PROBABILITY == 0) {
        success++;
    }
    if(success -1 > ACTIVE_LEVELS)
        ACTIVE_LEVELS = success - 1;
    newNode->next = malloc(success * sizeof(skipListNode *));
    newNode->numPtrs = success;
    //reassignment
    for(int i = 0; i <= success - 1; i ++){
        newNode->next[i] = reassignment[i]->next[i];
        reassignment[i]->next[i] = newNode;
    }
    return 0;
}

long removeNode(unsigned int id){
    unsigned int currLevel = ACTIVE_LEVELS;
    skipListNode *temp = HEAD->next[currLevel];
    // loop moves down
    while(currLevel >= 0){
        // loop moves right
        while(id < temp->id) {
            temp = temp->next[currLevel];
            if (temp->next[currLevel]->id == -1 && currLevel > 0) {
                currLevel--;
                temp = HEAD->next[currLevel];
            }
        }
        currLevel--;
    }
    // node found
    if(temp->next[currLevel]->id == id) {
        // pointer reassignment
        for(unsigned int i = 0; i < temp->next[currLevel]->numPtrs; i++){
            temp->next[i] = temp->next[currLevel]->next[i];
        }
        return 0;
    }
    else {
        return -1;
    }
}

long search(unsigned int id){
    unsigned int currLevel = ACTIVE_LEVELS;
    skipListNode *temp = HEAD->next[currLevel];
    // loop moves down
    while(currLevel >= 0){
        // loop moves right
        while(id < temp->id) {
            temp = temp->next[currLevel];
            if (temp->next[currLevel]->id == -1 && currLevel > 0) {
                currLevel--;
                temp = HEAD->next[currLevel];
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

void display(){
    skipListNode *temp = HEAD;
    int currLevel = ACTIVE_LEVELS;
    for(int i = TOTAL_LEVELS - 1; i >= 0; i--) {
        // while temps next is less than id and temps next is not tail
        while (temp->next[currLevel] != TAIL) {
            temp = temp->next[currLevel];
            printf("%d", temp->id);
        }
        if(currLevel > 0){
            printf("\n");
            currLevel--;
            temp = HEAD->next[currLevel];
        }
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
unsigned int ACTIVE_LEVELS = 0;
unsigned int TOTAL_LEVELS = 0;
unsigned int PROBABILITY = 0;
unsigned int totalNodes = 0;

// INITIALIZE
long mbx421_init(unsigned int ptrs, unsigned int prob){
    if(prob < 0)
        return -1;
    else if(ptrs < 1)
        return -1;
    else{
        PROBABILITY = prob;
        TOTAL_LEVELS = ptrs;
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
        struct list_head pos = levelHeads[ACTIVE_LEVELS];
        int posID = pos.next+ sizeof(int);
        while(id < posID){
            if(pos.next == NULL && ACTIVE_LEVELS-- != 0){

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
    while(curLevel < TOTAL_LEVELS && checkLevel) {
        //Check if I need to move level
        seed_random((unsigned int) time(NULL));
        ranVal = generate_random_int();

        //moves up level
        if (ranVal % PROBABILITY == 0) {
            curLevel++;

            // creates node for next level up that points to the same mailbox
            struct skip_list_node *newTemp = malloc(sizeof(struct skip_list_node));
            newTemp->m_below = temp;
            newTemp->m_key = temp->m_key;
            newTemp->m_link.next = NULL;
            newTemp->m_link.prev = NULL;
            newTemp->m_mailbox = temp->m_mailbox;

            //If first node on level
            if (curLevel > ACTIVE_LEVELS) {
                ACTIVE_LEVELS++;


                list_add(newTemp, &levelHeads[curLevel]);
                totalNodes++;

            }
            //COPY OF NORMAL INSERT
            else{
                struct list_head pos = levelHeads[ACTIVE_LEVELS];
                int posID = pos.next+ sizeof(int);
                while(id < posID){
                    if(pos.next == NULL && ACTIVE_LEVELS-- != 0){

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
    unsigned int curLevel = ACTIVE_LEVELS;
    struct list_head pos = levelHeads[ACTIVE_LEVELS];
    unsigned int posID = pos.next + sizeof(int);
    while(id < posID) {
        if (pos.next == NULL && curLevel >= 0) {
            pos = levelHeads[ACTIVE_LEVELS];
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
    unsigned int curLevel = ACTIVE_LEVELS;
    struct list_head pos = levelHeads[ACTIVE_LEVELS];
    unsigned int posID = pos.next + sizeof(int);
    while(id < posID) {
        if (pos.next == NULL && curLevel >= 0) {
            pos = levelHeads[ACTIVE_LEVELS];
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