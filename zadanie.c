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

#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)   //Permissions for IPC objects
#define WRITE_SEM 0
#define READ_SEM 1 

#define CONV_BUF_SIZE 32

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif 

enum errors {
    SUCCESS,
    E_ARGS,
    E_PIPE,
    E_FORK,
    E_EXEC,
    E_FTOK,
    E_IPCC,
    E_SEMCTL
} errors;

struct shmseg {
    int cnt;    //Number of bytes used in 'buf'
    char buf[BUF_SIZE];
};

_Bool semChange = false;

static void sigHandler(int SIG) {
    semChange = true;
}

pid_t lastPid = 0, childPid = 0;
int status = 0;

int isemget(key_t a, size_t b, int c){ return semget(a, (int)b, c); }
int ipc_create(char proj_id, int size, int (*create)(key_t, size_t, int)){
    key_t key = ftok("/tmp", proj_id);
    if(key == -1) {
        perror("Oops... Something went wrong during ftok()!");
        exit(E_FTOK);
    }
    int id = create(key, size, IPC_CREAT /*| IPC_EXCL*/ | OBJ_PERMS);
    if(id == -1) {
        perror("Oops... Somehing went wrong while creating a new IPC object!");
        exit(E_IPCC);
    }
    return id;
}

//  1 -- argc
//  2 -- progPath
//  3 -- waitStatus
short execution(int argc, ...) {
    va_list valist;
    va_start(valist, argc);

    char *argsD[argc-1];                    // Dynamically allocated array of pointers to static strings
    char argsS[argc][CONV_BUF_SIZE];        // Statically allocated array of strings 
    memset(argsS, 0, sizeof(argsS));
    memset(argsD, 0, sizeof(argsD));
    argsD[argc-2] = (char *)NULL;

    strcpy(argsS[0], va_arg(valist, char*));
    argsD[0] = argsS[0];

    size_t waitStatus = va_arg(valist, size_t);
    for(size_t i = 2; i < argc - 1; i++) {
        sprintf(argsS[i], "%d", (int) va_arg(valist, int));
        argsD[i-1] = argsS[i];
    }

    int result = SUCCESS;
    lastPid = fork();
    switch (lastPid) {
        case -1:
            perror("Oops... Something went wrong while fork()!");
            exit(E_FORK);
        case 0:
            result = execv(argsD[0], argsD);
            if(result != SUCCESS) {
                exit(E_EXEC);
            }
            break;
        default :
            if(waitStatus != 0) {                       // If has to wait for the death of current branch
                while(wait(&status) != lastPid) ;
            }
            va_end(valist);
    }
    return (status == 0 ) ? SUCCESS : status;
    
}

int main(int argc, char **argv) {
    if(argc != 3 || atoi(argv[1]) <= 0 || atoi(argv[2]) <= 0) {
        perror("Oops... Program was provided with invalid arguments!");
        exit(E_ARGS);
    }

    signal(SIGUSR1, sigHandler);

    printf("###Main %d\n", (int)getpid());
    int port1 = atoi(argv[1]);
    int port2 = atoi(argv[2]);

    //create new pipes
    int p[2], q[2];
    if(pipe(p) == -1 || pipe(q) == -1){
        perror("Oops... Something went wrong during the pipes creating!");
        exit(E_PIPE);
    }

    //execute p* processes
    if(execution(4,"./proc_p1", 0, p[1]) != SUCCESS ){
        perror("Oops... Something went wrong during the P1 execution!");
        exit(E_EXEC);
    }
    pid_t pidP1 = lastPid;
    printf("###P1 %d\n", (int) pidP1);
    if(execution(4,"./proc_p2", 0, p[1]) != SUCCESS) {
        perror("Oops... Something went wrong during the P2 execution!");
        exit(E_EXEC);
    }
    pid_t pidP2 = lastPid; 
    printf("###P2 %d\n", (int) pidP2);
    //execute pr
    if(execution(7, "./proc_pr", 1, pidP1, pidP2, p[0], q[1]) != SUCCESS) {
        perror("Oops... Something went wrong during the PR execution!");
        exit(E_EXEC);
    }
    pid_t pidPR = lastPid;
    printf("###PR %d\n", (int) pidPR);
    //execution(4, "./test", 1, 543);
    
    kill(pidP1, SIGUSR2);
    kill(pidP2, SIGUSR2);

    int idS1 = ipc_create('m', 2, isemget);
    int idS2 = ipc_create('n', 2, isemget);
    int idSM1 = ipc_create('j', BUF_SIZE, shmget);
    int idSM2 = ipc_create('k', BUF_SIZE, shmget);

    int semValues[] = {1,0};
    if(semctl(idS1, 0, SETALL, semValues) == -1) {
        perror("Oops... Something went wrong during the semctl()!");
        exit(E_SEMCTL);
    }

    if(semctl(idS2, 0, SETALL, semValues) == -1) {
        perror("Oops... Something went wrong during the semctl()!");
        exit(E_SEMCTL);
    }

    if(execution(7, "./proc_s", 0, idSM1, idS1, idSM2, idS2) != SUCCESS) {
        perror("Oops... Something went wrong during the S execution!");
        exit(E_EXEC);
    }
    pid_t pidS = lastPid;
    printf("###S %d\n", (int) pidS);
    if(semChange != true) {
        pause();
    } 
    
    if(execution(6, "./proc_t", 1, q[0], idSM1, idS1) != SUCCESS) {
        perror("Oops... Something went wrong during the T execution!");
        exit(E_EXEC);
    }
    pid_t pidT = lastPid;
    printf("###T %d\n", (int) pidT);

    // if(execution(5, "./proc_d", 1, idSM2, idS2) != SUCCESS) {
    //     perror("Oops... D execution error!");
    //     exit(E_EXEC);
    // }


    // if(semctl(idS2, 0, GETVAL) != 0) {
    //     pause();
    // }
    
    // char buf[BUF_SIZE];
    // int r;
    // for(size_t i = 0; i < 2; i++){
    //     r = read(q[0], &buf, BUF_SIZE);
    //     if(r <= 0) break;
    //     write(1, &buf, r);
    //     printf("%d\n", r);
    // }

    // close(p[0]);
    // close(p[1]);
    // close(q[0]);
    // close(q[1]);

    char *str = (char *) shmat(idSM1, NULL, 0);
    puts(str);
    str = (char *) shmat(idSM2, NULL, 0);
    puts(str);


    shmctl(idSM2, IPC_RMID, NULL);
    shmctl(idSM1, IPC_RMID, NULL);
    semctl(idS1, 0, IPC_RMID);
    semctl(idS2, 0, IPC_RMID);




}


// short t_execution(int q, int idSM1, char *progPath) {
//     char *qbuf, *idSM1buf;
//     sprintf(qbuf, "%d", q);
//     sprintf(idSM1buf, "%d", idSM1);
//     short result;
//     pid_t forkResult = fork();
//     switch(forkResult) {
//         case -1:
//             perror("Oops... Something went wrong while fork()!");
//             exit(E_FORK);
//         case 0:
//             result = execl(progPath, progPath, qbuf, idSM1buf, (char *)NULL);
//             break;
//         default:
//             wait(&forkResult);   //wait untill t ends
//             result = SUCCESS;
//     }
//     return result;    
// }

// short s_execution(int idSM1, int idS1, int idSM2, int idS2, char *progPath) {
//     char *idSM1buf, *idSM2buf, *idS1buf, *idS2buf;
//     sprintf(idS1buf, "%d", idS1);
//     sprintf(idS2buf, "%d", idS2);
//     sprintf(idSM1buf, "%d", idSM1);
//     sprintf(idSM2buf, "%d", idSM2);
//     short result;
//     pid_t forkResult = fork();
//     switch(forkResult) {
//         case -1:
//             perror("Oops... Something went wrong while fork()!");
//             exit(E_FORK);
//         case 0:
//             result = execl(progPath, idSM1buf, idS1buf, idSM2buf, idS2buf, (char *)NULL);
//             break;
//         default:
//             result = SUCCESS;
//     }
//     return result;    
// }