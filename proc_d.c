#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>

enum errors {
    SUCCESS,
    E_ARGS,
    E_SEMV,
    E_SHMAT,
    E_RDWR,
    E_SHMDT,
    E_SEMCTL
} errors;

int main(int argc, char **argv) {
    if(argc != 3)  {
        perror("Invalid arguments!");
        exit(E_ARGS);
    }

    int idSM2 = atoi(argv[1]);
    int idS2 = atoi(argv[2]);

    
}