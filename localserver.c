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

#define PORT 0x0da2
#define IP_ADDR 0x7f000001
#define QUEUE_LEN 20
#define SIZE 1024

int main(void)
{
	int listenS = socket(AF_INET, SOCK_STREAM, 0);
	if (listenS < 0)
	{
		perror("socket");
		return 1;
	}
	struct sockaddr_in s = {0};
	s.sin_family = AF_INET;
	s.sin_port = htons(PORT);
	s.sin_addr.s_addr = htonl(IP_ADDR);
	int i = bind(listenS, (struct sockaddr*)&s, sizeof(s));
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
	struct sockaddr_in clientIn;
	int clientInSize = sizeof clientIn;
	int newfd = accept(listenS, (struct sockaddr*)&clientIn, (socklen_t*)&clientInSize);
	if (newfd < 0)
	{
		perror("accept");
		return 1;
	}
	
	char buff[SIZE]={'\0'};
	if(recv(newfd, buff, sizeof(buff),0) >= 0)
	{
		printf("recieved from client: %s\n",buff);
	}
	strtok(buff,"\n");
	if (!strcasecmp(buff,"CLOSE CLIENT"))
	{
		printf("Close server'n");
		if (send(newfd, "CLOSE SERVER", sizeof("CLOSE SERVER"), 0) < 0)
		{
			perror("send");
			close(newfd);
			close(listenS);
			return 1;
		}	
		close(newfd);
		close(listenS);
		return 0;
	}

	
	FILE* result_file = popen(buff,"r");
	char result_buff[SIZE];
	
	while(fgets(result_buff, SIZE, result_file)!=NULL)
	{
		printf("%s\n",result_buff);
		if (send(newfd, result_buff, sizeof(result_buff), 0) < 0)
		{
			perror("send");
			close(newfd);
			close(listenS);
			return 1;
		}			
	}
	send(newfd, "-1", sizeof("-1"), 0);
	printf("Done sending data to client %d. Closing socket.\n", newfd);
	close(newfd);
	close(listenS);
	return 0;

}
