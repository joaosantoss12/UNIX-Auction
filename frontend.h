#ifndef FRONTEND_H_JB
#define FRONTEND_H_JB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "utils.h"

typedef struct tdados
{
    int pararThread;
    int threadComecou;
    pthread_mutex_t *trinco; // partilhado entre threads

    int pid;
    char nome[TAMANHO_STRING];
    char password[TAMANHO_STRING];
}TDados;

void executaComando(Utilizador dadosUtilizador, char* comando);
void fazerPedidoLogin(Utilizador dadosUtilizador);
void fazerPedido(Utilizador dadosUtilizador, char comando[TAMANHO_STRING]);
void lerResposta(int abrirFIFO_FRONTEND);

void *threadHEARTBEAT(void * dadosThread);

#endif
