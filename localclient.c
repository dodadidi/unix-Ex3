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
#include <sys/poll.h>

#define PORT 0x0da2
#define IP_ADDR 0x7f000001
#define QUEUE_LEN 20
#define SIZE 1024

int sock;
fd_set file_descriptor_set;

void exit_cmd() {
    if (send(sock, "CLOSE CLIENT", sizeof("CLOSE CLIENT"), 0) < 0)
    {
        perror("send");
    }
    close(sock);
    exit(0);
}

int check_read() {
    struct pollfd fds;
    fds.fd = sock; 
    fds.events = POLLIN;
    return poll(&fds, 1, 1);
}
void enter_cmd() {
    printf("Please enter command: ");
    fflush(stdout);
}
int read_from_server () {
    char buff[SIZE];
    int nrecv = recv(sock, buff, SIZE, 0);
    while (strcmp(buff, "-1") > 0) {
        if (!strcasecmp(buff, "CLOSE SERVER")) {
            return sock;
        }
        printf("%s", buff);
        nrecv = recv(sock, buff, SIZE, 0);
    }
    if (nrecv < 0) {
        {
            perror("recv");
            return -1;
        }
    } else if (nrecv > 0) {
        enter_cmd();
    }

    return 0;
}

int check_send() {
    struct pollfd fds;
    fds.fd = 0; 
    fds.events = POLLIN;
    return poll(&fds, 1, 1);
}

int main(void)
{
    signal(SIGINT, exit_cmd);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    FD_SET (sock, &file_descriptor_set);
    struct sockaddr_in s = {0};
    s.sin_family = AF_INET;
    s.sin_port = htons(PORT);
    s.sin_addr.s_addr = htonl(IP_ADDR);
    if (connect(sock, (struct sockaddr*)&s, sizeof(s)) < 0)
    {
        perror("connect");
        return 1;
    }
    enter_cmd();
    while(1) {
        if (check_send() > 0) {
            char buff[SIZE];
            fgets (buff, SIZE, stdin);
            strtok(buff, "\n");
            if (!strcasecmp(buff, "exit")) {
                exit_cmd();
                exit(0);
            }
            if (send(sock, buff, SIZE, 0) < 0)
            {
                perror("send");
                return 1;
            }
        }
        if (check_read() > 0) {
            if (read_from_server() > 0) {
                exit_cmd();
                exit(0);
            }
        }

    }
}
