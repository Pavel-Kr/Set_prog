#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(){
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(client_sock == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    char serv_ip[20];
    printf("Enter server IP address: ");
    scanf("%s", serv_ip);
    short port;
    printf("Enter server port: ");
    scanf("%hd", &port);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    inet_aton(serv_ip, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(port);
    char message[100];
    int num;
    socklen_t serv_addr_size;
    printf("Number to server: ");
    scanf("%d", &num);
    if(connect(client_sock, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in)) == -1){
        perror("connect");
        exit(EXIT_FAILURE);
    }
    int i = 0;
    while(i < 10){
        snprintf(message, sizeof(message), "%d", num);
        if(send(client_sock, message, sizeof(message), 0) == -1){
            perror("sendto");
            exit(EXIT_FAILURE);
        }
        printf("Sent %d message\n", i + 1);
        serv_addr_size = sizeof(struct sockaddr_in);
        if(recv(client_sock, message, sizeof(message), 0) == -1){
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        printf("Message from server: %s\n", message);
        i++;
        sleep(num);
    }
    shutdown(client_sock, SHUT_RDWR);
}