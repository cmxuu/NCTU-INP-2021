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

struct Client {
	struct sockaddr_in address;
	int sockfd;
	string name;
};

Client client[10];
int useClient[10];

int main(int atgc, char* argv[]) {

	int servPort = atoi(argv[1]);
	int listenfd, connfd, sockfd;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t clilen;


	/* Init Server */

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(servPort);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	listen(listenfd, 11);


	/* Init Select */

	int i, nready, maxfd, maxi, n;
	fd_set rset, allset;
	char line[1024], temp[1024];
	string str;

	maxfd = listenfd;
	maxi = -1;
	for (i = 0; i < 10; i++)
		useClient[i] = -1;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);


	/* Loop */

	for (;;) {

		rset = allset;
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

		/* New Collection */
		if (FD_ISSET(listenfd, &rset)) {

			cout << "New Client" << endl;

			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);

			// useClient
			for (i = 0; i < 10; i++)
				if (useClient[i] < 0) {
					useClient[i] = 1;
					break;
				}

			// Client Info.
			client[i].address = cliaddr;
			client[i].sockfd = connfd;
			client[i].name = "anonymous";

			// Hello to New
			str = "[Server] Hello, anonymous! From: ";
			str.append(inet_ntoa(client[i].address.sin_addr));
			str += ":";
			sprintf(temp, "%d", ntohs(client[i].address.sin_port));
			str.append(temp);
			str += "\n";
			strcpy(line, str.c_str());
			write(client[i].sockfd, line, sizeof(line));

			// Hello to Old
			str = "[Server] Someone is coming!\n";
			strcpy(line, str.c_str());
			for (int j = 0; j < 10; j++)
				if (useClient[j] == 1 && j != i)
					write(client[j].sockfd, line, sizeof(line));

			// Check Select
			FD_SET(connfd, &allset);
			if (connfd > maxfd)
				maxfd = connfd;
			if (i > maxi)
				maxi = i;
			if (--nready <= 0)
				continue;
		}


		/* Check Client */
		for (int i = 0; i < 10; i++) {

			if (useClient[i] == -1)
				continue;
			sockfd = client[i].sockfd;

			if (FD_ISSET(sockfd, &rset)) {

				n = read(sockfd, line, 1024);
				char* key = strtok(line, " \n");

				// Exit
				if (strcmp(key, "exit") == 0) {

					cout << "Exit" << endl;

					str = "[Server] " + client[i].name + " is offline.\n";
					strcpy(line, str.c_str());
					for (int j = 0; j < 10; j++) {
						if (useClient[j] == 1 && j != i) {
							write(client[j].sockfd, line, sizeof(line));
						}
					}
					close(sockfd);
					FD_CLR(sockfd, &allset);
					useClient[i] = -1;
				}

				// Who
				else if (strcmp(key, "who") == 0) {

					cout << "Who" << endl;

					for (int j = 0; j < 10; j++) {
						if (useClient[j] == 1) {
							str = "[Server] " + client[j].name + " ";
							str.append(inet_ntoa(client[j].address.sin_addr));
							str += ":";
							sprintf(temp, "%d", ntohs(client[j].address.sin_port));
							str.append(temp);
							if (j == i)
								str += " ->me";
							str += "\n";
							strcpy(line, str.c_str());
							write(client[i].sockfd, line, sizeof(line));
						}
					}
				}

				// Name
				else if (strcmp(key, "name") == 0) {

					cout << "Name: ";

					int flag = -1;
					key = strtok(NULL, " \n");
					string newName(key);
					string oldName = client[i].name;

					cout << newName << endl;
					if (newName == "anonymous") {
						flag = 1;
						str = "[Server] ERROR: Username cannot be anonymous.\n";
						strcpy(line, str.c_str());
						write(client[i].sockfd, line, sizeof(line));
					}

					int unique = -1;
					for (int j = 0; j < 10; j++) {
						if (useClient[j] == 1 && j != i && client[j].name == newName) {
							unique = 1;
							break;
						}
					}
					if (unique == 1) {
						flag = 1;
						str = "[Server] ERROR: " + newName + " has been used by others.\n";
						strcpy(line, str.c_str());
						write(client[i].sockfd, line, sizeof(line));
					}

					if (newName.length() < 2 || newName.length() > 12) {
						flag = 1;
						str = "[Server] ERROR: Username can only consists of 2~12 English letters.\n";
						strcpy(line, str.c_str());
						write(client[i].sockfd, line, sizeof(line));
					}

					if (flag == -1) {

						client[i].name = newName;
						str = "[Server] You're now known as " + newName + ".\n";
						strcpy(line, str.c_str());
						write(client[i].sockfd, line, sizeof(line));

						for (int j = 0; j < 10; j++) {
							str = "[Server] " + oldName + " is now known as " + newName + ".\n";
							strcpy(line, str.c_str());
							if (useClient[j] == 1 && j != i)
								write(client[j].sockfd, line, sizeof(line));
						}

					}

				}

				// Yell
				else if (strcmp(key, "yell") == 0) {

					cout << "Yell" << endl;

					string msg;
					key = strtok(NULL, "\n");
					if (key != NULL)
						msg.append(key);

					str = "[Server] " + client[i].name + " yell " + msg + "\n";
					strcpy(line, str.c_str());
					for (int j = 0; j < 10; j++) {
						if (useClient[j] == 1)
							write(client[j].sockfd, line, sizeof(line));
					}
				}

				// Tell
				else if (strcmp(key, "tell") == 0) {

					cout << "Tell" << endl;

					int flag = -1;
					string toName, msg;
					key = strtok(NULL, " \n");
					if (key != NULL)
						toName.append(key);
					key = strtok(NULL, "\n");
					if (key != NULL)
						msg.append(key);

					if (client[i].name == "anonymous") {
						flag = 1;
						str = "[Server] ERROR: You are anonymous.\n";
						strcpy(line, str.c_str());
						write(client[i].sockfd, line, sizeof(line));
					}
					if (toName == "anonymous") {
						flag = 1;
						str = "[Server] ERROR: The client to which you sent is anonymous.\n";
						strcpy(line, str.c_str());
						write(client[i].sockfd, line, sizeof(line));
					}

					int exist = -1;
					int toSock;
					for (int j = 0; j < 10; j++) {
						if (toName == client[j].name) {
							exist = 1;
							toSock = client[j].sockfd;
							break;
						}
					}
					if (exist == -1) {
						flag = 1;
						str = "[Server] ERROR: The receiver doesn't exist.\n";
						strcpy(line, str.c_str());
						write(client[i].sockfd, line, sizeof(line));
					}

					if (flag == -1) {
						str = "[Server] SUCCESS: Your message has been sent.\n";
						strcpy(line, str.c_str());
						write(client[i].sockfd, line, sizeof(line));

						str = "[Server] " + client[i].name + " tell you " + msg + "\n";
						strcpy(line, str.c_str());
						write(toSock, line, sizeof(line));
					}



				}

				// Error
				else {
					str = "[Server] ERROR: Error command.\n";
					strcpy(line, str.c_str());
					write(client[i].sockfd, line, sizeof(line));
				}

				if (--nready <= 0)
					break;
			}
		}
	}
	return 0;
}