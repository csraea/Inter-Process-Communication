#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif 

enum errors {
    SUCCESS,
    E_ARGS,
    E_SEMV,
    E_SHMAT,
    E_RDWR,
    E_SHMDT,
    E_SEMCTL
} errors;

struct shmseg {
    int cnt;    //Number of bytes used in 'buf'
    char buf[BUF_SIZE];
};

int main(int argc, char **argv) {
    if(argc != 4) {
        perror("Oops... Invalid arguments!");
        exit(E_ARGS);
    }

    int q = atoi(argv[1]);
    int idSM1 = atoi(argv[2]);
    int idS1 = atoi(argv[3]);

    char *shmp;

    if(semctl(idS1, 0, GETVAL) != 1) {
        perror("Oops... Semafore value is not propriate!");
        printf("%d\n", errno);
        exit(E_SEMV);
    }

    shmp = (char *) shmat(idSM1, NULL, 0);
    void *buf = shmp;
    if (shmp == (void *) -1) {
        perror("Oops... Shmat() went wrong!");
        exit(E_SHMAT);
    }

    int ret = 1, flag = 0;
    do {
        if(flag != 0) shmp++;
        ret = read(q, shmp, 1);
        if(ret == -1) {
            perror("Oops... Reading & writing went wrong!");
            exit(E_RDWR);
        }
        flag = 1;
    } while (*shmp != ' ' && *shmp != '\t' && *shmp != '\n' && *shmp != '\0' && ret != 0);
    *shmp = '\0';
    if(shmdt(buf) == -1) {
        perror("Oops... Detaching error!");
        exit(E_SHMDT);
    }
    
    int semValues[] = {0,1};
    if(semctl(idS1, 0, SETALL, semValues) == -1) {
        perror("Oops... Something went wrong during the semctl()!");
        exit(E_SEMCTL);
    }
    exit(SUCCESS);
}

/*
    while (semctl != 0) {
        read 1 word
        change semaphores
        block
        -- when unblocks by the s process, enters the loop again
    }

    */