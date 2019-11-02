//
// Created by chasec1 on 10/31/19.
//

#include <stdio.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/syscall.h>


#define sys_mbx421_init 434
#define sys_mbx421_shutdown 435
#define sys_mbx421_create 436
#define sys_mbx421_destroy 437
#define sys_mbx421_count 438
#define sys_mbx421_send 439
#define sys_mbx421_recv 440
#define sys_mbx421_length 441
#define sys_mbx421_acl_add 442
#define sys_mbx421_acl_remove 443
#define sys_skip_list_display 444

int main(){
    long success = 0;
    success = syscall(sys_mbx421_init, 4, 2);
    printf("INIT");
    printf("%ld\n", success);

    success = syscall(sys_mbx421_create, 1);
    printf("INSERT 1 ");
    printf("%ld\n", success);
    //display();

    success = syscall(sys_mbx421_create, 20);
    printf("INSERT 20 ");
    printf("%ld\n", success);


    success = syscall(sys_mbx421_create, 10);
    printf("INSERT 10 ");
    printf("%ld\n", success);
    syscall(sys_skip_list_display);


    // SEND ++++++++++
    unsigned char message1[6] = "hello";
    success = syscall(sys_mbx421_send,10, message1, 6);
    printf("send ");
    printf("%ld\n", success);
    /*
    // SEND +++++++++++
    unsigned char message2[7] = "world!";
    success = syscall(sys_mbx421_send, 10, message2, 7);
    printf("send ");
    printf("%ld\n", success);
    syscall(sys_skip_list_display);
    */
    // RECV +++++++++++
    unsigned char newMsg1;
    success = syscall(sys_mbx421_recv, 10, &newMsg1, 6);
    printf("receive ");
    printf("%s\n", &newMsg1);
    printf("%ld\n", success);

    /*
    unsigned char newMsg2;
    success = syscall(sys_mbx421_recv, 10, &newMsg2, 7);
    printf("receive  ");
    printf("%s\n", &newMsg2);
    printf("%ld\n", success);
    */
    success = syscall(sys_mbx421_destroy, 10);
    printf("REMOVE 10 ");
    printf("%ld\n", success);
    syscall(sys_skip_list_display);

    syscall(sys_skip_list_display);

    syscall(sys_mbx421_shutdown);
    return 0;
}