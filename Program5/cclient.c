#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <stdlib.h>

#define h_addr  h_addr_list[0]  /* for backward compatibility */

void debug(char * message) {
    char printBuffer[1024];
    sprintf(printBuffer, "%s\n", message);
    write(1, &printBuffer, strlen(printBuffer));
}

int main(int argc, char *argv[])
{
    /*define a socket fd, port, and variable to see how many bytes were read*/
    int sockfd, port, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[200];
    char output_message[300];
    int pid;

    if(argc != 4) {
        debug("Not correct number of args.");
    }

    /*make the socket*/
    debug("Creating socket");
    port = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        debug("Error creating socket.");
    }

    debug("Socket created, getting host by name");
    server = gethostbyname(argv[1]);
    debug("Got host");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (uint32_t *)&(serv_addr.sin_addr.s_addr), server->h_length);
    serv_addr.sin_port = htons(port);

    debug("Connecting");
    /*connect to socket to send things*/
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        debug("Could not connect to server");
    } 
    debug("Connected");
    pid = fork();
    if(pid != 0){
        while(1){
            bzero(buffer,200);
            n = read(0, &buffer, 128);
            bzero(output_message, 300);
            sprintf(output_message, "<%s> %s", argv[3], buffer);

            n = write(sockfd, &output_message, strlen(output_message));
            if (n < 0) {
                debug("could not connect to server.");
            }
        }
    } else {
        while(1){
            /*read output from the std of command run from the socket*/
            bzero(buffer,200);
            n = read(sockfd,&buffer,128);
            if (n < 0) {
                debug("Could not read input from socket");
            }
            sprintf(buffer, "%s", buffer);
            write(1, &buffer, strlen(buffer));
        }
    }
    
    return 0;
}