//
// Created by chasec1 on 10/16/19.
//

#include "skip_list.h"
int main(){
    long success = 0;
    seed_random((unsigned int)time(NULL));
    success = init(4,100);
    printf("INIT");
    printf("%ld\n", success);

    success = insert(20);
    printf("INSERT 20");
    printf("%ld\n", success);

    success = insert(1);
    printf("INSERT 1");
    printf("%ld\n", success);
    printf("INSERT 20");

    success = insert(20);
    printf("%ld\n", success);
    printf("INSERT 10");

    success = search(10);
    printf("%ld\n", success);

    success = removeNode(10);
    printf("REMOVE 10");
    printf("%ld\n", success);
    return 0;
}