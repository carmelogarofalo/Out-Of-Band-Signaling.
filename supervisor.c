#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <wait.h>
#include <pthread.h>
#include <semaphore.h>
#include "util.h"

volatile sig_atomic_t end_flag = 0;
volatile sig_atomic_t print_flag = 0;
volatile sig_atomic_t last_sig = 0;

struct data 
{
    sem_t empty;
    sem_t endwrite;
    int serverID;
    long secret;
    long clientID;
};

void handlerSIGINT(int s) 
{
    long now = get_ms();

    if (now - last_sig <= 1000) end_flag = 1;
    else print_flag = 1;
    
    last_sig = now;
}

void controllo_parametri(int argc, char* argv[]) 
{
    if (argc != 2 ) 
    { 
        printf("Uso corretto : %s numero di server da lanciare\n", argv[0]); 
        exit(EXIT_FAILURE); 
    }
}

int main (int argc, char* argv[]) 
{
    
    controllo_parametri(argc, argv);
    
    int S = atoi(argv[1]);
    int * pids = (int*)malloc(sizeof(int)*S);
    
    if (pids == NULL) 
    { 
        fprintf(stderr, "malloc pids\n"); 
        exit(EXIT_FAILURE); 
    }

    fprintf(stdout, "SUPERVISOR STARTING %d SERVERS\n", S);

    // STRUTTURA DATI PER STIMARE IL SECRET
    
    struct supervisor_n* supervisor_list = nuova_lista_supervisor();

    // SIGNAL HANDLER

    struct sigaction s;
    sigset_t set;
    
    sigfillset(&set);
    pthread_sigmask(SIG_SETMASK, &set, NULL);
    EC(sigaction(SIGINT, NULL, &s), "sigaction");
    
    s.sa_handler = handlerSIGINT;
    EC(sigaction(SIGINT, &s, NULL), "sigaction");
    sigdelset(&set, SIGINT);
    pthread_sigmask(SIG_SETMASK, &set, NULL);

    // SHARED MEMORY

    int size = sizeof(struct data);
    int fd = shm_open("/shm-supervisor-server", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    
    if (fd == -1) 
    { 
        fprintf(stderr, "shm_open error\n"); 
        exit(EXIT_FAILURE); 
    }

    EC(ftruncate(fd, size), "ftruncate");
    struct data * shmptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (shmptr == MAP_FAILED) 
    { 
        perror("mmap\n"); 
        exit(EXIT_FAILURE); 
    }

    EC(close(fd), "close");
    
    sem_init(&shmptr->empty, 1, 0);
    sem_init(&shmptr->endwrite, 1, 0);

    // LANCIO SERVER

    for (int i=0; i<S; i++) 
    {

        int pid = fork();
        
        if (pid == -1) 
        { 
            perror("fork\n"); 
            exit(EXIT_FAILURE); 
        }

        if (pid == 0) 
        {

            char* arg = (char*)malloc(sizeof(char)*10);
            sprintf(arg, "%d", i+1);
            
            execl("server", "./server", arg, "/shm-supervisor-server", NULL);
            
            perror("execl\n");
        
        } 
        
        else pids[i] = pid;
        
    }

    sem_post(&shmptr->empty);

    // LETTURA STIME

    char id[64];

    while (end_flag == 0) 
    {

        sem_wait(&shmptr->endwrite);
        
        if (end_flag == 1) break;
        
        if (print_flag == 1) 
        {
            stampa_tabella(supervisor_list, 0);
            print_flag = 0;
        } 
        
        else 
        {

            long clientID = shmptr->clientID;
            int serverID = shmptr->serverID;
            long secret = shmptr->secret;
            
            if (esiste_client(supervisor_list, clientID) && clientID!=0) aggiorna_client(supervisor_list, clientID, secret, serverID);
            
            else if (clientID!=0) aggiungi_nuovo_client(supervisor_list, clientID, secret, serverID);

            sprintf(id, "%lx", clientID);
            
            fprintf(stdout, "SUPERVISOR ESTIMATE %ld FOR %s FROM %d\n", secret, id, serverID);
            fflush(stdout);

        }

        sem_post(&shmptr->empty);
    
    }

    // ARRIVO DEL DOPPIO SIGINT
    
    stampa_tabella(supervisor_list, 1);

    for (int i=0; i<S; i++) EC(kill(pids[i], SIGTERM), "kill");
    
    free_lista_supervisor(&supervisor_list);
    free(pids);
    
    sem_destroy(&shmptr->empty);
    sem_destroy(&shmptr->endwrite);
    
    EC(write(1, "SUPERVISOR EXITING\n", 19), "write");
    
    exit(EXIT_SUCCESS);

}
