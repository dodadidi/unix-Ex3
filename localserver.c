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
#include <signal.h>
#include <sys/stat.h>

#define PORT 0x0da2
#define IP_ADDR 0x7f000001
#define QUEUE_LEN 20
#define SIZE 1024

//fd_set fileDescriptors;	//DELETE
int newfd,listenS;
int clientCount = 0;
//int clientList[QUEUE_LEN] = {0}; 	//DELETE

void close_server(int newfd)
{
	printf("Close server\n");
	if (send(newfd, "CLOSE SERVER", sizeof("CLOSE SERVER"), 0) < 0)
	{
		perror("send");
	}	
	close(newfd);
	close(listenS);
//	exit(0);
}	


int send_result_to_client(char* buff, int newfd, int listenS){
	
	FILE* result_file = popen(buff,"r");
	char result_buff[SIZE];	
	printf("%s\n",buff);
		while(fgets(result_buff, SIZE, result_file)!=NULL)
		{
			if (send(newfd, result_buff, sizeof(result_buff), 0) < 0)
			{
				perror("send");
				fclose(result_file);
				close(newfd);
				exit(1);
			}					
		}
	send(newfd, "-1", sizeof("-1"), 0);
	fclose(result_file);
	return 0;
}
	
int main(void)
{
	int clientCount = 0;
	listenS = socket(AF_INET, SOCK_STREAM, 0);
	char buff[SIZE];
	int i,clientInSize,newfd, maxcl, j, slc, nrecv;
	pid_t pid;
	
	if (listenS < 0)
	{
		perror("socket");
		return 1;
	}
	struct sockaddr_in s = {0};
	s.sin_family = AF_INET;
	s.sin_port = htons(PORT);
	s.sin_addr.s_addr = htonl(IP_ADDR);
	
	i = bind(listenS, (struct sockaddr*)&s, sizeof(s));
	if ((i) < 0)
	{
		perror("bind");
		return 1;
	}
	
	if (listen(listenS, QUEUE_LEN) < 0)
	{
		perror("listen");
		return 1;
	}	
	//signal (SIGABRT, close_server(newfd));
	//signal (SIGBREAK, close_server(newfd));

	struct sockaddr_in clientIn;
	clientInSize = sizeof clientIn;
	while(1){
		newfd = accept(listenS, (struct sockaddr*)&clientIn, (socklen_t*)&clientInSize);
		if (newfd < 0)
		{
			perror("accept");
			return 1;
		}
		pid = fork();
		if (pid < 0) {
		 perror("ERROR on fork");
		 exit(1);
	      	}
	      	if (!pid)
		{
			printf("this is child process %d\n",getpid()); 
			while(1){	
			nrecv = recv(newfd, buff, sizeof(buff),0);
			if(nrecv < 0)
			{
				perror("recv");
			}
			else if(nrecv == 0)
			{
				printf("empty input\n");
				close_server(newfd);
				break;
			}
			else{
				printf("recieved from client: %s\n",buff);
				strtok(buff,"\n");
				if (!strcasecmp(buff,"CLOSE CLIENT"))
				{
					close_server(newfd);
					break;
				}
				send_result_to_client(buff,newfd,listenS);
			}

		}
		printf("Done sending data to client %d. Closing socket.\n", newfd);
		exit(0);
	}
}

	close(listenS);
	return 0;
}




/*
/*	FD_ZERO(&fileDescriptors);
	for(j = 0; j < QUEUE_LEN; j++)
	{
		if(clientList[j] > 0)
		{
			FD_SET(clientList[j], &fileDescriptors);
		} 
		if(clientList[j] > maxcl)
		{
			onlineCount++;
			maxcl = clientList[j];
		} 
	}*/
//	slc = select(maxcl+1, &fileDescriptors, NULL , NULL , NULL); */
