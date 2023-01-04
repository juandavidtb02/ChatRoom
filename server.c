//Library
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include <pthread.h>

//Constant
#define PORT 8110
#define MAXLINE 4096
#define TRUE 1
#define SIZE 5
#define MAX_CLIENTS 5

int client_sockets[MAX_CLIENTS];
int num_clients = 0;

char *names[MAX_CLIENTS];
char *ips[MAX_CLIENTS];




int createSocket(int *port, int type)
{
    int sockfd;
    struct sockaddr_in adr;
    int length;

    if ((sockfd = socket(PF_INET, type, 0))==-1)
    {
        perror("Error: impossible create socket");
        exit(2);
    }

    bzero((char*)&adr, sizeof(adr));
    adr.sin_port = htons(*port);
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_family = PF_INET;

    if(bind(sockfd, (struct sockaddr*)&adr, sizeof(adr))==-1)
    {
        perror("Error: bind");
        exit(3);
    }

    length = sizeof(adr);

    if(getsockname(sockfd, &adr, &length))
    {
        perror("Error: get sock name");
        exit(4);
    }

    *port = ntohs(adr.sin_port);  

    return sockfd;
}


char *concatenar(char *message,char *name,int r){
    char *mensaje = malloc(MAXLINE);
    strcat(mensaje, name);
    if(r!=0){
        strcat(mensaje, ": ");
    }
    strcat(mensaje, message);
    return mensaje;
}

void send_message_to_all_clients(char *message,int sock)
{
    for (int i = 0; i < num_clients; i++)
    {
        if(client_sockets[i] != 0){
            if(sock != client_sockets[i]){
                write(client_sockets[i], message, strlen(message));
            }
        }
    }
}

void newClient(char *name,char *ip,int sock){
    if(num_clients >= MAX_CLIENTS){
        // gestionar cola de clientes
        printf("Servidor lleno!\n");
        return;
    }
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if(client_sockets[i] == 0) {
            client_sockets[i] = sock;
            break;
        }
    }
    char *nname = strtok(name,"\n");
    names[i] = malloc(strlen(nname) + 1);
    strcpy(names[i],nname);
    num_clients++;   
    
    ips[i] = malloc(strlen(ip) + 1);
    strcpy(ips[i],ip);

    printf("%s se ha conectado desde la direccion IP %s\n",nname,ip);
    send_message_to_all_clients(concatenar(" se ha conectado!\n",name,0),sock);

}

void desconectar(int id,int sock){
    printf("%s (%s) se ha desconectado\n",names[id],ips[id]);
    send_message_to_all_clients(concatenar(" se ha desconectado!\n",names[id],0),sock);
    close(client_sockets[id]);
    names[id] = "";
    ips[id] = "";
    client_sockets[id] = 0;
    num_clients--;
}

void service(int sock)
{
    int id = 0;
    for(int i=0;i<num_clients;i++){
        if(sock == client_sockets[i]){
            id = i;
        }
    }
    while (TRUE)
    {
        char line[MAXLINE] = {};
        int r = read(sock, line, MAXLINE);
        if (r == 0)
        {
            desconectar(id,sock);
            shutdown(sock, SHUT_RDWR);
            pthread_detach(pthread_self());
            return;
        }
        send_message_to_all_clients(concatenar(line,names[id],r),sock);
    }
}

void *service_thread(void *arg) {
    int sock = (int)arg;
    service(sock);
    return NULL;
}

int main(int argc, char *argv[])
{
    int listen_socket, service_socket;
    struct sockaddr_in adr;
    int lgadr = sizeof(adr);
    int port = PORT;

    pthread_t pth_send, pth_receive;

    if((listen_socket = createSocket(&port, SOCK_STREAM))==-1)
    {
        fprintf(stderr, "Error: Could not create sock");
        exit(2);
    } 

    listen(listen_socket, 1024);

    fprintf(stdout, "The server is running on port %d\n", port);

    socklen_t client_addr_len = sizeof(adr);
    for(int i=0;i<MAX_CLIENTS;i++){
        names[i] = "";
        ips[i] = "";
        client_sockets[i] = 0;
    }
    while(TRUE)
    {
        char name[MAXLINE] = {};
        char client_ip[INET_ADDRSTRLEN];
        lgadr = sizeof(adr);
        service_socket = accept(listen_socket, &adr, &lgadr);
        if(getpeername(service_socket,(struct sockaddr*)&adr,&client_addr_len == 0)){
            inet_ntop(AF_INET,&adr.sin_addr,client_ip,INET_ADDRSTRLEN);
            //printf("El cliente se conecto desde la direccion IP %s\n",client_ip);

        }
        else{
            perror("Error al conocer la ip");
        }

        read(service_socket, name, MAXLINE);
        newClient(name,client_ip,service_socket);
        pthread_t service_thread_id;
        pthread_create(&service_thread_id, NULL, service_thread, (void*)service_socket);
    } 
    close(listen_socket);
    return 0;
}