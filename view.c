#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

int main(int argc, char *argv[]) {
	char *ip = argv[1]; // ip сервера (например "127.0.0.1")
	int port = atoi(argv[2]); // номер порта (5567 например)
	
	int sock;
	struct sockaddr_in addr;
	socklen_t addr_size;
	char buffer[1024];
	
	// 1
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket error: ");
		exit(1);
	}
	// printf("UDP server socket created\n");

	// 2
	memset(&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	// КЛИЕНТ КОННЕКТИТСЯ
	connect(sock, (struct sockaddr*)&addr, sizeof(addr));
	// printf("client connected to the server\n");
	
	// 3
	////////////////////////////////////////////////////////////////////////
	printf("выставка открывается\n");
	while (1) {
		// получаем строку
		bzero(buffer, 1024);
		if (recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, (socklen_t * restrict)sizeof(struct sockaddr)) < 0) {
			perror("recv() failed");
			exit(1);
		}
		// выводим ее (если это конец то выходим):
		if (strcmp(buffer, "end") == 0) { // получили разрешение
			printf("выставка закрывается\n");
			break;
		}
		printf("%s", buffer);
	}
	////////////////////////////////////////////////////////////////////////
	close(sock);
	exit(0);
}
