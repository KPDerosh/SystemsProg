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
    int index = 0, counter = 0;
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
        /*get and output the port number*/
        length = sizeof(serv_addr);
        getsockname(sockfd, (struct sockaddr *)&serv_addr, &length);
        sprintf(printBuffer, "Assigned port number %d\n", ntohs(serv_addr.sin_port));
        write(1, &printBuffer, strlen(printBuffer));
    }
    
    listen(sockfd,5);
    /*until this process has sigint sent to it keep accepting things*/
    while(1){

        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            sprintf(printBuffer, "%s", "Error accepting connection");
            write(1, &printBuffer, strlen(printBuffer));
        }

        /*reset buffer then read for client process.*/
        bzero(buffer,1024);
        n = read(newsockfd,buffer,1024);
        if (n < 0){
            sprintf(printBuffer, "%s\n", "Error reading from socket");
            write(1, &printBuffer, strlen(printBuffer));
        }

        sprintf(printBuffer, "%s %s\n", "Received command", buffer);
        write(1, &printBuffer, strlen(printBuffer));

        /*split up command based on commas*/
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
            sprintf(printBuffer, "%s", "Something went wrong in the fork");
            write(1, &printBuffer, strlen(printBuffer));
        }
        if(pid != 0){   
            /*parent*/
            int returnStatus;
            /*wait for the process to execute all things.*/
            waitpid(pid, &returnStatus, 0);

        } else if(pid == 0){/*child*/
            /*redirect stdout to go to the socket*/
            dup2(newsockfd, 1);
            close(newsockfd);   /*close unused socket*/
            execvp(arg[0], arg);   /*run the process that the command is parsed to process*/
        }

        /*reset things so another command can be run*/
        counter=0;
        bzero(buffer,1024);
    }
    
    return 0; 
}