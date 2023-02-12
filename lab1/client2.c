#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(){
    int client_sock = socket(AF_INET, SOCK_DGRAM, 0);
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
    while(1){
        snprintf(message, sizeof(message), "%d", num);
        if(sendto(client_sock, message, sizeof(message), 0, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in)) == -1){
            perror("sendto");
            exit(EXIT_FAILURE);
        }
        serv_addr_size = sizeof(struct sockaddr_in);
        if(recvfrom(client_sock, message, sizeof(message), 0, (struct sockaddr*)&serv_addr, &serv_addr_size) == -1){
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        printf("Message from server: %s\n", message);
        sleep(num);
    }
}