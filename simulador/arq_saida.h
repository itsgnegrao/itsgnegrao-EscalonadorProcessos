#ifndef ARQ_SAIDA_H
#define	ARQ_SAIDA_H
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "arq_saida.h"
#include "erros.h"
#include "politicas.h"
#include "bcp.h"
#include "arq_processos.h"

typedef struct TMR_processos {
    int PID;
    uint64_t tPrimeiraExec;
    uint64_t tUltimaExec;
} TMR_processos;

typedef enum EVENTO_SAIDA {
    EVT_CRIACAO,
    EVT_EXEC,
    EVT_TERMIN,
    EVT_QUANTUM_EX,
    EVT_BLOQ,
    EVT_DESBLOQ
} EVENTO_SAIDA;

typedef struct diagrama_evento {
    EVENTO_SAIDA evento;
    int PID;
    uint64_t TTT;
} diagrama_evento;

typedef struct arq_saida_t {
    TMR_processos** tmr_processos;
    diagrama_evento** evento;
    float TME;
    uint64_t TMR;
    float VAZAO;
    int *TERMINO;
    int evento_n;
    int alocacao_evento_n;
    int chaveamentos_n;
    int termino_n;
    int tmr_n;
} arq_saida_t;

arq_saida_t* CRIA_arq_saida(arq_processos_t* arq_processos);
void CRIA_ADC_evento(arq_saida_t* arq_saida, char* evento, uint64_t relogio, bcp_t* processo);
void GRAVAR_saida(arq_saida_t* saida, char* arq_saida, int nProcessos);
void CALCULA_tme();
void CALCULA_tmr();
void CALCULA_vazao();


#endif	/* ARQ_SAIDA_H */

