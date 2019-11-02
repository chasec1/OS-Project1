#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/cred.h>
#include <linux/rwlock.h>

typedef struct mailbox{
    unsigned int aclSize;
    unsigned int aclMembers;
    pid_t * aclist;
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

SYSCALL_DEFINE2(mbx421_init, unsigned int, ptrs, unsigned int, prob){
    // already INITIALIZED
    if(INITIALIZED){
        return -EEXIST;
    }
    else {
        //seed_random((unsigned int)time(NULL));
        INITIALIZED = true;
        TOTAL_LEVELS = ptrs;
        PROBABILITY = prob;
        HEAD = kmalloc(sizeof(skipListNode), GFP_KERNEL);
        HEAD->id = -1;
        HEAD->numPtrs = ptrs;
        HEAD->next = kmalloc(TOTAL_LEVELS * sizeof(skipListNode), GFP_KERNEL);
        TAIL = kmalloc(sizeof(skipListNode), GFP_KERNEL);
        TAIL-> id = -1;
        TAIL-> numPtrs = 0;
        TAIL-> next = kmalloc(sizeof(skipListNode), GFP_KERNEL);
        TAIL->next[0] = NULL;
        unsigned int i = 0;
        for (i; i< TOTAL_LEVELS; i++) {
            HEAD->next[i] = TAIL;
        }
        return 0;
    }
}

SYSCALL_DEFINE0(mbx421_shutdown){
        //uninitialized skip list
    if(!INITIALIZED)
        return -ENODEV;

    skipListNode *temp = HEAD->next[0];
    unsigned int i = 0;
    for(i; i < TOTAL_NODES; i++){
        HEAD->next[0] = temp->next[0];
        kfree(temp->next);

        mail *mailPtr = temp->mailbox->head;
        while(mailPtr != NULL) {
            temp->mailbox->head = mailPtr->next;
            kfree(mailPtr->message);
            kfree(mailPtr);
            mailPtr = temp->mailbox->head;
        }

        kfree(temp->mailbox->head);
        kfree(temp->mailbox->aclist);
        kfree(temp->mailbox);
        kfree(temp);
        temp = HEAD->next[0];
    }
    kfree(HEAD->next);
    kfree(HEAD);
    kfree(TAIL->next);
    kfree(TAIL);
    INITIALIZED = false;
    return 0;
}

SYSCALL_DEFINE1(mbx421_create, unsigned long, id){
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
    skipListNode **reassignment = kmalloc(TOTAL_LEVELS * sizeof(skipListNode *), GFP_KERNEL);
    unsigned int i = 0;
    for(i; i < TOTAL_LEVELS; i++){
        reassignment[i] = NULL;
    }

    // wait for a rwlock here, we dont want the skip list to be changed while another process is navigating it

    // loop segfaults if i is made unsigned
    int j = TOTAL_LEVELS - 1;
    for(j; j >= 0; j--) {
        // while temps next is less than id and temps next is not tail
        while (temp->next[currLevel] != TAIL && temp->next[currLevel]->id < id) {
            temp = temp->next[currLevel];
        }
        reassignment[j] = temp;
        if(currLevel > 0){
            currLevel--;
        }
    }
    // mailbox already exists
    if(temp->next[0]->id == id){
        kfree(reassignment);
        return -EEXIST;
    }

    // creates new node
    skipListNode *newNode = kmalloc(sizeof(skipListNode), GFP_KERNEL);
    newNode->id = id;
    newNode->mailbox = kmalloc(sizeof(mailbox), GFP_KERNEL);
    newNode->mailbox->head = kmalloc(sizeof(mail), GFP_KERNEL);
    newNode->mailbox->tail = newNode->mailbox->head;
    newNode->mailbox->head->size = 0;
    newNode->mailbox->head->message = NULL;
    newNode->mailbox->tail->size = 0;
    newNode->mailbox->tail->message = NULL;
    newNode->mailbox->head->next = NULL;
    newNode->mailbox->numMessages = 0;
    // 4 is arbitrary size, this array will grow as needed
    newNode->mailbox->aclSize = 4;
    newNode->mailbox->aclMembers = 1;
    newNode->mailbox->aclist = NULL;

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
    newNode->next = kmalloc(success * sizeof(skipListNode *), GFP_KERNEL);
    newNode->numPtrs = success;

    //reassignment
    i = 0;
    for(i; i <= success - 1; i ++){
        newNode->next[i] = reassignment[i]->next[i];
        reassignment[i]->next[i] = newNode;
    }

    // signal rw lock here, alterations to list are complete

    TOTAL_NODES++;
    kfree(reassignment);
    return 0;
}

SYSCALL_DEFINE1(mbx421_destroy, unsigned long, id){
    //uninitialized skip list
    if(!INITIALIZED)
        return -ENODEV;

    //bad ID
    if (id < 0)
        return -ENOENT;

    unsigned int currLevel = ACTIVE_LEVELS;
    skipListNode *temp = HEAD;
    skipListNode **reassignment = kmalloc(ACTIVE_LEVELS * sizeof(skipListNode *), GFP_KERNEL);

    // wait for a rwlock here, we dont want the skip list to be changed while another process is navigating it

    // loop moves down
    // segfaults if i is made unsigned
    int i = ACTIVE_LEVELS;
    for (i; i >= 0; i--) {
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
        unsigned int j = 0;
        for(j; j < temp->numPtrs; j++){
            reassignment[i]->next[j] = temp->next[j];
        }
        TOTAL_NODES--;
        kfree(temp->next);

        // frees up the mail linked list within the mailbox
        mail *mailPtr = temp->mailbox->head;
        while(mailPtr != NULL) {
            temp->mailbox->head = mailPtr->next;
            kfree(mailPtr->message);
            kfree(mailPtr);
            mailPtr = temp->mailbox->head;
        }
        kfree(temp->mailbox->head);
        kfree(temp->mailbox->aclist);
        kfree(temp->mailbox);
        kfree(temp);
        kfree(reassignment);
        return 0;
    }

    // signal rw lock, all changes have been made

    // mailbox not found
    else {
        kfree(reassignment);
        return -ENOENT;
    }
}

SYSCALL_DEFINE1(mbx421_count, unsigned long, id){
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

    // wait rwlock for traversal

    // loop moves down
    int i = ACTIVE_LEVELS;
    for(i; i >= 0; i--) {
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

    // signal rwlock traversal complete

    // mailbox not found
    if(currBox == NULL){
        return -ENOENT;
    }

    return currBox->mailbox->numMessages;
}

SYSCALL_DEFINE3(mbx421_send, unsigned long, id, const unsigned char __user, *msg, long, len){
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

    // wait for rw lock so traversal can be performed

    // loop moves down
    int i = ACTIVE_LEVELS;
    for(i; i >= 0; i--) {
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
    // signal rwlock traversal complete
    // mailbox found

    bool accessible = false;
    if(currBox->mailbox->aclist == NULL){
        accessible = true;
    }
    else{
        int a = 0;
        for(a; a < currBox->mailbox->numMessages; a++){
            if(currBox->mailbox->aclist[i] == current->pid){
                accessible = true;
            }
        }
    }

    //wait for rwlock to be available so mail can be retrieved

    if(accessible){
        // without +16 I was getting a weird error and this corrected it, I am not sure why
        mail *newMail = kmalloc(sizeof(mail) + 16, GFP_KERNEL);
        newMail->size = len;
        newMail->message = kmalloc(sizeof(char) * len, GFP_KERNEL);
        memcpy(newMail->message,msg,len);
        currBox->mailbox->tail->next = newMail;
        newMail->next = NULL;
        currBox->mailbox->
        tail = newMail;
        currBox->mailbox->numMessages += 1;
        return 0;
    }

    // signal rw lock changes finished

    else{
        return -EPERM;
    }

}

SYSCALL_DEFINE3(mbx421_recv, unsigned long, id, unsigned char __user, *msg, long, len){
   //uninitialized skip list
    if(!INITIALIZED) {
        return -ENODEV;
    }
    //bad ID
    if(id < 0) {
        return -ENOENT;
    }
    skipListNode *currBox = NULL;
    unsigned int currLevel = ACTIVE_LEVELS;
    skipListNode *temp = HEAD;

    // wait for rwlock so traversal can be done

    // loop moves down
    int i = ACTIVE_LEVELS;
    for(i; i >= 0; i--) {
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

    //signal rwlock traversal complete

    // mailbox not found
    if(currBox == NULL){
        return -ENOENT;
    }

    // mailbox empty
    if(currBox->mailbox->numMessages == 0){
        return -ESRCH;
    }

    bool accessible = false;
    if(currBox->mailbox->aclist == NULL){
        accessible = true;
    }
    else{
        int a = 0;
        for(a; a < currBox->mailbox->numMessages; a++){
            if(currBox->mailbox->aclist[i] == current->pid){
                accessible = true;
            }
        }
    }

    //wait rw lock, dont want messages to change before they can be recieved
    if(accessible){
    // copies items in kernel memory to user memory
    memcpy(msg, currBox->mailbox->head->next->message, len);

    // manages the mail linked list
    mail *mailTmp = currBox->mailbox->head->next->next;
    kfree(currBox->mailbox->head->next->message);
    kfree(currBox->mailbox->head->next);
    currBox->mailbox->head->next = mailTmp;
    currBox->mailbox->numMessages -= 1;
    if(currBox->mailbox->numMessages == 0){
        currBox->mailbox->tail = currBox->mailbox->head;
    }
    // signal rwlock accessing of mail complete
    return 0;
    }
    else{
        return -EPERM;
    }
}

SYSCALL_DEFINE1(mbx421_length, unsigned long, id){
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

    // wait rwlock for traversal

    // loop moves down
    int i = ACTIVE_LEVELS;
    for(i; i >= 0; i--) {
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

    //signal rwlock traversal complete

    // mailbox not found
    if(currBox == NULL){
        return -ENOENT;
    }
    // no messages in mailbox
    if(currBox->mailbox->numMessages == 0){
        return -ESRCH;
    }
    else{
        return currBox->mailbox->head->next->size;
    }
    }



SYSCALL_DEFINE2(mbx421_acl_add, unsigned long, id, pid_t, process_id){
//uninitialized skip list
    if(!INITIALIZED) {
        return -ENODEV;
    }
    //bad ID
    if(id < 0) {
        return -ENOENT;
    }

    uid_t uid = current_uid().val;

    if(uid > 0){
        return EPERM;
    }

    skipListNode *currBox = NULL;
    unsigned int currLevel = ACTIVE_LEVELS;
    skipListNode *temp = HEAD;

    // wait rwlock for skip list traversal

    // loop moves down
    int i = ACTIVE_LEVELS;
    for(i; i >= 0; i--) {
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

    // signal rwlock traversal complete

    // mailbox not found
    if(currBox == NULL){
        return -ENOENT;
    }

    //wait rw lock dont want to make changes to acl structure while another process is

    if(currBox->mailbox->aclist == NULL){
       currBox->mailbox->aclist = kmalloc(currBox->mailbox->aclSize * sizeof(unsigned int), GFP_KERNEL);
    }
    currBox->mailbox->aclMembers += 1;
    if(currBox->mailbox->aclMembers > currBox->mailbox->aclSize){
        currBox->mailbox->aclist = krealloc(currBox->mailbox->aclist, currBox->mailbox->aclSize * 2, GFP_KERNEL);
        currBox->mailbox->aclSize *= 2;
    }
    currBox->mailbox->aclist[currBox->mailbox->aclMembers - 1] = process_id;

    // alterations finished signal rwlock

    return 0;
}

SYSCALL_DEFINE2(mbx421_acl_remove, unsigned long, id, pid_t, process_id){
   //uninitialized skip list
    if(!INITIALIZED) {
        return -ENODEV;
    }
    //bad ID
    if(id < 0) {
        return -ENOENT;
    }

    uid_t uid = current_uid().val;

    if(uid > 0){
        return EPERM;
    }

    skipListNode *currBox = NULL;
    unsigned int currLevel = ACTIVE_LEVELS;
    skipListNode *temp = HEAD;

   //wait rwlock for skip list traversal

    // loop moves down
    int i = ACTIVE_LEVELS;
    for(i; i >= 0; i--) {
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

    //traversal complete signal rwlock

    // mailbox not found
    if(currBox == NULL){
        return -ENOENT;
    }

    // wait rwlock so edits can be made uniterrupted

    int j = 0;
    for(j; j <currBox->mailbox->aclMembers; j++){
        // if process id is found
        if(currBox->mailbox->aclist[j] == process_id){
            // loop starts at found id and shifts all to the right of the deleted left
            int k = j;
            for(k; k < currBox->mailbox->aclMembers; k++){
                currBox->mailbox->aclist[k] = currBox->mailbox->aclist[k + 1];
            }
            currBox->mailbox->aclMembers -= 1;
        }
    }

    // edits finished signal rwlock

    return 0;

}

SYSCALL_DEFINE0(skip_list_display){
    skipListNode *temp = HEAD;
    unsigned long currLevel = ACTIVE_LEVELS;
    // segfaults if i is made unsigned
    int i = ACTIVE_LEVELS;
    for(i; i >= 0; i--) {
        // while temps next is less than id and temps next is not tail
        while (temp->next[currLevel] != TAIL) {
            temp = temp->next[currLevel];
            printk("%ld", temp->id);
            // prints mailboxes
            if(i == 0){
                mail *mailPtr = temp->mailbox->head->next;
                printk("num messages = %d\n", temp->mailbox->numMessages);
                int j = 0;
                for(j; j < temp->mailbox->numMessages; j++) {
                    printk("Message %s", mailPtr->message);
                    mailPtr = mailPtr->next;
                }
            }
        }
        if(currLevel > 0){
            printk("\n");
            currLevel--;
            temp = HEAD;
        }
    }
    printk("\n");
}


