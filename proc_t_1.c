#include <sys/sem.h>
#include <sys/shm.h>
#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 1024
#define SHM_KEY 0x1234

struct shmseg {
    int cnt;
    int complete;
    char buf[BUF_SIZE];
};


int fill_buffer(char * bufptr, int q);

int main(int argc, char *argv[]) {
    if(argc != 3)
    {
        perror("Wrong amount of arguments");
        return -1;

    }
    int q = *argv[1];
    int semid = atoi(argv[2]);
//    printf("%d\n", semid);
    int availability = semctl(semid, 0, GETVAL);
    int shmid, numtimes;
    struct shmseg *shmp;
    char *bufptr;
//    shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0642|IPC_EXCL);
//    if (shmid == -1) {
//        return -1;
//    }
//    shmp = shmat(shmid, NULL, 0);
//    if (shmp == (void *) -1) {
//        return -1;
//    }
    if(availability == 1) {
        bufptr = shmp->buf;
            shmp->cnt = fill_buffer(bufptr, q);
            if(shmp->cnt == 0)
            {
                perror("0 bytes were read from pipe");
                return -1;
            }
            shmp->complete = 0;
            strcpy(shmp->buf,bufptr);
            printf("%s\n", shmp->buf);
        shmp->complete = 1;

        if (shmdt(shmp) == -1) {
            perror("Problem detaching memory");
            return -1;
        }
    }
    return 0;
}

int fill_buffer(char * bufptr, int q) {
    int filled_count = 0;
    bufptr[0] = '0';
    int i = -1;
    do
    {
        i++;
        read(q, &bufptr[i], 1);
    }
    while (bufptr[i] != '\0');
//    for (int i = 0; bufptr[i] != '\0'; i++) {
//            read(q, &bufptr[i], 1);
//    }
    filled_count = strlen(bufptr);
    return filled_count;
}