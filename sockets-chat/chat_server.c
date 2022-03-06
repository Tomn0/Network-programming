#include        <sys/types.h>   /* basic system data types */
#include        <sys/socket.h>  /* basic socket definitions */
#include 		<unistd.h>
#if TIME_WITH_SYS_TIME
#include        <sys/time.h>    /* timeval{} for select() */
#include        <time.h>                /* timespec{} for pselect() */
#else
#if HAVE_SYS_TIME_H
#include        <sys/time.h>    /* includes <time.h> unsafely */
#else
#include        <time.h>                /* old system? */
#endif
#endif
#include        <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include        <arpa/inet.h>   /* inet(3) functions */
#include        <errno.h>
#include        <fcntl.h>               /* for nonblocking */
#include        <netdb.h>
#include        <signal.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include		<unistd.h>
#include        <string.h>
#define MAXLINE 1024

//#define SA struct sockaddr

#define LISTENQ 2

int
main(int argc, char **argv)
{
    int				listenfd, connfd;
    socklen_t			len;
    char				buff[MAXLINE], str[INET_ADDRSTRLEN+1], message[MAXLINE];
    time_t				ticks;
    struct sockaddr_in	servaddr, cliaddr;
    
    // init buffers to '\0'
    buff[0] = '\0';
    message[0] = '\0';

    if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr,"socket error : %s\n", strerror(errno));
        return 1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr   = htonl(INADDR_ANY);
    servaddr.sin_port   = htons(13);	/* daytime server */
    
    if ( bind( listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
        fprintf(stderr,"bind error : %s\n", strerror(errno));
        return 1;
    }

    if ( listen(listenfd, LISTENQ) < 0){
        fprintf(stderr,"listen error : %s\n", strerror(errno));
        return 1;
    }

    // wait for connections    
    len = sizeof(cliaddr);
    if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len)) < 0){
        fprintf(stderr,"accept error : %s\n", strerror(errno));
    }
    bzero(str, sizeof(str));
    
    // inet_ntop - convert IPv4 and IPv6 addresses from binary to text form
    inet_ntop(AF_INET, (struct sockaddr  *) &cliaddr.sin_addr,  str, sizeof(str));
    printf("Connection from %s\n", str);

    // set socket to non-blocking
    fcntl(connfd, F_SETFL, O_NONBLOCK);

    // set stdin to non-blocking
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

    for ( ; ; ) {
        // printf("Enter loop\n");

        // SENDING        
        // wait before read
        sleep(2);

        int nread;
        if ( (nread = read(0, buff, MAXLINE)) > 0) {
            buff[nread] = '\0';
        }
        fgetc(stdin);


        if (strncmp(buff, ":end", 4) == 0 ) {       
            printf("Ending connection\n");
            close(connfd);
            // sleep(10);
        }
        else {
            // MSG_NOSIGNAL flag - don't generate SIGPIPE signal if the peer socket closed
            if( send(connfd, buff, strlen(buff), MSG_NOSIGNAL  )< 0 ) { 
                fprintf(stderr,"write error : %s\n", strerror(errno));
                close(connfd);  

                // new connection
                printf("Waiting for new connection\n");
                if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len)) < 0){
                    fprintf(stderr,"accept error : %s\n", strerror(errno));
                }
                bzero(str, sizeof(str));
    
                // inet_ntop - convert IPv4 and IPv6 addresses from binary to text form
                inet_ntop(AF_INET, (struct sockaddr  *) &cliaddr.sin_addr,  str, sizeof(str));
                printf("Connection from %s\n", str);
            }
            buff[0] = '\0';
        }
        // printf("Reading\n");

        // READING
        // read buffer
        int n;
        while ( (n = recv(connfd, message, MAXLINE, 0)) > 0) {
            message[n] = 0;	/* null terminate */
            if (fputs(message, stdout) == EOF){
                fprintf(stderr,"fputs error : %s\n", strerror(errno));
                return 1;
            }

        }
        // if ( n == -1) {
        //     fprintf(stderr,"read error : %s\n", strerror(errno));
        // }
    }
}
