//server.c file

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <pthread.h>

#define CLIENT_NUMBER 10 //maximum 
#define BACKLOG 10     // how many pending connections queue will hold
#define BUFFERSIZE 1024 

//-------------------------------------------------

//            Global Variables

//-------------------------------------------------


void* clientHandler(void *);// thread associated functions for client handling
pthread_t clientThread[CLIENT_NUMBER];// thread array for clients
int client_count=0;// conneced client number
char buffer[BUFFERSIZE]; //golobal buffer

// client struct
struct client{
	int fd; //client file descriptor
	int port; //client port number
};

// client struct array
struct client client[CLIENT_NUMBER];


//-------------------------------------------------

//            Main Function

//-------------------------------------------------

int main(int argc, char *argv[]){

        int port=0;
	
	//check whether there is a port number
        if (argc!=2){
		printf("Please give a port number.Server terminates.\n");	
                exit(-1);
	}
	else{
		port = atoi(argv[1]);
	}

	//perfrom range check on port number(at least 1025)
        if(port<=1024)	{
		printf("Port number must be an integer above 1024.Server terminates.\n");	
		exit(-1);
	}

	// server socket params
        int sockfd; //server socket file descriptor
        struct sockaddr_in server_addr; //server socket address struct
        struct sockaddr_in client_addr; //client socket address struct
        socklen_t sin_size; //length of socket address
        int optval =1;

        //create socket, test if worked
        sockfd = socket(PF_INET, SOCK_STREAM, 0);
        if(sockfd<0){
                perror("can't create socket, program terminates.\n");
                exit(1);
        }

        //make socket re-use able in case of crashes/restars
        int set = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
        if( set == -1 ){
                perror("can't do setsockopt function, terminate.\n");
                exit(1);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

        // try to bind
        int bind_result = bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));
        if( bind_result == -1 ){
                perror("can't bind, terminate.\n");
                exit(1);
        }

        // try to listen
        int listen_result = listen(sockfd,BACKLOG);
        if( listen_result == -1 ){
                perror("can't listen, terminate.\n");
                exit(1);
        }

        // debug, show state of socket
        printf("listening on port [%d] .... \n", port );
        while(1){
                sin_size = sizeof(struct sockaddr_in);

                //accept incoming request
                client[client_count].fd = accept(sockfd, (struct sockaddr*)&client_addr, &sin_size); //return new fd for client
		client[client_count].port = htons(client_addr.sin_port); //return client port nubmer

                if((client[client_count].fd)!=-1){
                //	printf("\naccept connection %s.\n",inet_ntoa(client_addr.sin_addr));
                	printf("\nclient %d has joined the chatting room.",htons(client_addr.sin_port));
		}
                else
		{
                	printf("accept failed!\n ");
		}

		// create a new thread to handle the client
                pthread_create(&clientThread[client_count], NULL, clientHandler, &(client[client_count]));
		client_count++;//increase client index by 1
                printf("\ncurrently, there are %d clients in the chatting room.\n", client_count);
  	} 

	printf("Bottom of main function, should never get here! \n\n");
	return 0;

}//end of main


//-------------------------------------------------

//           Thread Handler Function 

//-------------------------------------------------
void* clientHandler(void * arg){

        //pass client fd and port nubmer to local variable
        int fd = (*(struct client *)arg).fd;
        int port= (*(struct client *)arg).port;

        //convert port number to char
        char port_str[10];
	sprintf(port_str,"%d",port);

        int recv_bytes;//return value of receive function

        //notify other clients about joining the chatting room
        int j;

	for(j=0; j<client_count; j++){
		if(fd!=client[j].fd){
			send(client[j].fd,port_str,sizeof(port_str),0);
			send(client[j].fd," has joined the chatting room.\n ", sizeof("  has joined the chatting room.\n "),0);
		}
	}

        while(1){
                int i;
                //receive message from client and write it to buffer
               	recv_bytes = recv(fd,buffer,BUFFERSIZE,0);

                if(recv_bytes == 0){ //|| strcmp(buffer, "quit")){
			printf("client %d has left the chatting room.\n", client[j].port);
			for(j=0; j<client_count; j++){
				if(fd!=client[j].fd){
					send(client[j].fd,port_str,sizeof(port_str),0);
					send(client[j].fd," has left the chatting room.\n ", sizeof("  has left the chatting room.\n "),0);
				}
			}
                       // close(fd);
			pthread_exit(0);

			
		}
		else{
        		printf("client %d says %s\n", port, buffer);

                	//send message to other clients
               		for(i=0; i<client_count; i++){

        			send(client[i].fd,"  ",sizeof("  "),0);
        			send(client[i].fd,port_str,sizeof(port_str),0);
        			send(client[i].fd," says: ",sizeof(" says: "),0);
        			send(client[i].fd,buffer,BUFFERSIZE,0);
			}
                }
	}

}
