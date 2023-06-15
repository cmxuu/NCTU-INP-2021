#include <stdio.h>
#include <string>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
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

int readable(int fd){

	fd_set rset;
	struct timeval tv;

	/* Set Select */
	FD_ZERO(&rset);
	FD_SET(fd, &rset);
	tv.tv_sec = 0;
	tv.tv_usec = 5000;

	return (select(fd + 1, &rset, NULL, NULL, &tv));

}

int main(int argc, char **argv){

	int sockfd;
	ssize_t length;	
	struct sockaddr_in servaddr, cliaddr;
	struct stat st;
	struct timeval start, end;
	
	/* Init Server (Receiver) */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[3]));
	inet_pton(AF_INET, argv[2], &servaddr.sin_addr);

	/* Set Socket */ 
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	

	// --------------------------------------------------------------------------------- //

	struct Frame frame;
	FILE *fptr;
	off_t fsize = 0;	
	char *fname = argv[1];
	int totalFrame = 0,  ackNum = 0;

	fptr = fopen(fname, "rb");

	/* Set Frame Info. */
	stat(fname, &st);
	fsize = st.st_size;
	if(fsize % BUF_SIZE != 0)	totalFrame = (fsize / BUF_SIZE) + 1;
	else	totalFrame = (fsize / BUF_SIZE);
	printf("[S] Frame Num: %d\n", totalFrame);

	/* Send Frame Info. */
	sendto(sockfd, &(totalFrame), sizeof(totalFrame), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
	if(readable(sockfd) > 0){
		recvfrom(sockfd, &(ackNum), sizeof(ackNum), 0, (struct sockaddr *) &cliaddr, (socklen_t *) &length);
	}
	while (ackNum != totalFrame){
		sendto(sockfd, &(totalFrame), sizeof(totalFrame), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
		if(readable(sockfd) > 0)
			recvfrom(sockfd, &(ackNum), sizeof(ackNum), 0, (struct sockaddr *) &cliaddr, (socklen_t *) &length); 
	}

	/* Send File */
	gettimeofday(&start, NULL);
	printf("[S] Sending...\n");
	for(int i = 1; i <= totalFrame; i++){

		int resend = 0;

		// Send Frame
		memset(&frame, 0, sizeof(frame));
		frame.ID = i;
		frame.len = fread(frame.data, 1, BUF_SIZE, fptr);
		sendto(sockfd, &(frame), sizeof(frame), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));

		// Check Frame ACK
		if(readable(sockfd) > 0)
			recvfrom(sockfd, &(ackNum), sizeof(ackNum), 1, (struct sockaddr *) &cliaddr, (socklen_t *) &length);
        while (ackNum != frame.ID)
		{
			sendto(sockfd, &(frame), sizeof(frame), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
			if(readable(sockfd) > 0)
				recvfrom(sockfd, &(ackNum), sizeof(ackNum), 0, (struct sockaddr *) &cliaddr, (socklen_t *) &length);
			
			resend++;
			if(resend == 100)
				break;
		}

		// Send Finish
		if (totalFrame == ackNum)
			printf("[S] Finish!!!\n");

	}

	gettimeofday(&end, NULL);

	// --------------------------------------------------------------------------------- //

	int sec = end.tv_sec - start.tv_sec;
	int usec = end.tv_usec - start.tv_usec;
	printf("[S] Total Time: %.4f sec\n", (sec * 1000 + (usec / 1000.0)) / 1000.0);

	fclose(fptr);
	close(sockfd);
	return 0;

}