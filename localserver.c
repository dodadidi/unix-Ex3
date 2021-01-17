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
#define SIZE 256

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
		close(listenS);
		return 0;
	}

	/*printf("\nExecuting command \"%s\" for client...",buf);
	strcat(buf," > output.txt");
	system(buf);
	
	int fd,i;
	char res_buff[SIZE];
	fd=open("output.txt",O_RDONLY);
	if(fd==-1)
	{
		printf("Error in Opening the file\n");exit(0);
	}
	while(1)
	{
		i=read(fd,res_buff,sizeof(res_buff));
		write(newfd,buff,i);
		if(i)
		{
			break;
		}
	}

	close(fd);*/
	if (send(newfd, buff, sizeof(buff), 0) < 0)
		{
			perror("send");
			return 1;
		}
	printf("Done sending data to client %d. Closing socket.\n", newfd);
	close(newfd);
	close(listenS);
	return 0;

}
