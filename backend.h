#ifndef BACKEND_H_JB
#define BACKEND_H_JB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>

#include "users_lib.h"
#include "frontend.h"
#include "utils.h"

void menos1Saldo_MetaUm(char * nome);

// FUNÇÕES PARA RESPONDER FRONTEND
bool venderItem(char * nomeVendedorNovoItem, char * nomeNovoItem, char * categoriaNovoItem,
                int precoInicialNovoItem, int precoCompreJaNovoItem, int durancaoNovoItem);

void testeItemsAdmin();
void testeItems();

void testeItemsCategoria(char * categoria);
void testeItemsVendedor(char * nome);
void testeItemsPrecoMax(int valor);
void testeItemsTempoMax(int tempo);
void mostrarSaldo(char * nome);
void adicionarSaldo(char * nome, int valor);
void licitarItem(char * nome, int id, int valor);

// COMANDOS -> FUNÇÕES ADMIN
void executaComandoAdmin(char * comando);
void testePromotor();
void testeUtilizadores();

// VARIÁVEIS DE AMBIENTE
int HEARTBEAT; // Tempo de resposta para garantir que o utilizador está vivo
char FPROMOTERS[25]; // Nome do ficheiro dos promotores
char FUSERS[25]; // Nome do ficheiro dos utilizadores
char FITEMS[25]; // Nome do ficheiro dos items

typedef struct items
{
    int id;
    int valInicial;
    int valCompreJa;
    int timeRestante;

    char nomeItem[TAMANHO_STRING];
    char nomeCategoria[TAMANHO_STRING];
    char nomeVendedor[TAMANHO_STRING];
    char nomeLicitador[TAMANHO_STRING];

    int valAntigo;
    int valNovo;
}Items;
Items listaItems[MAX_ITEMS];

int nItems = 0;
int readItems();
void saveItems();

typedef struct tdadosB
{
    int pararThread;
    int threadComecou;
    pthread_mutex_t *trinco; // partilhado entre threads
}TDadosB;

int numero_Utilizadores = 0;
int tempoSegundos = 0;

Utilizador listaUtilizadores[MAX_UTILIZADORES];

int nPromotores = 0;

typedef struct promotor
{
   int pid;
   char nome[TAMANHO_STRING];
}Promotor;
Promotor arrayPromotores[TAMANHO_STRING];

typedef struct promocao
{
    char categoria[TAMANHO_STRING];
    int desconto;
    int duracao;
}Promocao;
int nPromocoes = 0;
Promocao listaPromocoes[150];

void receberPedido(int abrirFIFO_BACKEND);

void resetResposta();

void *threadTempoSegundos(void * dadosThread);
void *threadPromotores(void * dadosThread);

int maxFDAberto(int numArgs, int **args);

#endif
