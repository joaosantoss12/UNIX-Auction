#include "frontend.h"

void *threadHEARTBEAT(void * dadosThread){
    TDados *dados = (TDados *) dadosThread;
    dados->threadComecou = 1;

    sprintf(stringSinalVida, "[UTILIZADOR %d] ESTA ONLINE!\n",dados->pid);
    int intervalo = atoi(getenv("HEARTBEAT"));

    do{
        pthread_mutex_lock(dados->trinco);

        if(dados->pid != 0)
            fazerPedido(newUtilizador,stringSinalVida);
        else
            dados->pararThread = 1;

        pthread_mutex_unlock(dados->trinco);
        sleep(intervalo);

    }while(dados->pararThread != 1);

    pthread_exit(NULL); //return
}

int main(int argc, char* argv[]){
    char comando[50], n;

    //Verifica se o fifo backend já existe, se não existir termina
    if(access(FIFO_BACKEND, F_OK)!=0){
        printf("[ERRO] O FIFO do backend nao existe!\n");
        exit(1);
    }

    newUtilizador.pid = getpid();

    // Criar fifo frontend
    int abrirFIFO_FRONTEND;
    char str_fifo_frontend[TAMANHO_STRING];

    // CRIAR UM FIFO PARA CADA FRONTEND (ALTERAR O DIRETÓRIO DE CADA UM | EX: FIFO_FRONTEND_2256 (pid))
    sprintf(str_fifo_frontend, FIFO_FRONTEND, newUtilizador.pid);	// str_fifo_frontend = FIFO_FRONTEND + newUtilizador.pid

    //criar o fifo [0 -> sucesso]
    if(mkfifo(str_fifo_frontend,0700) != 0){
        printf("\n[ERRO] Ao criar o FIFO FRONTEND!\n");
    }

    abrirFIFO_FRONTEND = open(str_fifo_frontend, O_RDWR);
    if(abrirFIFO_FRONTEND == -1){
        printf("\n[ERRO] Ao abrir o FIFO FRONTEND!\n");
    }

    if(argc != 3){
        printf("Erro ao logar! Use \"./frontend <nome> <password>\"\n");
        unlink(str_fifo_frontend);
        return -1;
    }
    else{
        strcpy(newUtilizador.nome, argv[1]);
        strcpy(newUtilizador.password, argv[2]);

        // ENVIA INFO
        fazerPedidoLogin(newUtilizador);
        // RECEBE INFO
        lerResposta(abrirFIFO_FRONTEND);

        if(respostaPedido.controlo == 1){
            printf("%s",respostaPedido.resposta);
            printf("Logado com sucesso!\t Nome: %s ; Password: %s",newUtilizador.nome,newUtilizador.password);

            printf("\n\n=================================================\n");
            printf("BEM-VINDO/A AO LEILAO SOBAY\n");
            printf("=================================================\n");
        }
        else if(respostaPedido.controlo == 0){
            printf("%s",respostaPedido.resposta);
            unlink(str_fifo_frontend);
            exit(100);
        }
        else if(respostaPedido.controlo == -1){
            printf("\n%s",respostaPedido.resposta);
            unlink(str_fifo_frontend);
            exit(101);
        }
    }

    // SELECT E THREADS PARA TROCAR ENTRE LER PIPE (RESPOSTA DO BACKEND) E ENVIAR PEDIDO (COMANDO)
    // SELECT
    int resultadoSelect;
    fd_set fds;
    struct timeval timeout = {5,0};

    // THREADS
    pthread_t threadID[1];
    TDados dadosThreads[1];
    pthread_mutex_t trinco;

    // INICALIZAR VALORES THREAD
    dadosThreads[0].pararThread = 0;
    dadosThreads[0].threadComecou = 0;
    dadosThreads[0].pid = getpid();
    dadosThreads[0].trinco = &trinco;
    pthread_mutex_init(&trinco, NULL);   // CRIAR TRINCO


    if(pthread_create(&threadID[0], NULL, threadHEARTBEAT, (void *) &dadosThreads[0]) != 0)
        exit(100);

    timeout.tv_sec = 20;

    do{
        printf("\n>> ");
        fflush(stdout);

        // SELECT
        FD_ZERO(&fds);
        FD_SET(0, &fds); // TECLADO
        FD_SET(abrirFIFO_FRONTEND, &fds); // FIFO (RESPOSTA)

        timeout.tv_usec = 0;

        resultadoSelect = select(abrirFIFO_FRONTEND+1, &fds, NULL, NULL, &timeout);

        if(resultadoSelect == 0){
            timeout.tv_sec = 20;
        }
        else if(resultadoSelect > 0 && FD_ISSET(0, &fds)) {
            fgets(comando, sizeof(comando), stdin);
            strtok(comando, "\n");               // LIMPAR /n DO STDIN

            executaComando(newUtilizador, comando);
        }
        else if(resultadoSelect > 0 && FD_ISSET(abrirFIFO_FRONTEND, &fds)){
            lerResposta(abrirFIFO_FRONTEND);
            printf("\n\n%s",respostaPedido.resposta);

            //// IF EXPULSO
            if(respostaPedido.controlo == -100){
                exit(-100);
            }
            //// IF TERMINOU BACKEND
            else if(respostaPedido.controlo == -2){
                exit(-2);
            }
        }

        fflush(stdout);
    }while(strcmp(comando, "exit") != 0);

    dadosThreads[0].pararThread = 1;
    close(abrirFIFO_FRONTEND);
    unlink(str_fifo_frontend);
    exit(0);
}

void fazerPedidoLogin(Utilizador dadosUtilizador){
    int abrirFIFO_BACKEND, n;

    newPedido.pid = dadosUtilizador.pid;
    strcpy(newPedido.nomeUtilizador, dadosUtilizador.nome);
    strcpy(newPedido.passwordUtilizador, dadosUtilizador.password);

    abrirFIFO_BACKEND = open(FIFO_BACKEND, O_WRONLY | O_NONBLOCK);
    if (abrirFIFO_BACKEND == -1){
        printf("[ERRO] Nao foi possivel abrir o FIFO_BACKEND!\n");
        exit(1);
    }

    n = write(abrirFIFO_BACKEND, &newPedido, sizeof(Pedido));

    if(n != sizeof(Pedido)){
        printf("[ERRO] A enviar mensagem FIFO!\n");
        exit(2);
    }

    close(abrirFIFO_BACKEND);
}

void fazerPedido(Utilizador dadosUtilizador, char comando[TAMANHO_STRING]){
    int abrirFIFO_BACKEND, n;

    newPedido.pid = dadosUtilizador.pid;
    strcpy(newPedido.nomeUtilizador, dadosUtilizador.nome);
    strcpy(newPedido.passwordUtilizador, dadosUtilizador.password);
    strcpy(newPedido.comando, comando);

    abrirFIFO_BACKEND = open(FIFO_BACKEND, O_WRONLY | O_NONBLOCK);
    if (abrirFIFO_BACKEND == -1){
        printf("[ERRO] Nao foi possivel abrir o FIFO_BACKEND!\n");
        exit(1);
    }

    n = write(abrirFIFO_BACKEND, &newPedido, sizeof(Pedido));

    if(n != sizeof(Pedido)){
        printf("[ERRO] A enviar mensagem FIFO!\n");
        exit(2);
    }

    close(abrirFIFO_BACKEND);
}

void lerResposta(int abrirFIFO_FRONTEND){
    int n;

    n = read(abrirFIFO_FRONTEND, &respostaPedido, sizeof(Pedido));

    if(n != sizeof(Pedido)){
        printf("[ERRO] A ler mensagem FIFO!\n");
        unlink(FIFO_FRONTEND);
        exit(2);
    }

}

void executaComando(Utilizador dadosUtilizador, char* comando){
    char comandoRecebido[50];
    strcpy(comandoRecebido,comando);
    char* argumentosComando[10];
    int nArgumentos = 0;        // numero de argumentos do comando ex: /sell bola 130 ; bola(posição 1) , 130(posição 2)

    argumentosComando[0] = strtok(comando, " ");
    fflush(stdin);
    while(argumentosComando[nArgumentos] != NULL){
        argumentosComando[++nArgumentos] = strtok(NULL, " ");
    }
    // strtok divide a string em palavras mas a última será sempre null.Temos de "eliminar" o ultimo "valor" , decrementando a variável nArgumentos
    // printf("%s",argumentosComando[nArgumentos]); = (null)
    nArgumentos -= 1;

    // COMANDOS
    if(strcmp(argumentosComando[0], "sell") == 0 && nArgumentos == 5){
        //printf("\nComando \"sell\" executado com sucesso! [Mostrar info do item (nome,categoria,preco-base,preco-compre-ja,duracao]\n\n");
        fazerPedido(dadosUtilizador,comandoRecebido);
    }
    else if(strcmp(argumentosComando[0], "list") == 0 && nArgumentos == 0){
        //printf("\nComando \"list\" executado com sucesso! [Mostrar lista de itens disponiveis]\n\n");
        fazerPedido(dadosUtilizador,comandoRecebido);
    }
    else if(strcmp(argumentosComando[0], "licat") == 0 && nArgumentos == 1){
        //printf("\nComando \"licat\" executado com sucesso! [Mostrar lista de itens daquela categoria]\n\n");
        fazerPedido(dadosUtilizador, comandoRecebido);
    }
    else if(strcmp(argumentosComando[0], "lisel") == 0 && nArgumentos == 1){
        //printf("\nComando \"lisel\" executado com sucesso! [Mostrar lista de itens daquele vendedor]\n\n");
        fazerPedido(dadosUtilizador,comandoRecebido);
    }
    else if(strcmp(argumentosComando[0], "lival") == 0 && nArgumentos == 1){
        //printf("\nComando \"lival\" executado com sucesso! [Mostrar lista de itens ate ao valor digitado]\n\n");
        fazerPedido(dadosUtilizador,comandoRecebido);
    }
    else if(strcmp(argumentosComando[0], "litime") == 0 && nArgumentos == 1){
        //printf("\nComando \"litime\" executado com sucesso! [Mostrar lista de itens com prazo ate ao valor digitado]\n\n");
        fazerPedido(dadosUtilizador,comandoRecebido);
    }
    else if(strcmp(argumentosComando[0], "time") == 0 && nArgumentos == 0){
        //printf("\nComando \"time\" executado com sucesso! [Obter a hora (em segundos) atual]\n\n");
        fazerPedido(dadosUtilizador,comandoRecebido);
    }
    else if(strcmp(argumentosComando[0], "buy") == 0 && nArgumentos == 2){
        //printf("\nComando \"buy\" executado com sucesso! [Comprar item com o id digitado pelo valor digitado (maior que licitação)]\n\n");
        fazerPedido(dadosUtilizador,comandoRecebido);
    }
    else if(strcmp(argumentosComando[0], "cash") == 0 && nArgumentos == 0){
        //printf("\nComando \"cash\" executado com sucesso! [Visualizar saldo do utilizador]\n\n");
        fazerPedido(dadosUtilizador,comandoRecebido);
    }
    else if(strcmp(argumentosComando[0], "add") == 0 && nArgumentos == 1){
        //printf("\nComando \"add\" executado com sucesso! [Adicionar um valor ao saldo]\n\n");
        fazerPedido(dadosUtilizador,comandoRecebido);
    }
    else if(strcmp(argumentosComando[0], "exit") == 0 && nArgumentos == 0){
        fazerPedido(dadosUtilizador,comandoRecebido);
        printf("\nA sair...\n");
    }

    else
        printf("\nComando não encontrado ou executado sem sucesso!\n\n");
}

