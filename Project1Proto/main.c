//
// Created by chasec1 on 10/16/19.
//

#include "skip_list.h"
int main(){
    long success = 0;
    seed_random((unsigned int)time(NULL));
    success = init(4,2);
    printf("INIT");
    printf("%ld\n", success);

    success = addNode(1);
    printf("INSERT 1 ");
    printf("%ld\n", success);
    //display();

    success = addNode(20);
    printf("INSERT 20 ");
    printf("%ld\n", success);


    success = addNode(-100);
    printf("INSERT -100 ");
    printf("%ld\n", success);
    display();

    /*
    success = search(10);
    printf("SEARCH 10");
    printf("%ld\n", success);

    success = removeNode(10);
    printf("REMOVE 10");
    printf("%ld\n", success);
    */
    return 0;
}