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
#include <pthread.h>


#define MAX_CLIENTS 100
#define BUFFER_SIZE 128

/*stuct to hold file descriptor of the socket client*/
typedef struct SocketClient {
    int fd;
    char *username;
    struct SocketClient * next;
    struct SocketClient * prev;
} SocketClient;

/*Struct to hold first and last element of the list.*/
typedef struct {
    SocketClient * head;
    SocketClient * tail;
    unsigned int size;
} ClientLinkedList;

/*Global Variables.*/
ClientLinkedList * clients;
pthread_mutex_t list_lock;

void debug(char * message) {
    char printBuffer[BUFFER_SIZE];
    sprintf(printBuffer, "%s\n", message);
    write(1, &printBuffer, strlen(printBuffer));
}

/*Insert a file descriptor into the linked list.*/
void list_insert(int fd, ClientLinkedList * list) {
    SocketClient * newClient;
    SocketClient * tail = list->tail;
    SocketClient * prev = tail->prev;
    newClient = malloc(sizeof(newClient));
    newClient->next = tail;
    newClient->prev = prev;
    newClient->fd = fd;
    pthread_mutex_lock(&list_lock);
    prev->next = newClient;
    tail->prev = newClient;
    list->size++;
    pthread_mutex_unlock(&list_lock);
}

/* delete a node from the linked list.*/
void list_delete(SocketClient * toDelete, ClientLinkedList * list) {
    SocketClient * next, * prev;
    debug("Deleting by reference");
    pthread_mutex_lock(&list_lock);
    debug("Lock acquired");
    next = toDelete->next;
    debug("Got todelete->next");
    prev = toDelete->prev;
    debug("Got todelete->prev");
    prev->next = next;
    next->prev = prev;
    list->size--;
    debug("Subtracted size");
    pthread_mutex_unlock(&list_lock);
    debug("Lock unlocked, deleting object");
    free(toDelete);
}

/* delete a node from the list using the file descriptor.*/ 
void list_delete_fd(int fd, ClientLinkedList * list) {
    int found = 0;
    SocketClient * to_remove;
    SocketClient * node;
    debug("Deleting client by fd");
    for(node = list->head; node != list->tail && found == 0; node = node->next) {
        if(node->fd == fd){
            found = 1;
            to_remove = node;
        }
    }
    debug("Node found, deleting");
    list_delete(to_remove, list);
}

/*delete the entire linked list.*/
void delete_list(ClientLinkedList * list) {
    unsigned int listsize = list->size;
    SocketClient ** toremove = malloc(sizeof(SocketClient)*listsize);
    int i = 0;
    int j = 0;
    SocketClient * node;
    for(node = list->head; node != list->tail; node = node->next) {
        toremove[i] = node;
        i++;
    }
    for(j = 0; j < listsize; j++){
        free(toremove[j]);
    }
    free(list);
    free(toremove);
}

/*Method to write to all file descriptors*/
void * client_thread(void * myfd) {
    int fd = *(int*)myfd;
     /*reset buffer then read for client process.*/
    int n = 1;
    char buffer[BUFFER_SIZE];
    char printBuffer[BUFFER_SIZE];
    char output_message[BUFFER_SIZE];

    SocketClient * node;
    bzero(buffer,BUFFER_SIZE);

    /*While the socket exists/is reading a message then send it to all the other fd's*/
    while(n > 0) {
        bzero(buffer, 128);
        n = read(fd, &buffer, BUFFER_SIZE);

        if (n < 0){
            sprintf(printBuffer, "%s\n", "Error reading from socket");
            write(1, &printBuffer, strlen(printBuffer));
        }
       
        /*go through all of the list and write to all the client fd's that are not the one that sent the message.*/
        for(node = clients->head->next; node != clients->tail; node = node->next) { 
            if(node->fd != fd){
                bzero(output_message, 300);
                sprintf(output_message, "%s", buffer);
                write(node->fd, &output_message, strlen(output_message));  
            }
        }
        
    }
   
    /*used outside of the while loop because if the socket doesn't exist we want to close
    and delete it from the linked list so that it doesn't send any messages.*/
    /*close(fd);*/
    list_delete_fd(fd, clients);
    pthread_exit(NULL);
}

/*main method*/
int main(int argc, char *argv[])
{
    /*Start the socket descriptors.*/
    int sockfd, newsockfd, n;
    socklen_t length, clilen;
    char printBuffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    unsigned int numclients = 0;
    pthread_t threads[MAX_CLIENTS];

    /*Allocate space for linked list.*/
    clients = malloc(sizeof(ClientLinkedList));
    pthread_mutex_init(&list_lock, NULL);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);   /*open socket*/
    
    /*initialize linked list*/
    clients->head = malloc(sizeof(SocketClient));
    clients->tail = malloc(sizeof(SocketClient));
    clients->head->next = clients->tail;
    clients->tail->prev = clients->head;

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
        /*connected to server*/
        sprintf(printBuffer, "Connection accepted from %s\n", inet_ntoa(cli_addr.sin_addr));
        write(1, &printBuffer, strlen(printBuffer));

        /*Add a new fd to the linked list and run the thread to accept and send messages 
        to all the other fd in the linked list.*/
        list_insert(newsockfd, clients);
        if(numclients <= MAX_CLIENTS) {
            /*client method.*/
            pthread_create(&threads[numclients], NULL, client_thread, &newsockfd);
        }
    }
    
    /*Clean up at the end of the server.*/
    pthread_mutex_destroy(&list_lock);
    delete_list(clients);
    return 0; 
}