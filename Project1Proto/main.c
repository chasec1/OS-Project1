//
// Created by chasec1 on 10/16/19.
//

#include "skip_list.h"
int main(){
    long success = 0;
    success = init(4,2);
    printf("%ld\n", success);
    success = insert(20);
    printf("%ld\n", success);
    success = insert(1);
    printf("%ld\n", success);
    success = insert(20);
    printf("%ld\n", success);
    success = search(10);
    printf("%ld\n", success);
    return 0;
}