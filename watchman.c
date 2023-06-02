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

// используется в listen(server_sock, N);
int N = 1000; // ограничивает число подключений к серверу, соответсвенно кол-во посетителей тоже
sem_t* paintings[5];

int main(int argc, char *argv[]) {
	char *ip = argv[1]; // ip сервера (например "127.0.0.1")
	int port = atoi(argv[2]); // номер порта (5567 например)
	int n = atoi(argv[3]); // кол-во посетителей
	
	// именованный семафор вахтера
	// изначальное значение == 50
	sem_t* sem_watchman = sem_open("watchman", O_CREAT | O_EXCL, 0644, 50);
	
	paintings[0] = sem_open("painting0", O_CREAT | O_EXCL, 0644, 11);
	paintings[1] = sem_open("painting1", O_CREAT | O_EXCL, 0644, 11);
	paintings[2] = sem_open("painting2", O_CREAT | O_EXCL, 0644, 11);
	paintings[3] = sem_open("painting3", O_CREAT | O_EXCL, 0644, 11);
	paintings[4] = sem_open("painting4", O_CREAT | O_EXCL, 0644, 11);	
	
	int server_sock; // для самого сервера
	int client_sock; // для подключения посетителей
	int view_sock; // для модуля отображения информации о выставке
	struct sockaddr_in server_addr, client_addr, view_addr;
	socklen_t addr_size;
	char buffer[1024];
	
	// 1
	server_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (server_sock < 0) {
		perror("socket error: ");
		exit(1);
	}
	printf("server ip: %s\n", ip);
	printf("UDP server socket created\n");
	
	// 2
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);
	// ИМЕННО СЕРВЕРУ НУЖНО БИНДИТЬСЯ К ПОРТУ
	int bind_check = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (bind_check < 0) {
		perror("bind error: ");
		exit(1);
	}
	printf("bind to the port number: %d\n", port);
	
	// 4
	pid_t childpid;
	for (int i = 1; i <= n; ++i) {
		for (int j = 1; i <= 7; ++i)
		if((childpid = fork()) == 0) {
			// получаем 1 запрос
			bzero(buffer, sizeof(buffer));
			if (recvfrom(server_sock, buffer, 1024, 0, (struct sockaddr *)&client_addr, (socklen_t * restrict)sizeof(struct sockaddr)) < 0) {
				perror("recv error: ");
				exit(1);
			}
			if (strcmp(buffer, "let me in") == 0) { // это запрос на вход
				sem_wait(sem_watchman); // пытается войти (стоит в очереди)
				
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) != strlen(buffer)) { // даем разрешение (зеркалим запрос)
					perror("send error: ");
					exit(1);
				}
				
				printf("посетитель %d вошел\n", i);
				bzero(buffer, 1024);
				snprintf (buffer, 1024, "посетитель %d вошел\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
				srand(getpid());
			} if (strcmp(buffer, "0") == 0) {
				sem_wait(paintings[0]);
				printf("посетитель %d смотрит картину 0\n", i);
				bzero(buffer, 1024);
				snprintf (buffer, 1024, "посетитель %d смотрит картину 0\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
				sleep(rand() % 5);
				printf("посетитель %d заканчивает смотреть картину 0\n", i);
				snprintf (buffer, 1024, "посетитель %d заканчивает смотреть картину 0\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
				sem_post(paintings[0]);
				// он посмотрел картину (зеркалим запрос)
				bzero(buffer, 1024);
				strcpy(buffer, "0");
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
			} else if (strcmp(buffer, "1") == 0) {
				sem_wait(paintings[1]);
				printf("посетитель %d смотрит картину 1\n", i);
				snprintf (buffer, 1024, "посетитель %d смотрит картину 1\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
				sleep(rand() % 5);
				printf("посетитель %d заканчивает смотреть картину 1\n", i);
				snprintf (buffer, 1024, "посетитель %d заканчивает смотреть картину 1\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
				sem_post(paintings[1]);
				// он посмотрел картину (зеркалим запрос)
				bzero(buffer, 1024);
				strcpy(buffer, "1");
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
			} else if (strcmp(buffer, "2") == 0) {
				sem_wait(paintings[2]);
				printf("посетитель %d смотрит картину 2\n", i);
				snprintf (buffer, 1024, "посетитель %d смотрит картину 2\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
				sleep(rand() % 5);
				printf("посетитель %d заканчивает смотреть картину 2\n", i);
				snprintf (buffer, 1024, "посетитель %d заканчивает смотреть картину 2\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
				sem_post(paintings[2]);
				// он посмотрел картину (зеркалим запрос)
				bzero(buffer, 1024);
				strcpy(buffer, "2");
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
			} else if (strcmp(buffer, "3") == 0) {
				sem_wait(paintings[3]);
				printf("посетитель %d смотрит картину 3\n", i);
				snprintf (buffer, 1024, "посетитель %d смотрит картину 3\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
				sleep(rand() % 5);
				printf("посетитель %d заканчивает смотреть картину 3\n", i);
				snprintf (buffer, 1024, "посетитель %d заканчивает смотреть картину 3\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
				sem_post(paintings[3]);
				// он посмотрел картину (зеркалим запрос)
				bzero(buffer, 1024);
				strcpy(buffer, "3");
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
			} else if (strcmp(buffer, "4") == 0) {
				sem_wait(paintings[4]);
				printf("посетитель %d смотрит картину 4\n", i);
				snprintf (buffer, 1024, "посетитель %d смотрит картину 4\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
				sleep(rand() % 5);
				printf("посетитель %d заканчивает смотреть картину 4\n", i);
				snprintf (buffer, 1024, "посетитель %d заканчивает смотреть картину 4\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
				sem_post(paintings[4]);
				// он посмотрел картину (зеркалим запрос)
				bzero(buffer, 1024);
				strcpy(buffer, "4");
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
			} else if (strcmp(buffer, "exit") == 0) {
				sem_post(sem_watchman); // вышел из галереи
				printf("посетитель %d вышел\n", i);
				snprintf (buffer, 1024, "посетитель %d вышел\n", i);
				if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
					perror("send error: ");
					exit(1);
				}
			} else { // посетитель просит что-то странное а не картину
				printf("%s\n", buffer);
				perror("didn't receive a request to watch painting");
				exit(1);
			}
			exit(0);
		}
	}
	
	// 5
	if (childpid > 0) {
		// parent process ждет, когда все дети закончат свою работу
		while (childpid = waitpid(-1, NULL, 0)) {
			if (errno == ECHILD) {
				break;
			}
		}
		// sem_post(sem_watchman); // повышаем до 51;
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
		
		// говорим view model закругляться
		bzero(buffer, 1024);
		strcpy(buffer, "end");
		if (sendto(server_sock, buffer, 1024, 0, (struct sockaddr *)&view_addr, sizeof(struct sockaddr)) != strlen(buffer)) {
			perror("send error: ");
			exit(1);
		}
		
		exit(0);
	}
	// ВОЗМОЖНО ПЕРЕД if, ВОЗМОЖНО В НЕМ ПЕРЕД EXIT(0), но оно работает и ладно
	close(client_sock);
	return 0;
}
