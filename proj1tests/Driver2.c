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
    pid_t  pid;
    pid = fork();
    if(pid < 0){
        printf("Fork Failed \n");
    }
    // child process
    else if(pid == 0){

    }
    // parent process
    else{

    }
    return 0;
}