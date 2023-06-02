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

int sock;
struct sockaddr_in addr;
socklen_t addr_size;
char buffer[1024];
	
// посетитель смотрит картины
void watch_paintings() {
	srand(getpid());
	int watched[5] = {0, 0, 0, 0, 0};
	const char *digits[5];
	digits[0] = "0";
	digits[1] = "1";
	digits[2] = "2";
	digits[3] = "3";
	digits[4] = "4";
	// нужно посмотреть ВСЕ 5 картин
	// посетитель смотрит только 1 раз на 1 картину
	// когда посмотрел на все - выходит
	for (int i = 0; i < 5; ++i) {
		if (watched[0] == 1 &&
			watched[1] == 1 &&
			watched[2] == 1 &&
			watched[3] == 1 &&
			watched[4] == 1) {
			break; // посмотрели все картины	
		}
		int j = rand() % 5;
		while (watched[j]) { // нужно смотреть на НОВУЮ (для посетителя) картину
			j = rand() % 5;
		}
		watched[j] = 1;
		
		
		// Запрашиваем просмотр картины
		bzero(buffer, 1024);
		strcpy(buffer, digits[j]);
		if (send(sock, buffer, strlen(buffer), 0) != strlen(buffer)) {
			perror("send() failed");
			exit(1);
		}
		printf("sent: %s digit: %s\n", buffer, digits[j]);
		
		// Получаем результат (зеркальный ответ, картину значит посмотрели)
		bzero(buffer, 1024);
		if (recv(sock, buffer, sizeof(buffer), 0) < 0) {
			perror("recv() failed");
			exit(1);
		}
		printf("received: %s\n", buffer);
		if (strcmp(buffer, digits[j])) { // не получили зеркальный ответ
			printf("%s\n", buffer);
			perror("something went wrong while watching paintings");
			exit(1);
		}
	}
}


int main(int argc, char *argv[]) {
	char *ip = argv[1]; // ip сервера (например "127.0.0.1")
	int port = atoi(argv[2]); // номер порта (5567 например)
	
	// 1
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket error: ");
		exit(1);
	}
	printf("UDP server socket created\n");

	// 2
	memset(&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	// КЛИЕНТ КОННЕКТИТСЯ
	connect(sock, (struct sockaddr*)&addr, sizeof(addr));
	printf("client connected to the server\n");
	
	// 3
	////////////////////////////////////////////////////////////////////////
	// Запрашиваем вход
	bzero(buffer, 1024);
	strcpy(buffer, "let me in");
	if (sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr)) != strlen(buffer)) {
		perror("send() failed");
		exit(1);
	}
	printf("sent: %s\n", buffer);
	
	// Получаем разрешение (зеркальный ответ)
	bzero(buffer, 1024);
	if (recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, (socklen_t * restrict)sizeof(struct sockaddr)) < 0) {
		perror("recv() failed");
		exit(1);
	}
	printf("received: '%s'\n", buffer);
	if (strcmp(buffer, "let me in") == 0) { // получили разрешение
		watch_paintings();
		// Запрашиваем выход
		bzero(buffer, 1024);
		strcpy(buffer, "exit");
		if (sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr)) != strlen(buffer)) {
			perror("send() failed");
			exit(1);
		}
	} else { // не получили разрешение
		perror("couldn't get in");
		exit(1);
	}
	////////////////////////////////////////////////////////////////////////
	close(sock);
	exit(0);
}

