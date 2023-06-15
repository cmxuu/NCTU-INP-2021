#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
using namespace std;

void str_cli(int sockfd);

int main(int argc, char* argv[]) {

	int sockfd;
	int addrPort = atoi(argv[2]);
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// Server
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(addrPort);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	str_cli(sockfd);
	close(sockfd);

	return 0;
}

void str_cli(int sockfd) {

	int maxfdpl;
	fd_set rset;
	char sendline[1024], recvline[1024];

	FD_ZERO(&rset);
	for (;;) {

		// Init Select
		FD_SET(fileno(stdin), &rset);
		FD_SET(sockfd, &rset);
		maxfdpl = max(fileno(stdin), sockfd) + 1;
		select(maxfdpl, &rset, NULL, NULL, NULL);

		// Receive Socket
		if (FD_ISSET(sockfd, &rset)) {
			memset(recvline, '\0', sizeof(recvline));
			read(sockfd, recvline, 1024);
			cout << recvline;
		}

		// Receive Stdin
		if (FD_ISSET(fileno(stdin), &rset)) {
			fgets(sendline, 1024, stdin);
			write(sockfd, sendline, sizeof(sendline));
			if (strcmp(sendline, "exit\n") == 0) {
				break;
			}
		}
	}
}