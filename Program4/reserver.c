/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    /*Start the socket descriptors.*/
    int sockfd, newsockfd, clilen, n, length;
    int pid;
    char buffer[1024];
    char printBuffer[1024];
    struct sockaddr_in serv_addr, cli_addr;
    char *token;
    int index = 0, counter = 0, stringCounter = 0;
    char *arg[100];
    const char s[2] = ",";

    sockfd = socket(AF_INET, SOCK_STREAM, 0);   /*open socket*/

    /*set the server address and ports*/
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(0);

    /*bind the socket to an address.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        write(1, "Binding failed.", 15);
    } else {
        length = sizeof(serv_addr);
        getsockname(sockfd, (struct sockaddr *)&serv_addr, &length);
        sprintf(printBuffer, "Assigned port number %d\n", ntohs(serv_addr.sin_port));
        write(1, &printBuffer, strlen(printBuffer));
    }
    
    listen(sockfd,5);
    while(1){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            sprintf(printBuffer, "%s", "Error accepting connection");
            write(1, &printBuffer, strlen(printBuffer));
        }

        bzero(buffer,1024);
        n = read(newsockfd,buffer,1024);
        if (n < 0){
            sprintf(printBuffer, "%s", "Error reading from socket");
            write(1, &printBuffer, strlen(printBuffer));
        }

        sprintf(printBuffer, "%s %s", "Received command", buffer);
        write(1, &printBuffer, strlen(printBuffer));

        token = strtok(buffer, s);
        while(token != NULL){
            arg[counter] = token;
            counter++;
            token = strtok(NULL,s);
        }
        arg[counter + 1] = NULL;
        /*execute  the command*/
        /*create child process*/
        pid = fork(); 
        if(pid == -1){ 

        }
        if(pid != 0){   
            /*parent*/
            int returnStatus;
            /*wait for the process to execute all things.*/
            waitpid(pid, &returnStatus, 0);

        } else if(pid == 0){/*child*/
            /*redirect stdout to go to the socket*/
            dup2(newsockfd, 1);
            close(newsockfd);
            execvp(arg[0], arg);   /*run player process*/
        }

        if (n < 0){
            sprintf(printBuffer, "%s", "Error reading from socket");
            write(1, &printBuffer, strlen(printBuffer));
        }
    }
    
    return 0; 
}