#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
// для семафоров и wait
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>

sem_t* paintings[5];

int main(int argc, char *argv[]) {
		sem_t* sem_watchman = sem_open("watchman", O_CREAT | O_EXCL, 0644, 50);

		paintings[0] = sem_open("painting0", O_CREAT | O_EXCL, 0644, 11);
		paintings[1] = sem_open("painting1", O_CREAT | O_EXCL, 0644, 11);
		paintings[2] = sem_open("painting2", O_CREAT | O_EXCL, 0644, 11);
		paintings[3] = sem_open("painting3", O_CREAT | O_EXCL, 0644, 11);
		paintings[4] = sem_open("painting4", O_CREAT | O_EXCL, 0644, 11);	
		sem_unlink("watchman"); // removes named semaphore
		sem_close(sem_watchman); // frees resources
		
		sem_unlink("painting0");
		sem_unlink("painting1");
		sem_unlink("painting2");
		sem_unlink("painting3");
		sem_unlink("painting4");
		
		sem_close(paintings[0]);
		sem_close(paintings[1]);
		sem_close(paintings[2]);
		sem_close(paintings[3]);
		sem_close(paintings[4]);
}

