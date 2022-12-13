#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "util.h"

// FUNZIONI SERVER

struct nodo_client* nuova_lista_stime() 
{
    struct nodo_client *lista = (struct nodo_client *)malloc(sizeof(struct nodo_client));
    
    lista->ID = -1;
    lista->prec = -1;
    lista->best = -1;
    lista->next = NULL;
    
    return lista;
}

void aggiungi_client(struct nodo_client* current, long ID, long current_time) 
{

    if (current->ID == -1)  //primo
    {    
        current->ID = ID;
        current->prec = current_time;
        current->best = current_time;
        current->next = NULL;
    } 
    
    else if (current->next == NULL)  //ultimo
    {
        struct nodo_client *nodo = (struct nodo_client *)malloc(sizeof(struct nodo_client));
        
        nodo->ID = ID;
        nodo->prec = current_time;
        nodo->best = current_time;
        nodo->next = NULL;
        current->next = nodo;
    } 
    
    else aggiungi_client(current->next, ID, current_time);
    
}

void aggiorna_stime(struct nodo_client* lista, long ID, long current_time, int sID) 
{

    struct nodo_client* temp = lista;

    while (temp!=NULL) 
    {

        if (temp->ID == ID) 
        {
            
            if ( (current_time - temp->prec) < temp->best ) temp->best = current_time - temp->prec;
            
            temp->prec = current_time;

            return;
        
        } 
        
        else temp = temp->next;
        
    }

}

int list_contains(struct nodo_client*lista, long ID) 
{

    struct nodo_client* temp = lista;
    
    while (temp != NULL) 
    {

        if (ID == temp->ID) return 1;
        
        temp = temp->next;
    
    }

    return 0;

}

long stima_secret(struct nodo_client*lista, long ID) 
{

    struct nodo_client* temp = lista;
    
    while (temp != NULL) 
    {

        if (ID == temp->ID) return temp->best;
        
        temp = temp->next;
    
    }
    
    return -1;

}

void free_lista_stime(struct nodo_client** lista) 
{

    struct nodo_client* temp = *lista;
    struct nodo_client* prec = NULL;
    
    while (temp!=NULL) 
    {

        prec = temp;
        temp = temp->next;
        
        free(prec);
    
    }

}

// FUNZIONI SUPERVISOR

struct supervisor_n* nuova_lista_supervisor() 
{

    struct supervisor_n* list = (struct supervisor_n*)malloc(sizeof (struct supervisor_n));
    
    list->ID = -1;
    list->based_on = 0;
    list->stima = -1;
    list->next = NULL;

    struct server_n* server_ = (struct server_n*)malloc(sizeof(struct server_n));
    
    list->servers = server_;
    list->servers->serverID = -1;
    list->servers->next = NULL;

    return list;

}

void aggiungi_nuovo_client(struct supervisor_n* list, long cID, long val, int sID) 
{

    if (list->ID == -1) //primo
    {
           
        list->ID = cID;
        list->based_on = 1;
        list->stima = val;
        list->servers->serverID = sID;
    
    } 
    
    else if (list->next == NULL) 
    {

        struct supervisor_n* nodo = (struct supervisor_n*)malloc(sizeof(struct supervisor_n));
        
        nodo->ID = cID;
        nodo->stima = val;
        nodo->based_on = 1;
        nodo->next = NULL;

        struct server_n* servers = (struct server_n*)malloc(sizeof(struct server_n));
        
        nodo->servers = servers;
        nodo->servers->next = NULL;
        nodo->servers->serverID = sID;

        list->next = nodo;
    
    } 
    
    else aggiungi_nuovo_client(list->next, cID, val, sID);
    
}

void aggiorna_client(struct supervisor_n* list, long cID, long val, int sID) 
{

    struct supervisor_n* temp = list;
    
    while (temp!=NULL) 
    {

        if (temp->ID == cID) //e' presente nella lista
        {
            
            if ( temp->stima == -1 ) temp->stima = val;
            
            else if ( val < temp->stima && val!=-1 && val!=0) temp->stima = val;
            
            if (!server_registrato(temp->servers, sID)) 
            {
                
                temp->based_on++;
                struct server_n* temp1 = temp->servers;
                
                while (temp1->next!=NULL) temp1 = temp1->next;
                
                struct server_n* node = (struct server_n*)malloc(sizeof(struct server_n));
                
                node->next = NULL;
                node->serverID = sID;
                temp1->next = node;
            
            }
            
            return;
        
        } 
        
        else temp = temp->next;
        
    }

}

int esiste_client(struct supervisor_n* list, long ID) 
{

    struct supervisor_n* temp = list;
    
    while (temp!=NULL) 
    {
        if (temp->ID == ID) return 1;

        temp = temp->next;
    }
    
    return 0;

}

int server_registrato(struct server_n* list, int sID) 
{

    struct server_n * temp = list;
    
    while (temp != NULL) 
    {
        if (temp->serverID == sID) return 1;
        
        temp = temp->next;
    }

    return 0;

}

void stampa_tabella(struct supervisor_n* list, int stream) 
{

    FILE* str;
    
    if (stream == 0) str = stderr; 
    else str = stdout;

    struct supervisor_n* temp = list;
    
    if (temp->stima == -1) 
    { 
        fprintf(str, "\nNESSUNA STIMA\n"); 
        fflush(str); 
        return; 
    }

    fprintf(str, "\n\nTABELLA DELLE STIME\n");
    
    char id[64];

    while (temp!=NULL) 
    {

        sprintf(id, "%lx", temp->ID);

        fprintf(str, "SUPERVISOR ESTIMATE %ld FOR %s BASED ON %d\n", temp->stima, id, temp->based_on);

        temp = temp->next;
    
    }

    fprintf(str, "\n");
    fflush(str);

}

void free_lista_supervisor(struct supervisor_n** list) 
{

    struct supervisor_n * temp = *list;
    struct supervisor_n * prec = NULL;
    
    while (temp!=NULL) 
    {

        struct server_n * temp1 = temp->servers;
        struct server_n * prec1;
        
        while (temp1!=NULL) 
        {

            prec1 = temp1;
            temp1 = temp1->next;
            
            free(prec1);
        
        }

        prec = temp;
        temp = temp->next;
        
        free(prec);
    
    }
    
}

// FUNZIONI GENERALI

void EC(int val, char* name) 
{
    if (val == -1) 
    { 
        perror(name); 
        exit(EXIT_FAILURE); 
    }
}

long get_ms() 
{

    long ms;  
    time_t s;
    struct timespec spec;
    
    clock_gettime(CLOCK_MONOTONIC, &spec);
    s = spec.tv_sec;
    ms = spec.tv_nsec / 1.0e6;
    
    if (ms > 999) 
    {
        s++;
        ms = 0;
    }

    return (s * 1000) + ms;

}
