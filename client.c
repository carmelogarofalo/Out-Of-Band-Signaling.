#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include "util.h"
#define IP "127.0.0.1"

void controllo_parametri(int argc, char* argv[])
{

    int legal = 1;

    if (argc != 4) 
    {
         printf("Numero argomenti diversi da 4\n"); 
         legal = 0; 
    }

    if (atoi(argv[2]) < 1) 
    { 
        printf("Il numero di server a cui connettersi deve essere >= 1\n"); 
        legal = 0; 
    }

    if (atoi(argv[2]) >= atoi(argv[1])) 
    { 
        printf("Il numero di server a cui connettersi deve essere minore del numero di server disponibili\n");
        legal = 0;
    }

    if (atoi(argv[3]) <= 3*atoi(argv[2])) 
    { 
        printf("Il numero di messaggi da inviare deve essere almeno 3 volte del numero di server a cui connettersi\n"); 
        legal = 0;
    }

    if (!legal) 
    {
        fprintf(stdout, "Uso corretto: %s <#server disponibili> <#server a cui connettersi> <#messaggi da inviare>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

}

int isInArr(int* arr, int num, int dim) 
{

    for (int i=0; i<dim; i++) 
    { 
        if(arr[i]==num) return 1;
    } 
    
    return 0;

}

int main (int argc, char*argv[])
{

    controllo_parametri(argc, argv);
    
    srand(time(NULL)*getpid());
    int secret = rand() % 3001;
    int S = atoi(argv[1]);
    int k = atoi(argv[2]);
    int m = atoi(argv[3]);
    long ID = rand() % 3001;
    struct timespec timer;
    timer.tv_sec = secret/1000;
    timer.tv_nsec = (secret%1000)*1000*1000;
    int serverIDs[k];
    int serverSockets[k];
    struct sockaddr_in servers[k];
    char id[64];
    int i;
  
    sprintf(id, "%lx", ID);

    fprintf(stdout,"CLIENT %s SECRET %d\n", id, secret);

    //scelta casuale dei server e connessione

    for (i=0; i<k; i++) 
    {   

        serverIDs[i] = ( rand() % S ) + 1; // [1, S-1] estraggo un numero dall'insieme
        
        while (isInArr(serverIDs, serverIDs[i], i)) serverIDs[i] = ( rand() % S ) + 1; // fino a quando e'presente lo riestraggo

        servers[i].sin_family = AF_INET;
        servers[i].sin_addr.s_addr = inet_addr(IP);
        servers[i].sin_port = htons(9000+serverIDs[i]);
        serverSockets[i] = socket(AF_INET,SOCK_STREAM,0);

        if (serverSockets[i] == -1) 
        { 
            perror("socket"); 
            exit(EXIT_FAILURE); 
        }
        
        if (connect(serverSockets[i],(struct sockaddr *)&servers[i], sizeof(servers[i])) == -1) 
        { 
            perror("connect"); 
            exit(EXIT_FAILURE); 
        }

    }

    //selezione casuale del socket e invio del messaggio

    for (int i=0; i<m; i++) 
    { 

        int sock = serverSockets[rand() % k]; //estraggo casualmente il socket
        long msg = htonl(ID);
        
        EC(write(sock, &msg, sizeof(long)), "write"); //invio il messaggio
        EC(nanosleep(&timer, NULL), "nanosleep"); //aspetto
    
    }

    //chiusura dei socket

    for (int i=0; i<k; i++) EC(close(serverSockets[i]), "close");

    fprintf(stdout, "CLIENT %s DONE\n", id);
    fflush(stdout);

}