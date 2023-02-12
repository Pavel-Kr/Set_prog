#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet.h>

int main(){
    int serv_sock = socket(AF_INET, SOCK_DGRAM, 0);
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
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(struct sockaddr_in);
    char message[100];
    while (1)
    {
        if(recvfrom(serv_sock, message, sizeof(message), 0, (struct sockaddr*)&client_addr, &client_addr_size) == -1){
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        printf("Client with IP address %s, port %d sent message: \"%s\"\n", 
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message);

        snprintf(message, sizeof(message), "Message received!");
        if(sendto(serv_sock, message, sizeof(message), 0, (struct sockaddr*)&client_addr, client_addr_size) == -1){
            perror("sendto");
            exit(EXIT_FAILURE);
        }
    }
    
}