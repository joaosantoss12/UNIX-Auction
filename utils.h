#ifndef UTILS_H_JB
#define UTILS_H_JB

#define FIFO_BACKEND "/home/joaosantos/SO-PIPES/PIPE_SO_BACKEND"        // DO FRONTEND PARA BACKEND
#define FIFO_FRONTEND "/home/joaosantos/SO-PIPES/PIPE_SO_FRONTEND_%d"      // DO BACKEND PARA FRONTEND

#define TAMANHO_STRING 150
#define TAMANHO_STRING_XXL 1000
#define MAX_UTILIZADORES 20
#define MAX_PROMOTORES 10
#define MAX_ITEMS 30

#include <pthread.h>

typedef struct utilizador{
    int pid;
    char nome[TAMANHO_STRING];        // Guardar para enviar ao backend
    char password[TAMANHO_STRING];    // Guardar para enviar ao backend
}Utilizador;

Utilizador newUtilizador;        // nova estrutura para guardar o utilizador que tenta entrar

typedef struct pedido{
    int pid;
    char nomeUtilizador[TAMANHO_STRING];
    char passwordUtilizador[TAMANHO_STRING];

    char comando[TAMANHO_STRING];
    char resposta[TAMANHO_STRING_XXL];

    int controlo;   // 1 -> ACEITE   ||   0 -> RECUSADO   ||   -1 -> ERRO DO BACKEND
}Pedido;

Pedido newPedido;
Pedido respostaPedido;

char stringSinalVida[TAMANHO_STRING];

#endif //UTILS_H_JB