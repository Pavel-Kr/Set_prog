#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

int main(){
    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in)) == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    socklen_t serv_addr_size = sizeof(struct sockaddr_in);
    if(getsockname(serv_sock, (struct sockaddr*)&serv_addr, &serv_addr_size) == -1){
        perror("getsockname");
        exit(EXIT_FAILURE);
    }
    printf("Server port: %d\n", ntohs(serv_addr.sin_port));
    if(listen(serv_sock, 5) == -1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    signal(SIGCHLD, SIG_IGN);
    char message[100];
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(struct sockaddr_in);
        int client_sock = accept(serv_sock, (struct sockaddr*)&client_addr, &client_addr_size);
        if(client_sock == -1){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Client socket: %d\n", client_sock);
        pid_t pid = fork();
        if(pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            printf("Child\n");
            close(serv_sock);
            for(int i=0;i<10;i++){
                int bytes = recv(client_sock, message, sizeof(message), 0);
                if(bytes == -1){
                    perror("recv");
                    exit(EXIT_FAILURE);
                }
                printf("Client with IP address %s, port %d sent message: \"%s\"\n", 
                        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message);

                snprintf(message, sizeof(message), "Message received!");
                if(send(client_sock, message, sizeof(message), 0) == -1){
                    perror("send");
                    exit(EXIT_FAILURE);
                }
            }
            close(client_sock);
            return 0;
        }
        else{
            close(client_sock);
        }
    }
    
}