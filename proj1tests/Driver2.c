//
// Created by chasec1 on 11/1/19.
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
    syscall(sys_mbx421_init, 4, 2);
    pid_t  pid;
    pid = fork();
    if(pid < 0){
        printf("Fork Failed \n");
    }
    // child process
    else if(pid == 0){
        for(int i = 0; i < 30; i++) {
           syscall(sys_mbx421_create, i);
        }

        // adds a hi message to every other mailbox
        for(int i = 0; i < 60; i+=2){
            unsigned char msg[3] = "hi";
            syscall(sys_mbx421_create, i, msg, 3);
        }

        //attempts to receive from every 6th mailbox
        for(int i = 0; i < 60; i += 6){
            unsigned char message;
            syscall(sys_mbx421_recv, i, &message, 3);
        }

        // deletes every 5th mailbox
        for(int i = 0; i < 50; i+=2){
            syscall(sys_mbx421_destroy, i);

        }

        // tries to get length and count while parent deletes
        for(int i = 0; i < 60; i += 4){
            syscall(sys_mbx421_count, i);
            syscall(sys_mbx421_length, i);
        }

    }
    // parent process
    else{
        for(int i = 0; i < 60; i++) {
            syscall(sys_mbx421_create, i);
        }
        // adds a howdy to every third mailbox
        for(int i = 0; i < 60; i+=3){
            unsigned char msg[6] = "howdy";
            syscall(sys_mbx421_send, i, msg, 6);
        }

        // attempts to receive from every 10th mailbox
        for(int i = 0; i < 60; i += 10){
            unsigned char message;
            syscall(sys_mbx421_recv, i, &message, 6);
        }

        // destroys every 4th node while child attempts to read them
        for(int i = 0; i < 60; i += 4){
            syscall(sys_mbx421_destroy, i);
        }

    }

    syscall(sys_skip_list_display);

    return 0;
}