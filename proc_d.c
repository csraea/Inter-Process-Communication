#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

#define RED arg
#define GREEN arg
#define S2_0 S2_id, 0
#define S2_1 S2_id, 1

static _Bool exitPermission = false;
static void sigHandler(int SIG) {
   exitPermission = true;
}

int main(int argc, char *argv[]) {
   signal(SIGUSR1, sigHandler);
   union semun {
      int val;
      struct semid_ds *buf;
      unsigned short *array;
   } arg;
   struct sockaddr_in serv_addr;
	// struct hostent *he;
   char *word;
   int S2_id = atoi(argv[1]);
   int SHM2_id = atoi(argv[2]);
   int portno = atoi(argv[3]);
	int i,sfd;

   word = (char*)shmat(SHM2_id, NULL, SHM_RND);

	printf("(D)proc_d ..... start  (%d)\n", semctl(S2_id, 1, GETVAL));

   if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	   printf("(D) Nemozem otvorit socket\n");
	   perror("socket");
	   exit(1);
	}
   
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
   inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
      
   if (connect(sfd,(struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) 
	   printf("(D)ERROR connecting\n");

   puts("(D)Reading...\n");
   while(!exitPermission) {
      printf("(D)Cakam na ZELENU\n");

      while(semctl(S2_id, 1, GETVAL) == 0) sleep(1);

      printf("(D)Dostal som ZELENU\n");
		if ((write(sfd,word,50)) < 0) 
			printf("ERROR writing to socket\n");

      printf("(D) SHM2 -> ... %s\n", word);
      *word = '\n';
      arg.val = 0;
      semctl(S2_id, 1, SETVAL, RED);
      arg.val = 1;
      semctl(S2_id, 0, SETVAL, GREEN);
   }
}
