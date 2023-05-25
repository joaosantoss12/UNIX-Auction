#include "backend.h"

void resetResposta(){
    respostaPedido.pid = -10;
    strcpy(respostaPedido.nomeUtilizador,"");
    strcpy(respostaPedido.passwordUtilizador,"");
    strcpy(respostaPedido.comando,"");
    strcpy(respostaPedido.resposta,"");
    respostaPedido.controlo = -10;
}   // RESETA OS PARAMETROS DO pedidoResposta PARA NAO GUARDAR INFORMAÇÃO DE OUTRO FRONTEND/OUTRO PEDIDO ANTIGO

void receberPedido(int fifo_backend){
    int abrirFIFO_FRONTEND, retorno, n;

    loadUsersFile(getenv("FUSERS"));

    n = read(fifo_backend, &newPedido, sizeof(Pedido));

    char str_fifo_frontend[TAMANHO_STRING];
    // CRIAR UM FIFO PARA CADA FRONTEND (ALTERAR O DIRETÓRIO DE CADA UM | EX: FIFO_FRONTEND_2256 (pid))
    sprintf(str_fifo_frontend, FIFO_FRONTEND, newPedido.pid);	// str_fifo_frontend = FIFO_FRONTEND + newPedido.pid

    abrirFIFO_FRONTEND = open(str_fifo_frontend,O_WRONLY);

    if(n == sizeof(Pedido)) {
        fflush(stdout);
        printf("\nNOME: %s | PID: %d\n",newPedido.nomeUtilizador,newPedido.pid);
        printf("COMANDO RECEBIDO: %s\n",newPedido.comando);

        fflush(stdout);

        if (strcmp(newPedido.comando, "") != 0) {     // UTILIZADOR ENVIOU COMANDO OU APENAS LOGIN?
            char *argumentosComando[10];
            int nArgumentos = 0;

            argumentosComando[0] = strtok(newPedido.comando, " ");
            fflush(stdin);
            while (argumentosComando[nArgumentos] != NULL) {
                argumentosComando[++nArgumentos] = strtok(NULL, " ");
            }
            // strtok divide a string em palavras mas a última será sempre null.Temos de "eliminar" o ultimo "valor" , decrementando a variável nArgumentos
            // printf("%s",argumentosComando[nArgumentos]); = (null)
            nArgumentos -= 1;

            if (strcmp(argumentosComando[0], "sell") == 0) {
                venderItem(newPedido.nomeUtilizador, argumentosComando[1], argumentosComando[2], atoi(argumentosComando[3]), atoi(argumentosComando[4]), atoi(argumentosComando[5]));
            }
            if (strcmp(argumentosComando[0], "list") == 0) {
                testeItems();
            }
            else if (strcmp(argumentosComando[0], "licat") == 0) {
                testeItemsCategoria(argumentosComando[1]);
            }
            else if (strcmp(argumentosComando[0], "lisel") == 0) {
                testeItemsVendedor(argumentosComando[1]);
            }
            else if (strcmp(argumentosComando[0], "lival") == 0) {
                testeItemsPrecoMax(atoi(argumentosComando[1]));
            }
            else if (strcmp(argumentosComando[0], "lival") == 0) {
                testeItemsTempoMax(atoi(argumentosComando[1]));
            }
            else if (strcmp(argumentosComando[0], "litime") == 0) {
                testeItemsTempoMax(atoi(argumentosComando[1]));
            }
            else if(strcmp(argumentosComando[0], "time") == 0){
                //// FUNÇÃO
                char auxString[TAMANHO_STRING];
                sprintf(auxString, "Hora atual (segundos): %d\n", tempoSegundos);
                strcpy(respostaPedido.resposta, auxString);
            }
            else if (strcmp(argumentosComando[0], "buy") == 0) {
                licitarItem(newPedido.nomeUtilizador, atoi(argumentosComando[1]), atoi(argumentosComando[2]));
            }
            else if (strcmp(argumentosComando[0], "cash") == 0) {
                mostrarSaldo(newPedido.nomeUtilizador);
            }
            else if (strcmp(argumentosComando[0], "add") == 0) {
                adicionarSaldo(newPedido.nomeUtilizador, atoi(argumentosComando[1]));
            }
            else if (strcmp(argumentosComando[0], "exit") == 0) {
                printf("Utilizador saiu -> NOME: %s | PID: %d\n", newPedido.nomeUtilizador, newPedido.pid);
                --numero_Utilizadores;
                //// APAGAR DO ARRAY [FAZER FUNÇÃO]
                bool apagouUtilizador = false;
                for(int i=0;i<numero_Utilizadores;i++){
                    if(apagouUtilizador){
                        listaUtilizadores[i] = listaUtilizadores[i+1];

                        //// RESETAR PROPRIEDADES DO ULTIMO
                        listaUtilizadores[i+1].pid = 0;
                        strcpy(listaUtilizadores[i+1].nome, "");
                        strcpy(listaUtilizadores[i+1].password, "");
                    }
                    if(listaUtilizadores[i].pid == newPedido.pid){
                        apagouUtilizador = true;
                    }
                }
            }
            write(abrirFIFO_FRONTEND, &respostaPedido, sizeof(Pedido));
            resetResposta();
        }
        else {
            retorno = isUserValid(newPedido.nomeUtilizador, newPedido.passwordUtilizador);

            if (retorno == 0) {
                printf("\nUtilizador nao existe ou password invalida!\n");
                strcpy(respostaPedido.resposta, "Utilizador nao existe ou password invalida!\n");
                respostaPedido.controlo = 0;
                write(abrirFIFO_FRONTEND, &respostaPedido, sizeof(Pedido));
                resetResposta();

            }
            else if (retorno == 1) {
                ++numero_Utilizadores;

                ///// UTILIZADOR JA ATIVO NESTE SERVIDOR
                for(int i=0;i<numero_Utilizadores;i++){
                    if(strcmp(listaUtilizadores[i].nome, newPedido.nomeUtilizador) == 0){
                        --numero_Utilizadores;
                        printf("\nUtilizador ja esta ativo!\n");
                        strcpy(respostaPedido.resposta, "\nUtilizador ja esta ativo!\n");
                        respostaPedido.controlo = 0;
                        write(abrirFIFO_FRONTEND, &respostaPedido, sizeof(Pedido));
                        resetResposta();
                        break;
                    }
                }

                //// NUMERO MAXIMO DE UTILIZADORES ATINGIDO
                if(numero_Utilizadores > MAX_UTILIZADORES){
                    --numero_Utilizadores;
                    printf("\nUtilizador valido mas limite de utilizadores ativos atingido!\n");
                    strcpy(respostaPedido.resposta, "\nUtilizador valido mas limite de utilizadores ativos atingido!\n");
                    respostaPedido.controlo = 0;
                    write(abrirFIFO_FRONTEND, &respostaPedido, sizeof(Pedido));
                    resetResposta();
                }

                printf("\nUtilizador valido!\n");

                // PREENCHER ARRAY DE UTILIZADORES ATIVOS
                listaUtilizadores[numero_Utilizadores-1].pid = newPedido.pid;
                strcpy(listaUtilizadores[numero_Utilizadores-1].nome, newPedido.nomeUtilizador);
                strcpy(listaUtilizadores[numero_Utilizadores-1].password, newPedido.passwordUtilizador);

                strcpy(respostaPedido.resposta, "Utilizador valido!\n");
                respostaPedido.controlo = 1;
                write(abrirFIFO_FRONTEND, &respostaPedido, sizeof(Pedido));
                resetResposta();

            }
            else if (retorno == -1) {
                printf("\n%s", getLastErrorText());
                respostaPedido.controlo = -1;
                write(abrirFIFO_FRONTEND, &respostaPedido, sizeof(Pedido));
                resetResposta();
            }
        }
    }
}

int readItems(){
    int i = 0;
    nItems = 0;

    FILE *file;
    file = fopen(getenv("FITEMS"), "r");

    if(file != NULL){
        while(!feof(file)){
            fscanf(file, "%d %s %s %d %d %d %s %s", &listaItems[i].id, listaItems[i].nomeItem, listaItems[i].nomeCategoria, &listaItems[i].valInicial, &listaItems[i].valCompreJa, &listaItems[i].timeRestante, listaItems[i].nomeVendedor, listaItems[i].nomeLicitador);
            listaItems[i].valAntigo = 0;
            listaItems[i].valNovo = 0;
            i++;
            nItems++;
        }
    }
    else
        printf("\n\n[ERRO] Nao foi possivel abrir o ficheiro!\n");

    fclose(file);

    return i;
}

void saveItems(){

    FILE *fileWrite;
    fileWrite = fopen(getenv("FITEMS"), "w");

    if(fileWrite != NULL){
        for(int k=0;k<nItems;k++){
            if(k == (nItems-1))
                fprintf(fileWrite, "%d %s %s %d %d %d %s %s", listaItems[k].id, listaItems[k].nomeItem, listaItems[k].nomeCategoria, listaItems[k].valInicial, listaItems[k].valCompreJa, listaItems[k].timeRestante, listaItems[k].nomeVendedor, listaItems[k].nomeLicitador);
            else
                fprintf(fileWrite, "%d %s %s %d %d %d %s %s\n", listaItems[k].id, listaItems[k].nomeItem, listaItems[k].nomeCategoria, listaItems[k].valInicial, listaItems[k].valCompreJa, listaItems[k].timeRestante, listaItems[k].nomeVendedor, listaItems[k].nomeLicitador);
        }
    }

    fclose(fileWrite);
}

void updateItems(){
    int i = readItems();

    bool apagou = false;
    int posItem = 0;

    for(int j=0;j<nItems;j++){
        --listaItems[j].timeRestante;
        if(listaItems[j].timeRestante == 0) {
            --nItems;
            --i;
            posItem = j;
            apagou = true;

            for (int p = 0; p < numero_Utilizadores; p++) {
                if (strcmp(listaItems[j].nomeLicitador, listaUtilizadores[p].nome) == 0) {
                    printf("\nVencedor do item[%d]->[%s]: %s\n", listaItems[posItem].id, listaItems[posItem].nomeItem, listaUtilizadores[p].nome);

                    int abrirFifoFRONTENDD;
                    char auxString[TAMANHO_STRING];
                    sprintf(auxString, FIFO_FRONTEND, listaUtilizadores[p].pid);

                    abrirFifoFRONTENDD = open(auxString, O_WRONLY);

                    sprintf(respostaPedido.resposta, "Venceu a licitacao do item[%d]: %s por %d!\n", listaItems[j].id, listaItems[j].nomeItem, listaItems[j].valInicial);

                    updateUserBalance(listaUtilizadores[p].nome, getUserBalance(listaUtilizadores[p].nome) - listaItems[j].valInicial);
                    saveUsersFile(getenv("FUSERS"));

                    write(abrirFifoFRONTENDD, &respostaPedido, sizeof(Pedido));

                    close(abrirFifoFRONTENDD);

                }
            }

            if(apagou){
                // ADICIONAR DINHEIRO AO VENDEDOR
                for (int p = 0; p < numero_Utilizadores; p++) {
                    if (strcmp(listaUtilizadores[p].nome, listaItems[posItem].nomeVendedor) == 0) {

                        updateUserBalance(listaUtilizadores[p].nome, getUserBalance(listaUtilizadores[p].nome) + listaItems[posItem].valInicial);
                        saveUsersFile(getenv("FUSERS"));

                        int abrirFifoFRONTEND;
                        char auxString[TAMANHO_STRING];
                        sprintf(auxString, FIFO_FRONTEND, listaUtilizadores[p].pid);

                        abrirFifoFRONTEND = open(auxString, O_WRONLY);

                        sprintf(respostaPedido.resposta, "O seu item[%d]: %s foi vendido e recebeu +%d!\n", listaItems[posItem].id, listaItems[posItem].nomeItem, listaItems[posItem].valInicial);

                        write(abrirFifoFRONTEND, &respostaPedido, sizeof(Pedido));

                        close(abrirFifoFRONTEND);
                    }
                }

                for (int p = 0; p < numero_Utilizadores; p++) {
                    int abrirFifoFRONTENDDD;
                    char auxString[TAMANHO_STRING];

                    sprintf(auxString, FIFO_FRONTEND, listaUtilizadores[p].pid);

                    abrirFifoFRONTENDDD = open(auxString, O_WRONLY);

                    strcpy(respostaPedido.resposta, "");

                    sprintf(respostaPedido.resposta, "item[%d] foi vendido\nNome: %s | Categoria: %s | Valor de venda: %d | Comprador: %s\n", listaItems[posItem].id, listaItems[posItem].nomeItem, listaItems[posItem].nomeCategoria, listaItems[posItem].valInicial, listaItems[posItem].nomeLicitador);
                    write(abrirFifoFRONTENDDD, &respostaPedido, sizeof(Pedido));

                    close(abrirFifoFRONTENDDD);
                }

                for(int m=posItem;m<=nItems;m++){
                    strcpy(listaItems[m].nomeItem, listaItems[m+1].nomeItem);
                    strcpy(listaItems[m].nomeCategoria, listaItems[m+1].nomeCategoria);
                    listaItems[m].valInicial = listaItems[m+1].valInicial;
                    listaItems[m].valCompreJa = listaItems[m+1].valCompreJa;
                    listaItems[m].timeRestante = listaItems[m+1].timeRestante;
                    strcpy(listaItems[m].nomeCategoria, listaItems[m+1].nomeCategoria);
                    strcpy(listaItems[m].nomeVendedor, listaItems[m+1].nomeVendedor);
                    strcpy(listaItems[m].nomeLicitador, listaItems[m+1].nomeLicitador);
                }
            }

            if(strcmp(listaItems[j].nomeLicitador, "-")==0){
                printf("item[%d]: %s removido sem licitador!", listaItems[j].id, listaItems[j].nomeItem);

                for(int p=0;p<numero_Utilizadores;p++) {
                    int abrirFifoFRONTENDDDD;
                    char auxString[TAMANHO_STRING];
                    sprintf(auxString, FIFO_FRONTEND, listaUtilizadores[p].pid);

                    abrirFifoFRONTENDDDD = open(auxString, O_WRONLY);

                    sprintf(respostaPedido.resposta, "ITEM[%d] foi vendido\nNome: %s | Categoria: %s | Valor de venda: %d | Comprador: 'por vender'\n", listaItems[j].id, listaItems[j].nomeItem, listaItems[j].nomeCategoria, listaItems[j].valInicial);
                    write(abrirFifoFRONTENDDDD, &respostaPedido, sizeof(Pedido));

                    close(abrirFifoFRONTENDDDD);
                }
            }

        }
    }

    saveItems();
}

void updatePromocoes(){

    readItems();

    int posPromo = 0;

    for(int k=0;k<nPromocoes;k++){
        --listaPromocoes[k].duracao;
        if(listaPromocoes[k].duracao == 0){
            --nPromocoes;
            printf("\nPROMOCAO %s ACABOU!\n",listaPromocoes[k].categoria);

            for(int l=0;l<nItems;l++){
                if(strcmp(listaPromocoes[k].categoria, listaItems[l].nomeCategoria)==0){
                    printf("ANTES [promocao]: %d\n",listaItems[l].valCompreJa);
                    listaItems[l].valCompreJa = (listaItems[l].valCompreJa / (1 - listaPromocoes[k].desconto/100));
                    printf("AGORA: %d\n",listaItems[l].valCompreJa);
                }
            }

            for(int u=0; u<=numero_Utilizadores;u++){
                int abrirFifoFRONTEND_D;
                char auxStringg[TAMANHO_STRING];
                sprintf(auxStringg, FIFO_FRONTEND, listaUtilizadores[u].pid);

                abrirFifoFRONTEND_D = open(auxStringg, O_WRONLY);

                for(int v=0;v<nItems;v++){
                    if(strcmp(listaItems[v].nomeCategoria, listaPromocoes[k].categoria)==0) {
                        sprintf(respostaPedido.resposta, "\n[PROMOCAO TERMINADA]\nCategoria: %s\nANTES [promocao]: %d | AGORA: %d\n",listaPromocoes[k].categoria, listaItems[v].valNovo, listaItems[v].valAntigo);
                        write(abrirFifoFRONTEND_D, &respostaPedido, sizeof(Pedido));
                        resetResposta();
                    }
                }

                close(abrirFifoFRONTEND_D);
            }
        }
    }

    for(int y=posPromo;y<=nPromocoes;y++){
        listaPromocoes[y].desconto = listaPromocoes[y+1].desconto;
        listaPromocoes[y].duracao = listaPromocoes[y+1].duracao;
        strcpy(listaPromocoes[y].categoria, listaPromocoes[y+1].categoria);
    }
}

void *threadTempoSegundos(void * dadosThread){
    TDadosB *dados = (TDadosB *) dadosThread;
    dados->threadComecou = 1;

    do{
        pthread_mutex_lock(dados->trinco);

        ++tempoSegundos;
        updateItems();
        updatePromocoes();

        pthread_mutex_unlock(dados->trinco);
        sleep(1);

    }while(dados->pararThread != 1);

    pthread_exit(NULL); //return
}

int main(int argc, char* argv[], char* envp[]){
    char comando[TAMANHO_STRING];

    if(getenv("FUSERS") == NULL){
        printf("\nVariavel ambiente \"FUSERS\" nao foi definida!\n");
        exit(-1);
    }
    else if(getenv("FITEMS") == NULL){
        printf("\nVariavel ambiente \"FITEMS\" nao foi definida!\n");
        exit(-1);
    }
    else if(getenv("FPROMOTERS") == NULL){
        printf("\nVariavel ambiente \"FPROMOTERS\" nao foi definida!\n");
        exit(-1);
    }
    else if(getenv("HEARTBEAT") == NULL){
        printf("\nVariavel ambiente \"HEARTBEAT\" nao foi definida!\n");
        exit(-1);
    }

    // Criar fifo backend
    int abrirFIFO_BACKEND;
    if(access(FIFO_BACKEND, F_OK)==0){
        printf("\nJa existe um backend ativo!\n");
        exit(-1);
    }
    //criar o fifo [0 -> sucesso]
    if(mkfifo(FIFO_BACKEND,0700) != 0){
        // ERRO AO CRIAR FIFO BACKEND
        printf("\nErro ao criar o fifo Backend!\n");
        exit(0);
    }
    abrirFIFO_BACKEND = open(FIFO_BACKEND, O_RDWR);
    if(abrirFIFO_BACKEND == -1){
        printf("\nErro ao abrir o fifo para resposta!\n");
        exit(-1);
    }

    // LER FICHEIRO QUE CONTEM OS SEGUNDOS ATUALIADOS DESDE A ÚLTIMA EXECUÇÃO
    FILE *tempo;
    tempo = fopen("tempo.txt","r");

    if(tempo != NULL){
        while(!feof(tempo)) {
            fscanf(tempo,"%d", &tempoSegundos);
        }
    }
    else{
        printf("[ERRO] Ao retomar o tempo da ultima sessao!\n");
        fclose(tempo);
        exit(999);
    }
    fclose(tempo);

    printf("\n=================================================\n");
    printf("BEM-VINDO/A AO LEILAO SOBAY\n");
    printf("=================================================\n");

    // SELECT E THREADS PARA TROCAR ENTRE LER PIPE (RESPOSTA DO BACKEND) E ENVIAR PEDIDO (COMANDO)
    // SELECT
    int resultadoSelect;
    fd_set fds;
    struct timeval timeout = {5,0};

    // THREADS
    pthread_t threadID[1];
    TDadosB dadosThreads[1];
    pthread_mutex_t trinco;

    // INICALIZAR VALORES THREAD
    dadosThreads[0].pararThread = 0;
    dadosThreads[0].threadComecou = 0;
    dadosThreads[0].trinco = &trinco;
    pthread_mutex_init(&trinco, NULL);   // CRIAR TRINCO

    if(pthread_create(&threadID[0], NULL, threadTempoSegundos, (void *) &dadosThreads[0]) != 0)
        exit(100);

    int resultadoFork;
    int resultadoPipe;
    int unamed_pipe_read_from_promotor[2];

    int len = 0;

    char promocao[50][100];

    // ============= ABRIR O FICHEIRO PROMOTORES E GUARDAR NOME DOS PROMOTORES EM UM ARRAY ===============
    FILE *filePromotores;
    filePromotores = fopen(getenv("FPROMOTERS"), "r");
    char auxPromotor[TAMANHO_STRING];

    if(filePromotores != NULL){
        while(!feof(filePromotores)){

            fscanf(filePromotores, "%s", auxPromotor);

            strcpy(arrayPromotores[nPromotores].nome,auxPromotor);
            ++nPromotores;
        }
    }
    else
        printf("\n\n[ERRO] Nao foi possivel abrir o ficheiro!\n");

    fclose(filePromotores);


    resultadoPipe = pipe(unamed_pipe_read_from_promotor);
    if (resultadoPipe == -1){
        printf("Erro ao criar unnamed pipe!\n");
        exit(-1);
    }

    for(int k=0;k<nPromotores;k++){
        resultadoFork = fork();

        if(resultadoFork == -1){        // FILHO NAO FOI CRIADO COM SUCESSO
            printf("\n[ERRO] Criacao do processo filho nao foi possivel!\n\n");
            exit(-1);
        }
        else if(resultadoFork == 0)     // FILHO
        {
            // FECHAR STDOUT DO SISTEMA
            close(1);
            // COLOCAR STDOUT NO PIPE PARA RECEBER NO PAI
            dup(unamed_pipe_read_from_promotor[1]);
            // FECHAR PIPES
            close(unamed_pipe_read_from_promotor[1]);
            close(unamed_pipe_read_from_promotor[0]);



            // EXECUTAR PROMOTOR
            //execl("promotor_oficial","promotor_oficial",NULL);
            execl(arrayPromotores[k].nome,arrayPromotores[k].nome,NULL);   // VOLTA PARA O PAI

            printf("\n[ERRO] Executar promotor atraves do processo filho falhou!\n\n");
            exit(-1);
        }
        else if(resultadoFork > 0){
            arrayPromotores[k].pid = resultadoFork;
        }
    }
    // ========================= FIM DO LANÇAMENTO PROMOTORES ==========================================

    timeout.tv_sec = 20;

    //todos os file descriptores que vão ficar abertos
    int *openFDs[2] = {&abrirFIFO_BACKEND, &unamed_pipe_read_from_promotor[0]};

    do{
        printf("\n>> ");
        fflush(stdout);

        // SELECT
        FD_ZERO(&fds);
        FD_SET(0, &fds); // TECLADO
        FD_SET(abrirFIFO_BACKEND, &fds); // FIFO (PEDIDO)
        FD_SET(unamed_pipe_read_from_promotor[0], &fds);

        timeout.tv_usec = 0;

        if(nPromotores <= 0){
            int *openFDs[1] = {&abrirFIFO_BACKEND};
            resultadoSelect = select(maxFDAberto(1, openFDs)+1, &fds, NULL, NULL, &timeout);
        }
        resultadoSelect = select(maxFDAberto(2, openFDs)+1, &fds, NULL, NULL, &timeout);

        //pthread_mutex_lock(&trinco);

        if(resultadoSelect == 0){
            timeout.tv_sec = 20;
        }
        else if(resultadoSelect > 0 && FD_ISSET(0, &fds)) {
            fgets(comando, sizeof(comando), stdin);
            strtok(comando, "\n");               // LIMPAR /n DO STDIN

            executaComandoAdmin(comando);
        }
        else if(resultadoSelect > 0 && FD_ISSET(abrirFIFO_BACKEND, &fds)){
            receberPedido(abrirFIFO_BACKEND);
        }
        else if(resultadoSelect > 0 && FD_ISSET(unamed_pipe_read_from_promotor[0], &fds) && nPromotores > 0){
            char promocao[TAMANHO_STRING];

            close(unamed_pipe_read_from_promotor[1]);

            len = read(unamed_pipe_read_from_promotor[0], promocao, sizeof(promocao));
            strtok(promocao, "\n");

            if (len > 0) {
                printf("\nPROMOCAO: %s\n", promocao);
            }
            else
                printf("\nErro a receber a promocao!\n");


            char* argumentosPromocao[3];
            int nArgumentosPromocao = 0;        // numero de argumentos da promocao -> categoria desconto duração

            argumentosPromocao[0] = strtok(promocao, " ");
            fflush(stdin);
            while(argumentosPromocao[nArgumentosPromocao] != NULL){
                argumentosPromocao[++nArgumentosPromocao] = strtok(NULL, " ");
            }
            // strtok divide a string em palavras mas a última será sempre null.Temos de "eliminar" o ultimo "valor" , decrementando a variável nArgumentos
            // printf("%s",argumentosComando[nArgumentos]); = (null)
            nArgumentosPromocao -= 1;

            //for(int p=0;p<=nPromocoes;p++){
                //if(strcmp(listaPromocoes[p].categoria, argumentosPromocao[0]) != 0) // APENAS SE A CATEGORIA NAO ESTIVER EM PROMOÇÃO


            strcpy(listaPromocoes[nPromocoes].categoria, argumentosPromocao[0]);
            listaPromocoes[nPromocoes].desconto = atoi(argumentosPromocao[1]);
            listaPromocoes[nPromocoes].duracao = atoi(argumentosPromocao[2]);


            int n = readItems();
            bool existe = false;
            bool existe2 = false;

            for(int v=0;v<n;v++){
                if(strcmp(listaItems[v].nomeCategoria, listaPromocoes[nPromocoes].categoria)==0){
                    existe = true;
                    if(listaItems[v].valCompreJa > 0){
                        listaItems[v].valAntigo = listaItems[v].valCompreJa;
                        printf("ANTES: %d\n",listaItems[v].valAntigo);
                        listaItems[v].valCompreJa = (listaItems[v].valCompreJa * (100 - listaPromocoes[nPromocoes].desconto))/100;
                        listaItems[v].valNovo = listaItems[v].valCompreJa;
                        printf("AGORA: %d\n",listaItems[v].valNovo);
                    }
                }
            }
            if(!existe)
                printf("Nenhum item no leilao com a categoria da promocao...\n");

            saveItems();

            for(int u=0; u<=numero_Utilizadores;u++){
                int abrirFifoFRONTEND_D;
                char auxStringg[TAMANHO_STRING];
                sprintf(auxStringg, FIFO_FRONTEND, listaUtilizadores[u].pid);

                abrirFifoFRONTEND_D = open(auxStringg, O_WRONLY);

                for(int v=0;v<n;v++){
                    if(strcmp(listaItems[v].nomeCategoria, listaPromocoes[nPromocoes].categoria)==0) {
                        existe2 = true;

                        sprintf(respostaPedido.resposta, "\n[NOVA PROMOCAO]\nCategoria: %s | Desconto: %d | Duracao: %d\nANTES: %d | AGORA: %d\n",listaPromocoes[nPromocoes].categoria, listaPromocoes[nPromocoes].desconto, listaPromocoes[nPromocoes].duracao, listaItems[v].valAntigo, listaItems[v].valNovo);
                        write(abrirFifoFRONTEND_D, &respostaPedido, sizeof(Pedido));
                        resetResposta();

                        nPromocoes++;
                    }
                }
                if(existe2){
                    sprintf(respostaPedido.resposta, "\n[NOVA PROMOCAO]\nCategoria: %s | Desconto: %d | Duracao: %d\nNenhum item no leilao com a categoria em promocao...\n",listaPromocoes[nPromocoes].categoria, listaPromocoes[nPromocoes].desconto, listaPromocoes[nPromocoes].duracao);
                    write(abrirFifoFRONTEND_D, &respostaPedido, sizeof(Pedido));
                    resetResposta();
                }

                close(abrirFifoFRONTEND_D);
            }
        }


    }while(strcmp(comando,"close") != 0);

    // ESCREVER FICHEIRO QUE CONTEM OS SEGUNDOS ATUALIZADOS
    FILE *tempo2;
    tempo2 = fopen("tempo.txt","w");

    if(tempo2 != NULL){
        fprintf(tempo2,"%d", tempoSegundos);
    }
    else{
        printf("[ERRO] Nao foi possivel guardar o tempo em segundos atual!\n");
        fclose(tempo2);
        exit(999);
    }

    fclose(tempo2);

    unlink(FIFO_BACKEND);
    return 1;
}

int maxFDAberto(int numArgs, int **args){
    int maxFD = 0;

    for (int i = 0; i < numArgs; i++)
        if(maxFD < *args[i])
            maxFD = *args[i];

    return maxFD;
}

void executaComandoAdmin(char* comando){
    char* argumentosComando[10];
    int nArgumentos = 0;

    argumentosComando[nArgumentos] = strtok(comando, " ");
    while(argumentosComando[nArgumentos] != NULL){
        argumentosComando[++nArgumentos] = strtok(NULL, " ");
    }

    // strtok divide a string em palavras mas a última será sempre null.Temos de "eliminar" o ultimo "valor" , decrementando a variável nArgumentos
    // printf("%s",argumentosComando[nArgumentos]); = (null)
    nArgumentos -= 1;

    // COMANDOS
    if(strcmp(argumentosComando[0], "users") == 0 && nArgumentos == 0){
        //printf("\nComando \"users\" executado com sucesso! [Mostrar lista dos utilizadores atualmente ativos]\n\n");
        printf("\n================================\n");
        printf("Lista de utilizadores ativos\n");
        for(int i=0;i<numero_Utilizadores;i++){
            printf("\nPID: %d | NOME: %s",listaUtilizadores[i].pid, listaUtilizadores[i].nome);
        }
        printf("\n================================\n");
    }

    else if(strcmp(argumentosComando[0], "promotores") == 0 && nArgumentos == 0){
        testePromotor();
    }

    else if(strcmp(argumentosComando[0], "list") == 0 && nArgumentos == 0){
        //printf("\nComando \"list\" executado com sucesso! [Mostrar lista de itens disponiveis]\n\n");
        printf("\n==============================================\n");
        printf("Lista de items no leilao\n");
        testeItemsAdmin();
        printf("\n==============================================\n");
    }
    else if(strcmp(argumentosComando[0], "kick") == 0 && nArgumentos == 1){
        //printf("\nComando \"kick\" executado com sucesso! [Kickar utilizador digitado]\n\n");
        bool expulso = false;

        for(int i=0;i<numero_Utilizadores;i++){
            if(strcmp(listaUtilizadores[i].nome, argumentosComando[1]) == 0){
                expulso = true;
            }
            if(expulso){
                char str_fifo_frontend[TAMANHO_STRING] = FIFO_FRONTEND;
                sprintf(str_fifo_frontend, FIFO_FRONTEND, listaUtilizadores[i].pid);

                int abrirFIFO_FRONTEND = open(str_fifo_frontend, O_WRONLY);

                printf("\nExpulsou o utilizador %s!\n",argumentosComando[1]);

                --numero_Utilizadores;

                listaUtilizadores[i] = listaUtilizadores[i+1];

                strcpy(respostaPedido.resposta, "Foi expulso pelo servidor!\n");
                respostaPedido.controlo = -100;
                write(abrirFIFO_FRONTEND, &respostaPedido, sizeof(Pedido));
            }
        }
        if(!expulso)
            printf("Utilizador com o nome %s nao esta ativo ou nao existe!\n",argumentosComando[1]);
    }
    else if(strcmp(argumentosComando[0], "prom") == 0 && nArgumentos == 0){
        //printf("\nComando \"prom\" executado com sucesso! [Mostrar lista de promotores altualmente ativos]\n\n");
        printf("\n===================================\n");
        printf("PROMOTORES ATIVOS\n");
        for(int k=0;k<nPromotores;k++){
            printf("\n%s PID: %d", arrayPromotores[k].nome, arrayPromotores[k].pid);
        }
        printf("\n===================================\n");
    }
    else if(strcmp(argumentosComando[0], "reprom") == 0 && nArgumentos == 0){
        //printf("\nComando \"reprom\" executado com sucesso! [Atualiza lista de promotores altualmente ativos]\n\n");

        // ============= ABRIR O FICHEIRO PROMOTORES E GUARDAR NOME DOS PROMOTORES EM UM ARRAY ===============
        FILE *filePromotores;
        filePromotores = fopen(getenv("FPROMOTERS"), "r");
        char auxPromotor[TAMANHO_STRING];
        char auxauxPromotor[TAMANHO_STRING];
        bool existe = true;

        if(filePromotores != NULL){
            while(!feof(filePromotores)){
                fscanf(filePromotores, "%s", auxPromotor);
                for(int i=0;i<nPromotores;i++){
                    strcpy(auxauxPromotor,auxPromotor);
                    if(arrayPromotores[i].nome != auxPromotor){
                        existe = false;
                    }
                }
                if(!existe){};
            }
        }
        else
            printf("\n\n[ERRO] Nao foi possivel abrir o ficheiro!\n");

        fclose(filePromotores);

    }
    else if(strcmp(argumentosComando[0], "cancel") == 0 && nArgumentos == 1){
        //printf("\nComando \"cancel\" executado com sucesso! [Cancela um promotor atualmente ativo]\n\n");
        for(int i=0;i<=nPromotores;i++){
            if(strcmp(arrayPromotores[i].nome, argumentosComando[1]) == 0){
                union sigval valor;
                valor.sival_int = 123;

                // TERMINA O PROGRAMA USANDO SIGUSR1
                sigqueue(arrayPromotores[i].pid, SIGUSR1, valor);

                printf("A promocao %s foi cancelada!\n", arrayPromotores[i].nome);
                --nPromotores;
            }
        }
    }
    else if(strcmp(argumentosComando[0], "close") == 0 && nArgumentos == 0){
        //printf("\nComando \"close\" executado com sucesso! [Fechar backend]\n\n");
        char str_fifo_frontend[20][TAMANHO_STRING];

        for(int i=0;i<numero_Utilizadores;i++){
            sprintf(str_fifo_frontend[i], FIFO_FRONTEND, listaUtilizadores[i].pid);
            int abrirFIFO_FRONTEND = open(str_fifo_frontend[i], O_WRONLY);

            strcpy(respostaPedido.resposta, "O servidor fechou!\n");
            respostaPedido.controlo = -2;
            write(abrirFIFO_FRONTEND, &respostaPedido, sizeof(Pedido));
        }
        printf("\nA sair...\n");
    }

    else
        printf("\nComando não encontrado ou executado sem sucesso!\n\n");
}

bool venderItem(char * nomeVendedorNovoItem, char * nomeNovoItem, char * categoriaNovoItem, int precoInicialNovoItem, int precoCompreJaNovoItem, int durancaoNovoItem){

    if((precoInicialNovoItem >= precoCompreJaNovoItem) && (precoCompreJaNovoItem != 0)){
        printf("\nValor 'Compre Ja' tem de ser maior que o valor inicial ou zero!\n");
        strcpy(respostaPedido.resposta, "Valor 'Compre Ja' tem de ser maior que o valor inicial ou zero!\n");

        return false;
    }

    int i = readItems();



    // PREENCHER DADOS NOVO ITEM
    listaItems[nItems].id = nItems+1;
    strcpy(listaItems[nItems].nomeItem, nomeNovoItem);
    strcpy(listaItems[nItems].nomeCategoria, categoriaNovoItem);
    listaItems[nItems].valInicial = precoInicialNovoItem;
    listaItems[nItems].valCompreJa = precoCompreJaNovoItem;
    listaItems[nItems].timeRestante = durancaoNovoItem;
    strcpy(listaItems[nItems].nomeVendedor,nomeVendedorNovoItem);
    strcpy(listaItems[nItems].nomeLicitador, "-");

    // GUARDAR NO FICHEIRO COM APPEND
    FILE *fileA;
    fileA = fopen(getenv("FITEMS"),"a");
    if(fileA != NULL){
        fprintf(fileA, "\n%d %s %s %d %d %d %s %s",listaItems[nItems].id, listaItems[nItems].nomeItem, listaItems[nItems].nomeCategoria, listaItems[nItems].valInicial, listaItems[nItems].valCompreJa, listaItems[nItems].timeRestante, listaItems[nItems].nomeVendedor, listaItems[nItems].nomeLicitador);
    }
    else
        printf("\n\n[ERRO] Nao foi possivel abrir o ficheiro!\n");

    fclose(fileA);



    int abrirFifoFRONTEND;              // ESCREVER PARA O UTILIZADOR Q ENVIOU O COMANDO
    char auxString[TAMANHO_STRING];
    sprintf(auxString, FIFO_FRONTEND, newPedido.pid);

    abrirFifoFRONTEND = open(auxString, O_WRONLY);

    strcpy(respostaPedido.resposta, "Item adiconado ao leilao com sucesso!\n");
    write(abrirFifoFRONTEND, &respostaPedido, sizeof(Pedido));

    for(int p=0;p<numero_Utilizadores;p++) {    // ESCREVER PARA TODOS
        sprintf(auxString, FIFO_FRONTEND, listaUtilizadores[p].pid);

        abrirFifoFRONTEND = open(auxString, O_WRONLY);

        sprintf(respostaPedido.resposta, "item[%d] foi adicionado ao leilao!\nNome: %s | Categoria: %s | Preco Base: %d | Preco Compre Ja: %d\n", i+1, listaItems[i].nomeItem, listaItems[i].nomeCategoria, listaItems[i].valInicial, listaItems[i].valCompreJa);
        write(abrirFifoFRONTEND,&respostaPedido,sizeof(Pedido));

    }

    ++nItems;
    ++i;

    return true;
}

void testeItemsAdmin(){
    int n = readItems();

    for(int i=0;i<n;i++){
        printf("\n%d %s %s %d %d %d %s %s", listaItems[i].id, listaItems[i].nomeItem, listaItems[i].nomeCategoria, listaItems[i].valInicial, listaItems[i].valCompreJa, listaItems[i].timeRestante, listaItems[i].nomeVendedor, listaItems[i].nomeLicitador);
    }


}

void testeItemsCategoria(char * categoria){
    int i = readItems();
    char string[TAMANHO_STRING];

    for(int j=0;j<i;j++){
        if(strcmp(categoria,listaItems[j].nomeCategoria) == 0){
            sprintf(string, "ArrayItems[%d]: %d %s %s %d %d %d %s %s\n", j, listaItems[j].id, listaItems[j].nomeItem, listaItems[j].nomeCategoria, listaItems[j].valInicial, listaItems[j].valCompreJa, listaItems[j].timeRestante, listaItems[j].nomeVendedor, listaItems[j].nomeLicitador);
            strcat(respostaPedido.resposta,string);
        }
    }

    if(strcmp(respostaPedido.resposta, "") == 0){
        strcpy(respostaPedido.resposta,"Nenhum item com a categoria digitada disponivel!\n");
    }
}

void testeItemsVendedor(char * nome){
    int i = readItems();
    char string[TAMANHO_STRING];

    for(int j=0;j<i;j++){
        if(strcmp(nome,listaItems[j].nomeVendedor) == 0){
            sprintf(string, "ArrayItems[%d]: %d %s %s %d %d %d %s %s\n", j, listaItems[j].id, listaItems[j].nomeItem, listaItems[j].nomeCategoria, listaItems[j].valInicial, listaItems[j].valCompreJa, listaItems[j].timeRestante, listaItems[j].nomeVendedor, listaItems[j].nomeLicitador);
            strcat(respostaPedido.resposta,string);
        }
    }

    if(strcmp(respostaPedido.resposta, "") == 0){
        strcpy(respostaPedido.resposta,"Este utilizador nao possui itens na lista de venda!\n");
    }
}

void testeItemsPrecoMax(int valor){
    int i = readItems();
    char string[TAMANHO_STRING];

    for(int j=0;j<i;j++){
        if(valor >= listaItems[j].valInicial){
            sprintf(string, "ArrayItems[%d]: %d %s %s %d %d %d %s %s\n", j, listaItems[j].id, listaItems[j].nomeItem, listaItems[j].nomeCategoria, listaItems[j].valInicial, listaItems[j].valCompreJa, listaItems[j].timeRestante, listaItems[j].nomeVendedor, listaItems[j].nomeLicitador);
            strcat(respostaPedido.resposta,string);
        }
    }

    if(strcmp(respostaPedido.resposta, "") == 0){
        strcpy(respostaPedido.resposta,"Nao existe nenhum item com o valor ate ao digitado!\n");
    }
}

void testeItemsTempoMax(int tempo){
    int i = readItems();
    char string[TAMANHO_STRING];

    for(int j=0;j<i;j++){
        if(tempo >= listaItems[j].timeRestante){
            sprintf(string, "ArrayItems[%d]: %d %s %s %d %d %d %s %s\n", j, listaItems[j].id, listaItems[j].nomeItem, listaItems[j].nomeCategoria, listaItems[j].valInicial, listaItems[j].valCompreJa, listaItems[j].timeRestante, listaItems[j].nomeVendedor, listaItems[j].nomeLicitador);
            strcat(respostaPedido.resposta,string);
        }
    }

    if(strcmp(respostaPedido.resposta, "") == 0){
        strcpy(respostaPedido.resposta,"Nao existe nenhum item com o valor ate ao digitado!\n");
    }
}

void mostrarSaldo(char * nome){
    char string[TAMANHO_STRING];
    int retorno = getUserBalance(nome);

    if(retorno != -1){
        sprintf(string, "Saldo: %d\n",retorno);
        strcpy(respostaPedido.resposta, string);
    }
    else{
        sprintf(string,"%s\n",getLastErrorText());
        strcpy(respostaPedido.resposta, string);
    }
}

void adicionarSaldo(char * nome, int valor){
    char string[TAMANHO_STRING];
    int retorno = updateUserBalance(nome, getUserBalance(nome) + valor);

    if(retorno != -1){
        saveUsersFile(getenv("FUSERS"));
        sprintf(string, "Saldo atualizado: %d\n",getUserBalance(nome));
        strcpy(respostaPedido.resposta, string);
    }
    else{
        sprintf(string,"%s\n",getLastErrorText());
        strcpy(respostaPedido.resposta, string);
    }
}

void licitarItem(char * nome, int id, int valor){
    int i = readItems();
    bool existe = false;
    bool apagou = false;
    int posItem = 0;


    for(int j=0;j<i;j++){
        if((id == listaItems[j].id) && (listaItems[j].valCompreJa != 0) && (valor >= listaItems[j].valCompreJa) && (strcmp(nome,listaItems[j].nomeVendedor) != 0) && (valor <= getUserBalance(nome))){     // COMPRAR NO MOMENTO
            strcpy(listaItems[j].nomeLicitador, nome);
            listaItems[j].valCompreJa = valor;

            posItem = j;

            updateUserBalance(nome, getUserBalance(nome) - listaItems[j].valCompreJa);
            saveUsersFile(getenv("FUSERS"));

            for(int p=0;p<numero_Utilizadores;p++){
                if(strcmp(listaUtilizadores[p].nome,listaItems[j].nomeVendedor) == 0){
                    updateUserBalance(listaUtilizadores[p].nome, getUserBalance(listaUtilizadores[p].nome) + listaItems[j].valCompreJa);
                    saveUsersFile(getenv("FUSERS"));
                }
            }

            apagou = true;
            --nItems;
            --i;
        }
        else if((id == listaItems[j].id) && (valor > listaItems[j].valInicial) && (strcmp(nome,listaItems[j].nomeVendedor) != 0) && (valor <= getUserBalance(nome))){     // LICITAÇÃO
            listaItems[j].valInicial = valor;
            strcpy(listaItems[j].nomeLicitador, nome);

            existe = true;
        }
    }

    if(apagou){
        for(int pp=0;pp<numero_Utilizadores;pp++) {     // ENVIAR INFO PARA TODOS OS USERS
            int abrirFifoFRONTEND;
            char auxString[TAMANHO_STRING];

            sprintf(auxString, FIFO_FRONTEND, listaUtilizadores[pp].pid);

            abrirFifoFRONTEND = open(auxString, O_WRONLY);

            sprintf(respostaPedido.resposta, "item[%d] foi vendido!\nNome: %s | Categoria: %s | Valor de venda: %d | Comprador: %s\n", listaItems[posItem].id, listaItems[posItem].nomeItem, listaItems[posItem].nomeCategoria, listaItems[posItem].valCompreJa, listaItems[posItem].nomeLicitador);
            write(abrirFifoFRONTEND, &respostaPedido, sizeof(Pedido));

        }
        for(int pp=0;pp<numero_Utilizadores;pp++) {     // ENVIAR INFO PARA O VENDEDOR
            if(strcmp(listaUtilizadores[pp].nome, listaItems[posItem].nomeVendedor) == 0){
                int abrirFifoFRONTEND;
                char auxString[TAMANHO_STRING];
                sprintf(auxString, FIFO_FRONTEND, listaUtilizadores[pp].pid);

                abrirFifoFRONTEND = open(auxString, O_WRONLY);

                sprintf(respostaPedido.resposta, "O seu item[%d]: %s foi vendido e recebeu +%d!\n", listaItems[posItem].id, listaItems[posItem].nomeItem, listaItems[posItem].valCompreJa);

                write(abrirFifoFRONTEND, &respostaPedido, sizeof(Pedido));
            }
        }
        for(int m=posItem;m<=nItems;m++){
            strcpy(listaItems[m].nomeItem, listaItems[m+1].nomeItem);
            strcpy(listaItems[m].nomeCategoria, listaItems[m+1].nomeCategoria);
            listaItems[m].valInicial = listaItems[m+1].valInicial;
            listaItems[m].valCompreJa = listaItems[m+1].valCompreJa;
            listaItems[m].timeRestante = listaItems[m+1].timeRestante;
            strcpy(listaItems[m].nomeVendedor, listaItems[m+1].nomeVendedor);
            strcpy(listaItems[m].nomeLicitador, listaItems[m+1].nomeLicitador);
        }
    }

    if(apagou){
        strcpy(respostaPedido.resposta, "Comprou um item com sucesso!\n");
    }
    else if(existe)
        strcpy(respostaPedido.resposta, "Licitou um item com sucesso!\n");
    else
        strcpy(respostaPedido.resposta, "Nao existe o item com o id digitado ou o valor digitado e' inferior  ao valor 'Comprar Ja' ou 'Valor de Licitacao' do item!\n[Tambem nao pode licitar no seu item]\n");


    saveItems();
}

void testeItems(){
    int i = readItems();
    char string[TAMANHO_STRING];

    for(int j=0;j<i;j++){
        sprintf(string, "ArrayItems[%d]: %d %s %s %d %d %d %s %s\n", j, listaItems[j].id, listaItems[j].nomeItem, listaItems[j].nomeCategoria, listaItems[j].valInicial, listaItems[j].valCompreJa, listaItems[j].timeRestante, listaItems[j].nomeVendedor, listaItems[j].nomeLicitador);
        strcat(respostaPedido.resposta,string);
    }

}

void testePromotor(){
    int resultadoFork;
    int resultadoPipe;
    int unamed_pipe_read_from_promotor[2];

    int i = 0;
    int len = 0;
    int nPromotores=0;

    char promocao[50][100];

    // ABRIR O FICHEIRO PROMOTORES E GUARDAR NOME DOS PROMOTORES EM UM ARRAY
    FILE *filePromotores;
    filePromotores = fopen(getenv("FPROMOTERS"), "r");

    if(filePromotores != NULL){
        while(!feof(filePromotores)){
            fscanf(filePromotores, "%s", arrayPromotores[nPromotores]);
            printf("%s\n",arrayPromotores[nPromotores]);
            nPromotores++;
        }
    }
    else
        printf("\n\n[ERRO] Nao foi possivel abrir o ficheiro!\n");

    fclose(filePromotores);


    resultadoPipe = pipe(unamed_pipe_read_from_promotor);
    if (resultadoPipe == -1){
        printf("Erro ao criar unnamed pipe!\n");
        exit(-1);
    }

    for(int k=0;k<nPromotores;k++){

        resultadoFork = fork();

        if(resultadoFork == -1){        // FILHO NAO FOI CRIADO COM SUCESSO
            printf("\n[ERRO] Criacao do processo filho nao foi possivel!\n\n");
            exit(-1);
        }
        else if(resultadoFork == 0)     // FILHO
        {

            // FECHAR STDOUT DO SISTEMA
            close(1);
            // COLOCAR STDOUT NO PIPE PARA RECEBER NO PAI
            dup(unamed_pipe_read_from_promotor[1]);
            // FECHAR PIPES
            close(unamed_pipe_read_from_promotor[1]);
            close(unamed_pipe_read_from_promotor[0]);

            // EXECUTAR PROMOTOR
            //execl("promotor_oficial","promotor_oficial",NULL);      // VOLTA PARA O PAI
            execl(arrayPromotores[k].nome,arrayPromotores[k].nome,NULL);

            printf("\n[ERRO] Executar promotor atraves do processo filho falhou!\n\n");
            exit(-1);
        }
        else if(resultadoFork > 0) {     // PAI
            close(unamed_pipe_read_from_promotor[1]);

            do {
                len = read(unamed_pipe_read_from_promotor[0], promocao[i], sizeof(promocao[i]));
                strtok(promocao[i], "\n");

                if (len > 0) {
                    printf("\nPROMOCAO: %s | PROMOTOR: %d\n", promocao[i], k);
                    i++;
                } else
                    printf("\n");
            } while (len > 0);    // PARA EVITAR QUE IMPRIMA VAR AMBIENTES ANTES DO FILHO MORRER

            wait(&resultadoFork);

            close(unamed_pipe_read_from_promotor[0]);
        }
    }
}

void menos1Saldo_MetaUm(char * nome){
    int saldoAtual;

    saldoAtual = getUserBalance(nome);
    if(saldoAtual == -1){
        return;
    }

    updateUserBalance(nome, saldoAtual - 1);

    printf("\n\nO utilizador \"%s\" perdeu 1 de saldo\nSALDO ATUALIZADO: %d!\n",nome, getUserBalance(nome));
}