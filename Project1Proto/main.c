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


    // SEND ++++++++++
    unsigned char message1[7] = "hello!";
    success = send(10, message1, 7);
    printf("send ");
    printf("%ld\n", success);
    // SEND +++++++++++
    unsigned char message2[7] = "world!";
    success = send(10, message2, 7);
    printf("send ");
    printf("%ld\n", success);

    display();

    // RECV +++++++++++
    unsigned char newMsg1;
    success = recv(10, &newMsg1, 7);
    printf("receive ");
    printf("%s\n", &newMsg1);
    printf("%ld\n", success);



    /*
    success = removeNode(10);
    printf("REMOVE 10 ");
    printf("%ld\n", success);
    display();
    */




    unsigned char newMsg2;
    success = recv(10, &newMsg2, 7);
    printf("receive  ");
    printf("%s\n", &newMsg2);
    printf("%ld\n", success);

    display();

    success = acl_add(10, 7);
    printf("acl add  ");
    printf("%ld\n", success);

    success = acl_add(10, 8);
    printf("acl add  ");
    printf("%ld\n", success);

    success = acl_add(10, 9);
    printf("acl add  ");
    printf("%ld\n", success);

    success = acl_rem(10, 8);
    printf("acl rem  ");
    printf("%ld\n", success);


    cleanUp();
    /*
    success = init(8,2);
    printf("INIT");
    printf("%ld\n", success);

    for(int i = 0; i < 50; i++){
        success = addNode(i);
        printf("INSERT %d", i);
        printf("%ld\n", success);
    }
    for(int i = 0; i < 50; i+=2){
        unsigned char msg[3] = "hi";
        success = send(i, msg, 3);
        printf("send ");
        printf("%ld\n", success);
    }
    display();
     */
    return 0;
}