#include   <sys/types.h>   /* basic system data types */
#include   <sys/socket.h>  /* basic socket definitions */
#include   <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include   <arpa/inet.h>   /* inet(3) functions */
#include   <errno.h>
#include   <stdio.h>
#include   <stdlib.h>
#include   <unistd.h>
#include   <string.h>
#include   <fcntl.h>

#define MAXLINE 1024
#define SA      struct sockaddr

int
main(int argc, char **argv)
{
	int	sockfd, n, err, connfd;
	struct sockaddr_in	servaddr;
	char recvline[MAXLINE + 1], buff[MAXLINE];

	// init buffers to '\0'
    buff[0] = '\0';
	recvline[0] = '\0';

	if (argc != 2){
		fprintf(stderr, "usage: %s <IPaddress> \n", argv[0]);
		return 1;
	}
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr,"socket error : %s\n", strerror(errno));
		return 1;
	}

//sleep(3);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(13);	/* daytime server */
	if ( (err=inet_pton(AF_INET, argv[1], &servaddr.sin_addr)) == -1){
		fprintf(stderr,"ERROR: inet_pton error for %s : %s \n", argv[1], strerror(errno));
		return 1;
	}else if(err == 0){
		fprintf(stderr,"ERROR: Invalid address family \n");
		return 1;
	}
	
	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0){
		fprintf(stderr,"connect error : %s \n", strerror(errno));
		return 1;
	}
	printf("Connected\n");

    while (1){
		// set socket to non-blocking
		fcntl(sockfd, F_SETFL, O_NONBLOCK);

		// set stdin to non-blocking
		fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

		// READING
        // read buffer
        while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
            recvline[n] = 0;	/* null terminate */
            if (fputs(recvline, stdout) == EOF){
                fprintf(stderr,"fputs error : %s\n", strerror(errno));
                return 1;
            }
			fflush(stdout);
			fgetc(stdin);	


        }
		// if (n < 0)
        //     fprintf(stderr,"read error : %s\n", strerror(errno));


        // SENDING        

        sleep(2);
	
        int nread;
        if ( (nread = read(0, buff, MAXLINE)) > 0) {
            buff[nread] = '\0';
        }
        fgetc(stdin);

		if( send(sockfd, buff, strlen(buff), 0  )< 0 ) { 
			fprintf(stderr,"write error : %s\n", strerror(errno));
			close(sockfd);  
		}
		buff[0] = '\0';
        

    }
	fprintf(stderr,"OK\n");
	fflush(stdout);
	fgetc(stdin);	
//sleep(10);
	exit(0);
}
