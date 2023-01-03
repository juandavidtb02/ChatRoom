//Library
#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netdb.h>
#include<pthread.h>
//Constant
#define SIZE 256
#define MAXLINE 4096
#define TRUE 1

void send_echo(int sock)
{
    while(TRUE)
    {
        char sendline [MAXLINE] = {};
        fgets(sendline, MAXLINE, stdin);
        write(sock, sendline, strlen(sendline));
    }
}

void receive_echo(int sock)
{
    while(TRUE)
    {
        char recvline[MAXLINE] = {};
        read(sock, recvline, MAXLINE);
        printf("%s", recvline);
    }
}

int main(int argc, char* argv[])
{
    int sock;
    char com[SIZE];
    char name[MAXLINE] = {};
    struct sockaddr_in adr;
    struct hostent *hp, *gethostbyname();

    pthread_t pth_send, pth_receive;

    if(argc != 3)
    {
        fprintf(stderr, "format: %s <HOST> <PORT>\n", argv[0]);
        exit(1);
    }

    printf("Insert your name!: ");
    fgets(name,MAXLINE,stdin);

    if((sock = socket(PF_INET, SOCK_STREAM, 0))==-1)
    {
        perror("Error: imposible create sock");
        exit(2);
    }

    if((hp=gethostbyname(argv[1]))==NULL)
    {
        perror("Error: the machine name unknown");
        exit(3);
    }

    adr.sin_family = PF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(atoi(argv[2]));
    bcopy(hp->h_addr, &adr.sin_addr, hp->h_length);

    if(connect(sock, &adr, sizeof(adr))==-1)
    {
        perror("Error: connection failed");
        exit(4);
    }

    printf("You have successfully connected\n");

    write(sock,name,strlen(name));

    pthread_create(&pth_send, NULL, (void*)&send_echo, (void*)sock);
    receive_echo(sock);

    return 0;
}