#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "arq_saida.h"
#include "erros.h"
#include "politicas.h"
#include "bcp.h"
#include "arq_processos.h"
#include "arq_experimento.h"

extern arq_saida_t* saida;
extern arq_processos_t* processos;
extern experimento_t* experimento;
extern uint64_t relogio;
extern uint64_t trocas_de_contexto;

void CALCULA_vazao() {
    float relogio_r = relogio;
    //CALCULO VAZAO
    saida->VAZAO = (1000 * processos->nProcessos) / relogio_r;
}

void CALCULA_tme_RANDOM() {
    int pid_fix = 0;
    int j, x, y, i;
    uint64_t T_bloqueio = 0;
    uint64_t T_exec = 0;
    uint64_t soma = 0;

    for (i = 0; i < processos->nProcessos; i++) {//PARA CALCULAR TODOS DE UM MESMO PID
        //GRAVA PID DO PROCESSO
        pid_fix = saida->TERMINO[i];
        //INCREMENTA ATE ACHAR O PRIMEIRO EVENTO = CRIAÇÃO
        y = 0;
        while (saida->evento[y]->PID != pid_fix) y++;
        //INCREMENTA ATE ACHAR O PRIMEIRO EVENTO EXECUCAO
        x = 0;
        while (saida->evento[x]->PID != pid_fix || saida->evento[x]->evento != EVT_EXEC) x++;
        //DESCONTA O TEMPO DE ESPERA DA CRIACAO ATE A PRIMEIRA EXECUCAO
        soma = soma + (saida->evento[x]->TTT - saida->evento[y]->TTT);

        //RESTANTE DO CALCULO
        for (x; x < saida->evento_n; x++) {//corre a lista toda com pid fixo até -1
            if (saida->evento[x]->PID == pid_fix && saida->evento[x]->evento == EVT_BLOQ) {// acha pid equivalente
                T_bloqueio = saida->evento[x]->TTT; //pega o tempo da execução anterior
                for (j = x; j < saida->evento_n; j++) {
                    if (saida->evento[j]->PID == pid_fix && saida->evento[j]->evento == EVT_EXEC) {
                        T_exec = saida->evento[j]->TTT; //pega o tempo da proxima execucao                       
                        break;
                    }
                }
                //faz a diferença e soma
                soma += T_exec - T_bloqueio;
                printf("PID: %d %"PRIu64" %"PRIu64" %"PRIu64"\n", pid_fix, T_bloqueio, T_exec, soma);
            }
        }
    }
    saida->TME = soma / processos->nProcessos; //divide pelo numero de processos
}

void CALCULA_tme_SJF() {
    int pid_fix = 0;
    int j, x, y, i;
    uint64_t T_bloqueio = 0;
    uint64_t T_exec = 0;
    uint64_t soma = 0;

    for (i = 0; i < processos->nProcessos; i++) {//PARA CALCULAR TODOS DE UM MESMO PID
        //GRAVA PID DO PROCESSO
        pid_fix = saida->TERMINO[i];
        //INCREMENTA ATE ACHAR O PRIMEIRO EVENTO = CRIAÇÃO
        y = 0;
        while (saida->evento[y]->PID != pid_fix) y++;
        //INCREMENTA ATE ACHAR O PRIMEIRO EVENTO EXECUCAO
        x = 0;
        while (saida->evento[x]->PID != pid_fix || saida->evento[x]->evento != EVT_EXEC) x++;
        //DESCONTA O TEMPO DE ESPERA DA CRIACAO ATE A PRIMEIRA EXECUCAO
        soma = soma + (saida->evento[x]->TTT - saida->evento[y]->TTT);

        //RESTANTE DO CALCULO
        for (x; x < saida->evento_n; x++) {//corre a lista toda com pid fixo até -1
            if (saida->evento[x]->PID == pid_fix && saida->evento[x]->evento == EVT_BLOQ) {// acha pid equivalente
                T_bloqueio = saida->evento[x]->TTT; //pega o tempo da execução anterior
                for (j = x; j < saida->evento_n; j++) {
                    if (saida->evento[j]->PID == pid_fix && saida->evento[j]->evento == EVT_EXEC) {
                        T_exec = saida->evento[j]->TTT; //pega o tempo da proxima execucao                       
                        break;
                    }
                }
                //faz a diferença e soma
                soma += T_exec - T_bloqueio;
                printf("PID: %d %"PRIu64" %"PRIu64" %"PRIu64"\n", pid_fix, T_bloqueio, T_exec, soma);
            }
        }
    }
    saida->TME = soma / processos->nProcessos; //divide pelo numero de processos
}

void CALCULA_tme_RR() {
    int pid_fix = 0;
    int j, x, y, i;
    uint64_t T_quantum = 0;
    uint64_t T_proxima_exec = 0;
    uint64_t soma = 0;

    for (i = 0; i < processos->nProcessos; i++) {//PARA CALCULAR TODOS DE UM MESMO PID
        //GRAVA PID DO PROCESSO
        pid_fix = saida->TERMINO[i];
        //INCREMENTA ATE ACHAR O PRIMEIRO EVENTO = CRIAÇÃO
        y = 0;
        while (saida->evento[y]->PID != pid_fix) y++;
        //INCREMENTA ATE ACHAR O PRIMEIRO EVENTO EXECUCAO
        x = 0;
        while (saida->evento[x]->PID != pid_fix || saida->evento[x]->evento != EVT_EXEC) x++;
        //DESCONTA O TEMPO DE ESPERA DA CRIACAO ATE A PRIMEIRA EXECUCAO
        soma = soma + (saida->evento[x]->TTT - saida->evento[y]->TTT);

        //RESTANTE DO CALCULO
        for (x; x < saida->evento_n - 2; x++) {//corre a lista toda com pid fixo até -1
            if (saida->evento[x]->PID == pid_fix && saida->evento[x]->evento == EVT_QUANTUM_EX) {// acha pid equivalente
                T_quantum = saida->evento[x]->TTT; //pega o tempo da execução anterior
                for (j = x + 1; j < saida->evento_n - 1; j++) {
                    if (saida->evento[j]->PID == pid_fix && saida->evento[j]->evento == EVT_EXEC) {
                        T_proxima_exec = saida->evento[j]->TTT; //pega o tempo da proxima execucao                       
                        break;
                    }
                }
                //faz a diferença e soma
                soma += (T_proxima_exec - (T_quantum /*+ experimento->politica->param.rr->quantum*/));
                //PRINT de DEBUG
                printf("PID: %d %"PRIu64" %"PRIu64" %"PRIu64"\n", pid_fix, T_quantum, T_proxima_exec, soma);
            }
        }
    }
    //soma += proxima_exec - (proxima_exec + experimento->politica->param.rr->quantum); //faz o ultimo caso
    saida->TME = soma / processos->nProcessos; //divide pelo numero de processos
}

void CALCULA_tme_FCFS() {
    int pid_fix = 0;
    int j, x, y, i;
    uint64_t ultima_exec = 0;
    uint64_t proxima_exec = 0;
    uint64_t soma = 0;

    for (i = 0; i < processos->nProcessos; i++) {//PARA CALCULAR TODOS DE UM MESMO PID
        //GRAVA PID DO PROCESSO
        pid_fix = saida->TERMINO[i];
        //INCREMENTA ATE ACHAR O PRIMEIRO EVENTO = CRIAÇÃO
        y = 0;
        while (saida->evento[y]->PID != pid_fix) y++;
        //INCREMENTA ATE ACHAR O PRIMEIRO EVENTO EXECUCAO
        x = 0;
        while (saida->evento[x]->PID != pid_fix || saida->evento[x]->evento != EVT_EXEC) x++;
        //DESCONTA O TEMPO DE ESPERA DA CRIACAO ATE A PRIMEIRA EXECUCAO
        soma = soma + (saida->evento[x]->TTT - saida->evento[y]->TTT);
    }
    saida->TME = soma / processos->nProcessos; //divide pelo numero de processos
}

void CALCULA_tme() {

    if (experimento->politica->politica == POL_RANDOM) CALCULA_tme_RANDOM();

    if (experimento->politica->politica == POL_FCFS) CALCULA_tme_FCFS();

    if (experimento->politica->politica == POL_RR) CALCULA_tme_RR();
    
    if (experimento->politica->politica == POL_SJF) CALCULA_tme_SJF();
}

void CALCULA_tmr() {
    int i;
    uint64_t soma = 0;
    for (i = 0; i < processos->nProcessos; i++) {
        soma += saida->tmr_processos[i]->tUltimaExec - saida->tmr_processos[i]->tPrimeiraExec;
    }
    soma = soma / processos->nProcessos;
    saida->TMR = soma;

}

arq_saida_t* CRIA_arq_saida(arq_processos_t* arq_processos) {
    arq_saida_t* novo;
    int i;

    novo = calloc(1, sizeof (arq_saida_t));
    novo->evento = calloc(1000, sizeof (diagrama_evento));
    novo->TERMINO = (int*) calloc((arq_processos->nProcessos), sizeof (int));
    novo->TME = 0;
    novo->TMR = 0;
    novo->VAZAO = 0.0;
    novo->tmr_processos = calloc(arq_processos->nProcessos, sizeof (TMR_processos));
    novo->alocacao_evento_n = 1000;
    novo->chaveamentos_n = 0;
    novo->evento_n = 0;
    novo->termino_n = 0;
    novo->tmr_n = 0;

    return novo;
}

void CRIA_ADC_evento(arq_saida_t* arq_saida, char* evento, uint64_t relogio, bcp_t* processo) {
    diagrama_evento* novo_diagrama = malloc(sizeof (diagrama_evento));

    novo_diagrama->TTT = relogio;
    novo_diagrama->PID = processo->pid;

    if (!strcmp(evento, "criacao"))
        novo_diagrama->evento = EVT_CRIACAO;

    else if (!strcmp(evento, "execucao")) {
        //GRAVA EVENTO EXEC
        novo_diagrama->evento = EVT_EXEC;

    } else if (!strcmp(evento, "termino")) {
        novo_diagrama->evento = EVT_TERMIN;

        //GRAVA EVENTO TERMINO
        arq_saida->TERMINO[arq_saida->termino_n] = processo->pid;
        arq_saida->termino_n++;

        //GRAVA TMR_PROCESSO para calculo TMR
        TMR_processos* tmr = malloc(sizeof (TMR_processos));
        tmr->PID = processo->pid;
        tmr->tPrimeiraExec = processo->tPrimeiraExec;
        tmr->tUltimaExec = processo->tUltimaExec;
        arq_saida->tmr_processos[arq_saida->tmr_n] = tmr;
        arq_saida->tmr_n++;

    } else if (!strcmp(evento, "quantum ex"))
        novo_diagrama->evento = EVT_QUANTUM_EX;

    else if (!strcmp(evento, "bloqueio"))
        novo_diagrama->evento = EVT_BLOQ;

    else if (!strcmp(evento, "desbloqueio"))
        novo_diagrama->evento = EVT_DESBLOQ;

    //verifica se o vetor evento ta cheio e realoca
    if (arq_saida->evento_n == arq_saida->alocacao_evento_n) {
        arq_saida->alocacao_evento_n += 500;
        arq_saida->evento = realloc(arq_saida->evento, arq_saida->alocacao_evento_n * sizeof (diagrama_evento*));
    }

    // insere no vetor
    arq_saida->evento[arq_saida->evento_n] = novo_diagrama;
    arq_saida->evento_n++;
}

void GRAVAR_saida(arq_saida_t* saida, char* arq_saida, int nProcessos) {
    FILE* arq;
    int i;

    //GRAVA NUMERO DE CHAVEAMENTOS
    saida->chaveamentos_n = trocas_de_contexto;
    //CALCULA VAZAO
    CALCULA_vazao();
    //CALCULA O TMR
    CALCULA_tmr();
    //CALCULA O TME
    CALCULA_tme();

    //retira \n da string do nome do arquivo de saida, apenas correção de erro
    size_t ln = strlen(arq_saida) - 1;
    if (arq_saida[ln] == '\n') arq_saida[ln] = '\0';

    arq = fopen(arq_saida, "w+");


    if (!arq) {
        fprintf(stderr, "Arquivo %s não encontrado!\n", arq_saida);
        exit(ARQ_PROC_N_ENCONTRADO);
    }
    //Aqui começa a gravar no arquivo
    fprintf(arq, "CHAVEAMENTOS: %d \n", saida->chaveamentos_n);
    fprintf(arq, "TME: %f \n", saida->TME);
    fprintf(arq, "TMR: %"PRIu64"\n", saida->TMR);
    fprintf(arq, "VAZÃO: %f \n", saida->VAZAO);
    fprintf(arq, "TERMINO: ");
    for (i = 0; i < nProcessos; i++) {
        fprintf(arq, "%d ", saida->TERMINO[i]);
    }

    fputs("\nDIAGRAMA DE EXECUÇÃO\n", arq);
    for (i = 0; i < saida->evento_n; i++) {
        fprintf(arq, "%"PRIu64" ", saida->evento[i]->TTT);
        fprintf(arq, "%d ", saida->evento[i]->PID);
        if (saida->evento[i]->evento == EVT_CRIACAO) fprintf(arq, "%s", "CRIAÇÃO\n");
        else if (saida->evento[i]->evento == EVT_EXEC) fprintf(arq, "%s", "EXEC\n");
        else if (saida->evento[i]->evento == EVT_TERMIN) fprintf(arq, "%s", "TERMINO\n");
        else if (saida->evento[i]->evento == EVT_QUANTUM_EX) fprintf(arq, "%s", "QUANTUM_EX\n");
        else if (saida->evento[i]->evento == EVT_BLOQ) fprintf(arq, "%s", "BLOQUEIO\n");
        else if (saida->evento[i]->evento == EVT_DESBLOQ) fprintf(arq, "%s", "DESBLOQUEIO\n");
    }

}