#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
struct user{
    char name[50];
    struct sockaddr_in endp;
}user,serv;
struct message{
    char text[1000];
    struct user sender;
};
int usercmp(struct user u1, struct user u2){
    if(u1.endp.sin_addr.s_addr==u2.endp.sin_addr.s_addr && u1.endp.sin_port==u2.endp.sin_port) return 1;
    else return 0;
}
char** split(char* str, int* n){
    int i=0;
    int spaces=0;
    while(str[i]!=0){
        if(str[i]==' ') spaces++;
        i++;
    }
    char** res=malloc((spaces+1)*sizeof(char*));
    i=0;
    int j;
    for(j=0;j<=spaces;j++){
        res[j]=malloc(50);
        int k=0;
        while(str[i]!='\0'){
            if(str[i]==' '){
                break;
            }
            res[j][k]=str[i];
            k++;
            i++;
        }
        res[j][k]=0;
        i++;
    }
    *n=spaces+1;
    return res;
}
void* _send(void* arg){
    int serv_fd=*(int*)arg;
    char text[1000];
    struct message msg;
    while(1){
        fgets(text,1000,stdin);
        if(strcmp(text,"/exit\n")==0){
            strcpy(msg.text,"Exit");
            if(send(serv_fd,&msg,sizeof(msg),0)==-1){
                perror("send");
            }
            close(serv_fd);
            pthread_exit(0);
        }
        strcpy(msg.text,text);
        if(send(serv_fd,&msg,sizeof(msg),0)==-1){
            perror("send");
        }
    }
}
void* receive(void* arg){
    int serv_fd=*(int*)arg;
    while(1){
        struct message msg;
        if(recv(serv_fd,&msg,sizeof(msg),0)==-1){
            perror("recv");
            continue;
        }
        if(usercmp(msg.sender,serv)!=1){
            printf("%s: %s",msg.sender.name,msg.text);
        }
        else{
            if(strcmp(msg.text,"Dead")==0){
                printf("Server is not available now\n");
                pthread_exit(NULL);
            }
        }
    }
}
int main(){
    int sk_connect=socket(AF_INET,SOCK_STREAM,0);
    if(sk_connect==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server;
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    int connected=0;
    for(int i=4000;i<5000;i++){
        server.sin_port=htons(i);
        if(connect(sk_connect,(struct sockaddr*)&server,sizeof(server))!=-1){
            printf("Port: %d\n",i);
            connected=1;
            break;
        }
    }
    if(!connected){
        server.sin_port=htons(4000);
        if(connect(sk_connect,(struct sockaddr*)&server,sizeof(server))==-1){
            perror("connect");
            exit(EXIT_FAILURE);
        }
    }
    serv.endp=server;
    printf("Enter your name: ");
    fgets(user.name,50,stdin);
    for(int i=0;i<50;i++){
        if(user.name[i]=='\n'){
            user.name[i]=0;
            break;
        }
    }
    struct message msg;
    if(recv(sk_connect,&msg,sizeof(msg),0)==-1){
        perror("recv Ready");
        exit(EXIT_FAILURE);
    }
    if(strcmp(msg.text,"Ready")!=0){
        printf("Server error\n");
        exit(EXIT_FAILURE);
    }
    sprintf(msg.text,"Name %s",user.name);
    if(send(sk_connect,&msg,sizeof(msg),0)==-1){
        perror("send Name");
        exit(EXIT_FAILURE);
    }
    pthread_t sender,receiver;
    pthread_create(&sender,NULL,_send,&sk_connect);
    pthread_create(&receiver,NULL,receive,&sk_connect);
    pthread_join(sender,NULL);
    pthread_cancel(receiver);
    exit(EXIT_SUCCESS);
}