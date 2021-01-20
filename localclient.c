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

#define PORT 0x0da2
#define IP_ADDR 0x7f000001
#define SIZE 1024

int sock;

void exit_cmd()
{
	if (send(sock, "CLOSE CLIENT", sizeof("CLOSE CLIENT"), 0) < 0)
	{
		perror("send");
	}
	close(sock);
	exit(0);	
}
int main(void)
{
	int nrecv;
	char input[SIZE];
	char someBuffer1[SIZE];

	signal(SIGABRT, exit_cmd);
    #ifdef SIGBREAK
    	signal(SIGBREAK, exit_cmd);
    #endif
	sock = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in s = {0};
	s.sin_family = AF_INET;
	s.sin_port = htons(PORT);
	s.sin_addr.s_addr = htonl(IP_ADDR);
	if (connect(sock, (struct sockaddr*)&s, sizeof(s)) < 0)
	{
		perror("connect");
		return 1;
	}
	printf("Successfully connected.\n");
	while(1){
		printf("Please enter command: ");
		fgets(input,SIZE,stdin);
		strtok(input,"\n");
		if (!strcasecmp(input,"exit"))
		{
			exit_cmd();
			return 0;
			//exit(0);
		}
		
		if (send(sock, input, sizeof(input), 0) < 0)
		{
			perror("send");
			return 1;
		}
		
		nrecv = recv(sock, someBuffer1, sizeof(someBuffer1), 0);
		while(strcmp(someBuffer1,"-1")>0)
		{
			if(!strcmp(someBuffer1,"CLOSE SERVER"))
			{
				printf("close socket");
				close(sock);
				return 0;
			}
			printf("%s", someBuffer1);
			if ((nrecv = recv(sock, someBuffer1, sizeof(someBuffer1), 0)) < 0)
			{
				printf("nrecv: %d",nrecv);
				perror("recv");
				close(sock);
				return 1;
			}	
		}	
		//printf("Successfully received %d bytes. Message Received: %s\n", nrecv, someBuffer1);
	//	fflush(stdout);		
	}
	
	close(sock);
	return 0;
}

