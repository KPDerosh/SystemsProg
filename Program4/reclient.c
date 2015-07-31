#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>


int main(int argc, char *argv[])
{
    int sockfd, port, n, index = 0;
    char command[1024];
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    char print[4000];

    /*make the socket*/
    port = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        sprintf(print, "%s\n", "Error socket not opened.");
        write(1, &print, strlen(print));
    }
       
    server = gethostbyname(argv[1]);
   
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(port);
    
    /*connect to socket to send things*/
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        sprintf(print, "%s\n", "Could not connect to server");
        write(1, &print, strlen(print));
    } 
        
    /*create string for command*/
    for(index = 3; index < argc; index++){
        strcat(command, argv[index]);
        strcat(command, ",");
    }

    /*send csv to server*/
    command[strlen(command) -1] = '\0';
    strcat(buffer, command);
    bzero(buffer,256);
    n = write(sockfd,command,strlen(command));
    if (n < 0) {
        sprintf(print, "%s\n", "could not write to server");
        write(1, &print, strlen(print));
    }
    
    /*read output from the std of command run from the socket*/
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) {
        sprintf(print, "%s\n", "Could not read input from socket");
    }
    sprintf(print, "%s\n", buffer);
    write(1, &print, strlen(print));
    return 0;
}