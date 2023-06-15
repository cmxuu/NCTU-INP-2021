#include <stdio.h>
#include <string>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_SIZE 1000

struct Frame{
	int ID;
	int len;
	char data[BUF_SIZE];
};

int main(int argc, char **argv){

	int sockfd;
	ssize_t length;	
	struct sockaddr_in servaddr, cliaddr;
	struct stat st;
	struct timeval tv;

	/* Init Server (Receiver) */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	servaddr.sin_addr.s_addr =  inet_addr("127.0.0.1");
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	// --------------------------------------------------------------------------------- //

	struct Frame frame;
	FILE *fptr;
	off_t fsize = 0;	
	char *fname = argv[1];
	int totalFrame = 0,  ackNum = 0, recvSize = 0;

	/* Get Frame Info. */
	recvfrom(sockfd, &(totalFrame), sizeof(totalFrame), 0, (struct sockaddr *) &cliaddr, (socklen_t *) &length);
	sendto(sockfd, &(totalFrame), sizeof(totalFrame), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
	printf("[R] Total Frame: %d\n", totalFrame);

	/* Receive File */
	printf("[R] Receiving...\n");
	fptr = fopen(fname, "wb");
	for (int i = 1; i <= totalFrame; i++){

		// Receive Frame
		memset(&frame, 0, sizeof(frame));
		recvfrom(sockfd, &(frame), sizeof(frame), 0, (struct sockaddr *) &cliaddr, (socklen_t *) &length);
		sendto(sockfd, &(frame.ID), sizeof(frame.ID), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));

		// Get Correct Frame
		if( frame.ID == i){
			fwrite(frame.data, 1, frame.len, fptr);
		}
		else
			i--;

		// Receive Finish
		if(i == totalFrame){
			printf("[R] Finish!!!\n");
		}
	}

	// --------------------------------------------------------------------------------- //

	fclose(fptr);
	close(sockfd);
	return 0;

}