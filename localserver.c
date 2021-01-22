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
#define QUEUE_LEN 20
#define SIZE 1024



int listenS;
fd_set file_descriptor_set;
int open_clients[SIZE] = {0};
pid_t pid;

void exit_clients() {
    int i;
    for (i=0; i<SIZE; i++) {
        if (open_clients[i]) {
            open_clients[i] = 0;
            if (send(i, "CLOSE SERVER", sizeof("CLOSE SERVER"), 0) < 0)
            {
                perror("send");
            }
            close(i);
        }
    }
    close(listenS);
    exit(1);
}


FILE* read_from_client (int file_descriptor)
{
    char buffer[SIZE];
    int nbytes = recv(file_descriptor, buffer, SIZE, 0);

    if (nbytes < 0) { 
        perror ("recv");
        exit (1);
    } else if (nbytes == 0) { 
        return NULL;
    } else { 
        if (strcasecmp(buffer, "CLOSE CLIENT") == 0)
            return NULL;
        printf ("command from client %d: '%s'\n", file_descriptor, buffer);
        return popen(buffer, "r");
    }
}
int send_to_client(int file_descriptor, FILE* res_file) {
    char buffer[SIZE];
    while (fgets(buffer, SIZE, res_file) != NULL) {
        if (send(file_descriptor, buffer, SIZE, 0) < 0)
        {
            perror("send");
            return 1;
        }
    }
    send(file_descriptor, "-1", sizeof("-1"), 0);
    return 0;
}
void client_treatment(int* socket) {
    pid = fork();
    if (!pid) {
        FILE* res_file;
        while ((res_file = read_from_client (*socket)) != NULL) {
            send_to_client(*socket, res_file);
        }
        pclose(res_file);
        if (res_file == NULL) {
            open_clients[*socket] = 0;
            close(*socket);
            FD_CLR(*socket, &file_descriptor_set);
        }
        exit(0);
    }
}
int set_newfd (struct sockaddr_in clientIn,  int clientInSize) {
    int new_file_descriptor = accept(listenS, (struct sockaddr*)&clientIn, (socklen_t*)&clientInSize);
    if (new_file_descriptor < 0)
    {
        perror("accept");
        return -1;
    }
    open_clients[new_file_descriptor] = 1;
    FD_SET (new_file_descriptor, &file_descriptor_set);
    return 0;
}
int main(void)
{
    listenS = socket(AF_INET, SOCK_STREAM, 0);
    if (listenS < 0)
    {
        perror("listen");
        return 1;
    }
    struct sockaddr_in s = {0};
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

    signal(SIGINT, exit_clients);

    struct sockaddr_in clientIn;
    int clientInSize = sizeof clientIn;

    while(1) {
        FD_ZERO(&file_descriptor_set);
        FD_SET(listenS, &file_descriptor_set);
        struct timeval timeout = {1, 0};

        int select_response = select(SIZE, &file_descriptor_set,  NULL, NULL, &timeout);
        if (select_response > 0) {
	    int possibleS=0;
            for  (possibleS=0;possibleS<SIZE; possibleS++) {
                if (FD_ISSET(possibleS, &file_descriptor_set)) {
                   int* socket = &possibleS; 
                   if (*socket == listenS) {
                       if (set_newfd(clientIn, clientInSize) < 0) {
                           exit (1);
                       }
                   } else {
                       client_treatment(socket);
                   }
                }

            }
        }


    }
    close(listenS);
    return 0;
}
