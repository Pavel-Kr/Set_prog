#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

typedef struct{
    int fd;
    struct sockaddr_in client_addr;
} client_info;

pthread_spinlock_t lock;
FILE* log_file;

void free_res(int signo){
    printf("Shut down the server\n");
    fclose(log_file);
    pthread_spin_destroy(&lock);
    exit(EXIT_SUCCESS);
}

void print_logs(char* message, client_info info){

    printf("Client with IP address %s, port %d sent message: \"%s\"\n", 
            inet_ntoa(info.client_addr.sin_addr), ntohs(info.client_addr.sin_port), message);

    pthread_spin_lock(&lock);
    fprintf(log_file, "Client with IP address %s, port %d sent message: \"%s\"\n", 
            inet_ntoa(info.client_addr.sin_addr), ntohs(info.client_addr.sin_port), message);

    fflush(log_file);
    pthread_spin_unlock(&lock);
}

void* client_handler(void *arg){
    client_info info = *(client_info*)arg;
    free(arg);
    char message[100];
    for(int i=0;i<10;i++){
        int bytes = recv(info.fd, message, sizeof(message), 0);
        if(bytes == -1){
            perror("recv");
            exit(EXIT_FAILURE);
        }
        print_logs(message, info);
        snprintf(message, sizeof(message), "Message received!");
        if(send(info.fd, message, sizeof(message), 0) == -1){
            perror("send");
            exit(EXIT_FAILURE);
        }
    }
    close(info.fd);
    pthread_exit(NULL);
}

int main(){
    pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);
    log_file = fopen("logs.txt", "w");
    signal(SIGINT, free_res);
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
        client_info *info = malloc(sizeof(client_info));
        info->fd = client_sock;
        info->client_addr = client_addr;
        pthread_t tid;
        int retval = pthread_create(&tid, NULL, client_handler, info);
        if(retval != 0){
            errno = retval;
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        pthread_detach(tid);
    }
    
}