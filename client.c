//Library
#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netdb.h>
#include<pthread.h>
#include <string.h>
#include <unistd.h>

//Constant
#define SIZE 256
#define MAXLINE 4096
#define TRUE 1
#define FALSE 0

char name[SIZE] = {};

char *descrypt(char *message){
    for(int i=0;i<strlen(message)-1;i++){
        message[i] = message[i]-1;
    }
    return message;
}

char *encrypt(char *message,int sock){
    for(int i=0;i<strlen(message)-1;i++){
        message[i] = message[i]+1; //para encriptar, se suma 1 al caracter
    }
    write(sock, message, strlen(message));
}


void protocol(char *message){
    char buffer[MAXLINE] = {};
    char messageReceived[MAXLINE] = {};
    char destino[MAXLINE] = {};
    char origen[SIZE] = {};
    for(int j=0;j<strlen(message);j++){ //verifica si es un mensaje del servidor o de otro usuario
        if(message[j] != ':' && message[j+1] != ' '){
            origen[j] = message[j];
        }
        else{
            int recorrer=0;
            for (int k = j+2; k < strlen(message); k++)
            {
                messageReceived[recorrer] = message[k];
                recorrer++;
            }    
            break;
        }
    }
    char checkProtocol = FALSE;
    int i;
    if(messageReceived[0]-1 == '_'){ //verifica si el mensaje es privado mediante el protocolo
        int indice = 0;
        for (i = 1; i < strlen(messageReceived); i++) //verifica si el protocolo es correcto "_nombre_"
        {
            if(messageReceived[i]-1 != '_'){
                destino[indice] = messageReceived[i]-1;
                indice++;
            }
            else{
                checkProtocol = TRUE;
                if(messageReceived[i+1]-1 == ' '){
                    i = i+2;
                }
                else{
                    i = i+1;
                }
                break;
            }
        }

        if(checkProtocol==FALSE){ //en caso de que no sea correcto, el mensaje se desencripta y se muestra
            strcat(buffer, origen);
            strcat(buffer, ": ");
            strcat(buffer,descrypt(messageReceived));
            printf("%s",buffer);
            return;
        }
        strcat(destino,"\n"); //si el mensaje es correcto, se verifica si es enviado al actual cliente
        char mensaje[MAXLINE] = {};
        if(strcmp(destino,name) != 0){
            return; //en caso de que no sea para el actual cliente, no muestra nada
        }
        indice = 0;
        for(int j=i;j<strlen(messageReceived);j++){ //se separa el protocolo del mensaje
            buffer[indice] = messageReceived[j];
            indice++;
        }
        strcat(mensaje,"*"); //se añade un * para indiciar que es un mensaje privado
        strcat(mensaje,origen); //se añade el origen
        strcat(mensaje,": ");
        strcat(mensaje,descrypt(buffer)); //se desencripta
        printf("%s",mensaje); //se muestra
        return;
    }
    if(strlen(messageReceived) > 0){
        strcat(buffer, origen); //si el mensaje no contiene el protocolo, se desencripta y se muestra con su origen
        strcat(buffer, ": ");
        strcat(buffer,descrypt(messageReceived));
        printf("%s",buffer);
    }
    else{
        printf("%s",descrypt(message)); //se muestran mensajes del servidor
    }
    return;
    
}


void send_echo(int sock)
{
    while(TRUE)
    {
        char sendline [MAXLINE] = {};
        fgets(sendline, MAXLINE, stdin);
        encrypt(sendline,sock);
    }
}

void receive_echo(int sock)
{
    while(TRUE)
    {
        char recvline[MAXLINE] = {};
        if(read(sock, recvline, MAXLINE) > 0){
            protocol(recvline);
        }
        // char recvline[MAXLINE] = {};
        // read(sock, recvline, MAXLINE);
        // printf("%s", recvline);
    }
}

int main(int argc, char* argv[])
{
    int sock;
    char com[SIZE];
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

    if(connect(sock, (struct sockaddr*) &adr, sizeof(adr))==-1)
    {
        perror("Error: connection failed");
        exit(4);
    }

    char *succesfull;
    succesfull = malloc(sizeof(char) * 1);
    
    write(sock,name,strlen(name));
    read(sock,succesfull,1);

    if(strcmp(succesfull,"0") == 0){
        printf("Server is full, please try again later!\n");
        exit(1);
    }
    else if(strcmp(succesfull,"2") == 0){
        printf("Name already exist, please try again!\n");
        exit(1);
    }
    free(succesfull);
    
    printf("You have successfully connected\n");
    printf("---------- WELCOME TO CHATROOM ----------\n");
    printf("Use _name_ to talk to someone in private!\n");
    printf("Use /online to see online users!\n");
    printf("-----------------------------------------\n\n");
    
    pthread_create(&pth_send, NULL, (void* (*)(void*))&send_echo, (void*)(long)sock);
    receive_echo(sock);
    return 0;
}