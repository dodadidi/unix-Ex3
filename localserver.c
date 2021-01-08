#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>

#define PORT 0x0da2
#define IP_ADDR 0x7f000001
#define QUEUE_LEN 20
#define SIZE 256

int main(void)
{
	char buff[SIZE]={'\0'};
	int listenS = socket(AF_INET, SOCK_STREAM, 0);	// server socket
	if (listenS < 0)
	{
		perror("socket");
		return 1;
	}
	struct sockaddr_in s = {0};						//client address
	s.sin_family = AF_INET;
	s.sin_port = htons(PORT);
	s.sin_addr.s_addr = htonl(IP_ADDR);
	if (bind(listenS, (struct sockaddr*)&s, sizeof(s)) < 0)
	{
		perror("bind");
		return 1;
	}
	if (listen(listenS, QUEUE_LEN) < 0)
	{
		perror("listen");
		return 1;
	}
	struct sockaddr_in clientIn;
	int clientInSize = sizeof clientIn;
	int newfd = accept(listenS, (struct sockaddr*)&clientIn, (socklen_t*)&clientInSize);		// client socket after connection request accepted
	if (newfd < 0)
	{
		perror("accept\n");
		return 1;
	}
	printf("Sending data to client %d\n", newfd);			
	
	if (send(newfd, "Hello World", sizeof("Hello World\n"), 0) < 0)
	{
		perror("send");
		return 1;
	}
	//thread
	/*int num = recv(newfd, buff, SIZE,0);
	printf("num: %d\n",num);
	printf("buff: %s\n",buff);*/
	while(recv(newfd, buff, SIZE,0) >= 0)
	{
		printf("recieved from client: %s\n",buff);
		printf("%d",strcmp(buff, "CLOSE CLIENT"));
		if(strcmp(buff, "CLOSE CLIENT") == 0)
		{
			break;
		}
		
		//TODO Command handling 

		else{
			/*if (send(newfd, "invalid command", sizeof("invalid command"), 0) < 0)
			{
				perror("send");
				return 1;
			}
			else
			{*/
			
			int res = system(buff);
			printf("%d",res);
			//}
		}
	}
	printf("closing client %d\n", newfd);
	close(newfd);
	
	
	printf("Done sending data to client %d. Closing socket.\n", newfd);
	//close(listenS);
	//end of thread
	//pthread_exit(NULL);//TODO

	return 0;
}
