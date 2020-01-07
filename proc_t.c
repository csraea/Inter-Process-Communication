#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

#define RED arg
#define GREEN arg
#define S1_0 S1_id, 0
#define S1_1 S1_id, 1

static _Bool exitPermission = false;
static void sigHandler(int SIG) {
   exitPermission = true;
}

void readWord(int idP, int idSM) {
   int i = -1, ret = -1;
   char *word = (char*)shmat(idSM, NULL, SHM_RND);
   do {
      i++;
      ret = read(idP, &word[i], 1);
   } while(ret > 0 && word[i] != '\n' && word[i] != '\0' && word[i] != '\n' && word[i] != ' ' && word[i] != '\t');

   word[i] = '\0';
   printf("(T)R2 -> SHM1 %s \n", word);
}


int main(int argc, char *argv[]) {
   signal(SIGUSR1, sigHandler);
   union semun {
      int val;
      struct semid_ds *buf;
      unsigned short *array;
   } arg;

   int R2_id = atoi(argv[1]);
   int SHM1_id = atoi(argv[2]);
   int S1_id = atoi(argv[3]);

   //fcntl(R2_id, F_SETFL, O_NONBLOCK);

   printf("(T) proc_t ..... start  (%d)\n", semctl(S1_id, 0, GETVAL));
   sleep(1);

   while(!exitPermission){
      while(0 == (semctl(S1_0, GETVAL))) sleep(1);
      arg.val = 0;
      readWord(R2_id, SHM1_id);
      semctl(S1_0, SETVAL, RED);
      arg.val = 1;
      semctl(S1_1, SETVAL, GREEN);
      sleep(3);
   }
   printf("Exiting T...\n");
}