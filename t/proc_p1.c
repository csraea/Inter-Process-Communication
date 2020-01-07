#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define INFILE_FILENAME "p1.txt"

enum errors {
    SUCCESS,
    E_ARGS,
    E_OPEN,
    E_READ,
    E_WRITE,
    E_CLOSE
} errors;

static _Bool readAccess = false;
static _Bool exitPermission = false;

//function to perform I/O operations and handle errors
size_t io_cntl(int fd1, int fd2) {
    char buf = '\0';
    ssize_t ret = 0;
    do {
        ret = read(fd1, &buf, 1);
        if(ret == -1) {
            if(errno == EINTR) continue;
            return E_READ;
        }
        ret = write(fd2, &buf, ret);
        if(ret == -1) {
            if(errno == EINTR) continue;
            return E_WRITE;
        }
    } while(buf != '\0' && buf != '\n' && buf != ' ' && buf != '\t' && ret != 0);
    return SUCCESS;
}

//signal handlers
static void sigHandler(int SIG) {
    readAccess = true;
}

static void sigExit(int SIG) {
    exitPermission = true;
}

int main(int argc, char **argv) {
    if(argc != 2 || atoi(argv[1]) == 0) {
        perror("Oops... Something went wrong with arguments!");
        exit(E_ARGS);
    }

    signal(SIGUSR1, sigHandler);
    signal(SIGUSR2, sigExit);
    
    int fd1 = open(INFILE_FILENAME, O_RDONLY);
    if(fd1 == -1) {
        return E_OPEN;
    } 

A:  if(!readAccess && !exitPermission){
        pause();
    } 
    if(exitPermission) goto EXIT;
    errno = 0;
    size_t ioResult = io_cntl(fd1, atoi(argv[1]));
    if(ioResult != SUCCESS) {
        perror("Oops... Something went wrong during the reading or writing!");
        printf("ERRNO %d\n", errno);
        exit(ioResult);
    }
    readAccess = false;
    if(!exitPermission) goto A;
EXIT:   if(close(fd1) == -1) {
            return E_CLOSE;
        }
    exit(EXIT_SUCCESS);
}

/*
    Contact autor:
    MAIL -          csraea@gmail.com
    TELEGAM -       t.me/csraea
    INSTAGRAM -     @korotetskiy_

    Korotetskiy©. All rights reserved. 2019.

*/