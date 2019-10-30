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


    success = addNode(10);
    printf("INSERT 10 ");
    printf("%ld\n", success);
    display();


    success = removeNode(9);
    printf("REMOVE 9 ");
    printf("%ld\n", success);
    display();

    /*
    unsigned char message[3] = "hi";
    success = send(20, message, 3);
    printf("send 101 ");
    printf("%ld\n", success);

    unsigned char newMsg;
    success = recv(20, &newMsg, 3);
    printf("receive 101 ");
    printf("%s\n", &newMsg);
    printf("%ld\n", success);
    */
    /*
    success = search(10);
    printf("SEARCH 10 ");
    printf("%ld\n", success);
    */
    cleanUp();

    return 0;
}