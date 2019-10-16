#include "list.h"
#include <stdio.h>
#include <stdlib.h>


long mbx421_init(unsigned int ptrs, unsigned int prob);
long mbx421_create(unsigned long id);
long mbx421_destroy(unsigned long id);
long mbx421_read(unsigned long id);

typedef struct mailbox{
    int id;
};

typedef struct skip_list_node{
    struct skip_list_node* m_below;
    struct list_head m_link;
    int m_key;
    struct mailbox* m_mailbox;

};

struct list_head *levelHeads = NULL;
unsigned int numLevels = 0;
unsigned int probability = 0;

long mbx421_init(unsigned int ptrs, unsigned int prob){
    if(prob < 0)
        return -1;
    else if(ptrs < 1)
        return -1;
    else{
        probability = prob;
        numLevels = ptrs;
        levelHeads = malloc (ptrs * sizeof(struct list_head));
        for(int i = 0; i < ptrs; i++) {
            INIT_LIST_HEAD(NULL);
    }
        return 0;
    }
    }

long mbx421_create(unsigned long id){
    // initializes mailbox that is being added
    struct skip_list_node *temp = malloc(sizeof(struct skip_list_node));
    temp->m_below = NULL;
    temp->m_key = id;
    temp->m_link.next = NULL;
    temp->m_link.prev = NULL;
    temp->m_mailbox = malloc(sizeof(struct mailbox));

    //First item in List
    if(levelHeads[0].next == NULL){
        list_add(temp, listHeads[0]);

        static unsigned int next_random = 9001;

        static unsigned int generate_random_int(void) {
            next_random = next_random * 1103515245 + 12345;
            return (next_random / 65536) % 32768;
        }

        static void seed_random(unsigned int seed) {
            next_random = seed;
        }

        return 0;
    }
    else{


        return 0;
    }
}
long mbx421_destroy(unsigned long id){


}

