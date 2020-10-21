#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "politicas.h"
#include "bcp.h"
#include "bcpList.h"

extern bcpList_t *bloqueados;
extern bcpList_t *prontos;
extern bcp_t* executando;

/*
 * Funções DUMMY são aquelas que não fazem nada... 
 * 
 * Existem políticas que não tomam ações em determinados pontos da simulação. Nestes casos
 * usa-se as rotinas DUMMY para não ter que tratar essas casos no loop de simulação.
 */

void DUMMY_tick(struct politica_t *p) {
    return;
}

void DUMMY_novo(struct politica_t *p, bcp_t* novoProcesso) {
    return;
}

void DUMMY_fim(struct politica_t *p, bcp_t* processoTerminado) {
    return;
}

void DUMMY_desbloqueado(struct politica_t* p, bcp_t* processoDesbloqueado) {
    return;
}

/*
 * Round-Robin
 * 
 * Os callbacks abaixo implementam a política round-robin para escalonamento de processos
 * 
 */

void RR_tick(struct politica_t *p) {
    if (executando) {
        //decrementar o tempo restante deste processo
        executando->timeSlice--;
        if (executando->timeSlice <= 0) {
            //se o tempo do processo acabou, inserir o processo atual na lista de prontos
            LISTA_BCP_inserir(prontos, executando);
            //remover o processo atual de execução
            executando = NULL;
        }
    }
}

void RR_novoProcesso(struct politica_t *p, bcp_t* novoProcesso) {
    //quando um novo processo chega, ele é inserido na fila round robin
    LISTA_BCP_inserir(p->param.rr->fifo, novoProcesso);
}

bcp_t* RR_escalonar(struct politica_t *p) {
    bcp_t* ret;
    int nBloqueados = 0;

    //Se não há processos na fila round-robin, retornar nenhum
    if (p->param.rr->fifo->tam == 0)
        return NULL;

    //testar todos os processos da fila round-robin a partir da posição atual
    while (nBloqueados < p->param.rr->fifo->tam) {

        //verificar é necessário apontar para o primeiro elemento novamente
        if (p->param.rr->pos >= p->param.rr->fifo->tam) {
            p->param.rr->pos = 0;
        }

        ret = p->param.rr->fifo->data[p->param.rr->pos];

        //verificar se o atual da fila round-robin está bloqueado
        if (LISTA_BCP_buscar(bloqueados, ret->pid) != LISTA_N_ENCONTRADO) {
            //Se estiver, testar o próximo! 
            nBloqueados++;
            ret = NULL;
        } else {
            //retornar o processo para ser executado!
            LISTA_BCP_remover(prontos, ret->pid);
            ret->timeSlice = p->param.rr->quantum;
            break;
        }

        p->param.rr->pos++;
    }

    p->param.rr->pos++;

    return ret;
}

void RR_fimProcesso(struct politica_t *p, bcp_t* processo) {
    //Quando um processo termina, removê-lo da fila round-robin
    LISTA_BCP_remover(p->param.rr->fifo, processo->pid);
}

/**
 * AQUI INSTANCIA RANDOM
 */
void RANDOM_novoProcesso(struct politica_t *p, bcp_t* novoProcesso) {
    //quando um novo processo chega, ele é inserido na fila round robin
    LISTA_BCP_inserir(p->param.random->fifo, novoProcesso);
}

bcp_t* RANDOM_escalonar(struct politica_t *p) {
    bcp_t* ret;
    int nBloqueados = 0;

    //Se não há processos na fila fcfs, retornar nenhum
    if (p->param.random->fifo->tam == 0)
        return NULL;

    //testar todos os processos da fila fcfs a partir da posição atual
    while (nBloqueados < p->param.random->fifo->tam) {
        srand((unsigned) time(NULL));
        ret = p->param.random->fifo->data[(rand() % p->param.random->fifo->tam)];
        //verificar se o atual da fila fcfs está bloqueado
        if (LISTA_BCP_buscar(bloqueados, ret->pid) != LISTA_N_ENCONTRADO) {
            //Se estiver, testar o próximo! 
            nBloqueados++;
            ret = NULL;
        } else {
            //retornar o processo para ser executado!
            LISTA_BCP_remover(prontos, ret->pid);
            break;
        }
    }
    return ret;
}

void RANDOM_fimProcesso(struct politica_t *p, bcp_t* processo) {
    //Quando um processo termina, removê-lo da fila random
    LISTA_BCP_remover(p->param.random->fifo, processo->pid);
}

/*
 * Aqui instancia FCFS
 */
void FCFS_novoProcesso(struct politica_t *p, bcp_t* novoProcesso) {
    //quando um novo processo chega, ele é inserido na fila round robin
    LISTA_BCP_inserir(p->param.fcfs->fifo, novoProcesso);
}

bcp_t* FCFS_escalonar(struct politica_t *p) {
    bcp_t* ret;
    int nBloqueados = 0;

    //Se não há processos na fila fcfs, retornar nenhum
    if (p->param.fcfs->fifo->tam == 0)
        return NULL;

    //testar todos os processos da fila fcfs a partir da posição atual
    while (nBloqueados < p->param.fcfs->fifo->tam) {
        ret = p->param.fcfs->fifo->data[nBloqueados];
        //verificar se o atual da fila fcfs está bloqueado
        if (LISTA_BCP_buscar(bloqueados, ret->pid) != LISTA_N_ENCONTRADO) {
            //Se estiver, testar o próximo! 
            nBloqueados++;
            ret = NULL;
        } else {
            //retornar o processo para ser executado!
            LISTA_BCP_remover(prontos, ret->pid);
            break;
        }
    }
    return ret;
}

void FCFS_fimProcesso(struct politica_t *p, bcp_t* processo) {
    //Quando um processo termina, removê-lo da fila fcfs
    LISTA_BCP_remover(p->param.fcfs->fifo, processo->pid);
}

void FCFS_desbloqueado(struct politica_t *p, bcp_t* processoBloq) {
    executando = NULL;
    FCFS_escalonar(p);
}

/*
 * Aqui instancia SJF
 */
void SJF_CALC_TIME_PROCESSO(bcp_t* novoProcesso) {
    novoProcesso->timeSlice = novoProcesso->eventos[novoProcesso->nEventos - 1]->tempo;
}

void SJF_novoProcesso(struct politica_t *p, bcp_t* novoProcesso) {
    //quando um novo processo chega, ele é inserido na fila round robin
    SJF_CALC_TIME_PROCESSO(novoProcesso);
    LISTA_BCP_inserir(p->param.sjf->fifo, novoProcesso);
}

bcp_t* SJF_escalonar(struct politica_t *p) {
    bcp_t* ret;
    int i, proc = 0, nBloqueados = 0;
    uint64_t tempo = 0, tempo_anterior = 999999999999999999;

    //Se não há processos na fila SJF, retornar nenhum
    if (p->param.sjf->fifo->tam == 0)
        return NULL;

    //testar todos os processos da fila fcfs a partir da posição atual
    for (i = 0; i < p->param.sjf->fifo->tam; i++) {
        ret = p->param.sjf->fifo->data[i];
        //verificar se o atual da fila fcfs está bloqueado
        if (LISTA_BCP_buscar(bloqueados, ret->pid) != LISTA_N_ENCONTRADO) {
            //Se estiver, testar o próximo! 
            nBloqueados++;
        } else {
            //retornar o processo para ser executado!
            tempo = p->param.sjf->fifo->data[i]->timeSlice - p->param.sjf->fifo->data[i]->tempoExecutado;
            if (tempo <= tempo_anterior) {
                tempo_anterior = tempo;
                proc = i;
            }
        }
    }

    if (nBloqueados == p->param.sjf->fifo->tam) {
        ret = NULL;
    } else {
        ret = p->param.sjf->fifo->data[proc];
        LISTA_BCP_remover(prontos, ret->pid);
    }
    return ret;
}

void SJF_fimProcesso(struct politica_t *p, bcp_t* processo) {
    //Quando um processo termina, removê-lo da fila fcfs
    LISTA_BCP_remover(p->param.fcfs->fifo, processo->pid);
}

void SJF_desbloqueado(struct politica_t *p, bcp_t* processoBloq) {
    executando = NULL;
    FCFS_escalonar(p);
}

/**
 * 
 * Aqui instancia FP
 */
void FP_novoProcesso(struct politica_t *p, bcp_t* novoProcesso) {
    if (p->param.fp->filas[novoProcesso->prioridade]->politica == POL_FCFS) {
        FCFS_novoProcesso(p->param.fp->filas[novoProcesso->prioridade], novoProcesso);
    } else if (p->param.fp->filas[novoProcesso->prioridade]->politica == POL_RANDOM) {
        RANDOM_novoProcesso(p->param.fp->filas[novoProcesso->prioridade], novoProcesso);
    } else if (p->param.fp->filas[novoProcesso->prioridade]->politica == POL_RR) {
        RR_novoProcesso(p->param.fp->filas[novoProcesso->prioridade], novoProcesso);
    } else if (p->param.fp->filas[novoProcesso->prioridade]->politica == POL_SJF) {
        SJF_novoProcesso(p->param.fp->filas[novoProcesso->prioridade], novoProcesso);
    }
}

void FP_tick(struct politica_t *p) {
    if (executando) {
        if (p->param.fp->filas[executando->prioridade]->politica == POL_RR) {
            RR_tick(p->param.fp->filas[executando->prioridade]);
        } else DUMMY_tick(p);
    }
}

void FP_fimProcesso(struct politica_t *p, bcp_t* processo) {
    if (p->param.fp->filas[processo->prioridade]->politica == POL_FCFS) {
        FCFS_fimProcesso(p->param.fp->filas[processo->prioridade], processo);
    } else if (p->param.fp->filas[processo->prioridade]->politica == POL_RANDOM) {
        RANDOM_fimProcesso(p->param.fp->filas[processo->prioridade], processo);
    } else if (p->param.fp->filas[processo->prioridade]->politica == POL_RR) {
        RR_fimProcesso(p->param.fp->filas[processo->prioridade], processo);
    } else if (p->param.fp->filas[processo->prioridade]->politica == POL_SJF) {
        SJF_fimProcesso(p->param.fp->filas[processo->prioridade], processo);
    }
}

void FP_desbloqueado(struct politica_t *p, bcp_t* processoBloq) {
    if (p->param.fp->filas[processoBloq->prioridade]->politica == POL_FCFS) {
        FCFS_desbloqueado(p->param.fp->filas[processoBloq->prioridade], processoBloq);
    } else if (p->param.fp->filas[processoBloq->prioridade]->politica == POL_SJF) {
        SJF_desbloqueado(p->param.fp->filas[processoBloq->prioridade], processoBloq);
    }
}

bcp_t* FP_escalonar(struct politica_t *p) {
    int i;
    bcp_t* ret;
    for (i = 0; i < p->param.fp->faixa_max; i++) {
        if (p->param.fp->filas[i]->politica == POL_FCFS) {
            if (p->param.fp->filas[i]->param.fcfs->fifo->tam > 0) {
                ret = FCFS_escalonar(p->param.fp->filas[i]);
                break;
            } else ret = NULL;
        }
        else if (p->param.fp->filas[i]->politica == POL_RANDOM) {
            if (p->param.fp->filas[i]->param.random->fifo->tam > 0) {
                ret = RANDOM_escalonar(p->param.fp->filas[i]);
                break;
            } else ret = NULL;
        }
        else if (p->param.fp->filas[i]->politica == POL_RR) {
            if (p->param.fp->filas[i]->param.rr->fifo->tam > 0) {
                ret = RR_escalonar(p->param.fp->filas[i]);
                break;
            } else ret = NULL;
        }
        else if (p->param.fp->filas[i]->politica == POL_SJF) {
            if (p->param.fp->filas[i]->param.sjf->fifo->tam > 0) {
                ret = SJF_escalonar(p->param.fp->filas[i]);
                break;
            } else ret = NULL;
        }
    }
    return ret;
}

/**
 * Aqui são criadas e intanciadas respectivas politicas: RANDOM, FSFS, SJF, RR, FP.
 * Uma função print e uma função Politica geral
 */
politica_t * POLITICARANDOM_criar() {
    politica_t* p;
    random_t* random;

    p = malloc(sizeof (politica_t));

    p->politica = POL_RANDOM;

    //Ligar os callbacks com as rotinas RANDOM
    p->escalonar = RANDOM_escalonar;
    p->tick = DUMMY_tick;
    p->novoProcesso = RANDOM_novoProcesso;
    p->fimProcesso = RANDOM_fimProcesso;
    p->desbloqueado = DUMMY_desbloqueado;

    //Alocar a struct que contém os parâmetros para a política RANDOM
    random = malloc(sizeof (random_t));

    //inicializar a estrutura de dados RANDOM
    random->fifo = LISTA_BCP_criar();

    //Atualizar a política com os parâmetros do escalonador
    p->param.random = random;

    return p;

}

politica_t * POLITICAFCFS_criar() {
    politica_t* p;
    fcfs_t* fcfs;

    p = malloc(sizeof (politica_t));

    p->politica = POL_FCFS;

    //Ligar os callbacks com as rotinas FCFS
    p->escalonar = FCFS_escalonar;
    p->tick = DUMMY_tick;
    p->novoProcesso = FCFS_novoProcesso;
    p->fimProcesso = FCFS_fimProcesso;
    p->desbloqueado = FCFS_desbloqueado;

    //Alocar a struct que contém os parâmetros para a política fcfs
    fcfs = malloc(sizeof (fcfs_t));

    //inicializar a estrutura de dados fcfs
    fcfs->fifo = LISTA_BCP_criar();

    //Atualizar a política com os parâmetros do escalonador
    p->param.fcfs = fcfs;

    return p;

}

politica_t * POLITICASJF_criar() {
    politica_t* p;
    sjf_t* sjf;

    p = malloc(sizeof (politica_t));

    p->politica = POL_SJF;

    //Ligar os callbacks com as rotinas SJF
    p->escalonar = SJF_escalonar;
    p->tick = DUMMY_tick;
    p->novoProcesso = SJF_novoProcesso;
    p->fimProcesso = SJF_fimProcesso;
    p->desbloqueado = SJF_desbloqueado;

    //Alocar a struct que contém os parâmetros para a política sjf
    sjf = malloc(sizeof (sjf_t));

    //inicializar a estrutura de dados sjf
    sjf->fifo = LISTA_BCP_criar();

    //Atualizar a política com os parâmetros do escalonador
    p->param.sjf = sjf;

    return p;

}

politica_t * POLITICARR_criar(char* quantum) {
    politica_t* p;
    rr_t* rr;

    p = malloc(sizeof (politica_t));

    p->politica = POL_RR;

    //Ligar os callbacks com as rotinas RR
    p->escalonar = RR_escalonar;
    p->tick = RR_tick;
    p->novoProcesso = RR_novoProcesso;
    p->fimProcesso = RR_fimProcesso;
    p->desbloqueado = DUMMY_desbloqueado;

    //Alocar a struct que contém os parâmetros para a política round-robin
    rr = malloc(sizeof (rr_t));

    //inicializar a estrutura de dados round-robin
    rr->quantum = atoi(quantum);
    rr->fifo = LISTA_BCP_criar();
    rr->pos = 0;

    //Atualizar a política com os parâmetros do escalonador
    p->param.rr = rr;

    return p;

}

politica_t * POLITICAFP_criar(FILE * arqProcessos) {
    politica_t* p = malloc(sizeof (politica_t));
    int i, tam;
    fp_t* fp = malloc(sizeof (fp_t));
    char* s = malloc(sizeof (char) * 10);
    char* tok;

    fgets(s, 10, arqProcessos);
    //retira \n
    size_t ln = strlen(s) - 1;
    if (s[ln] == '\n') s[ln] = '\0';

    tam = atoi(s);
    fp->filas = malloc(tam * sizeof (politica_t*));

    for (i = 0; i < tam; i++) {
        fgets(s, 10, arqProcessos);
        //retira \n
        ln = strlen(s) - 1;
        if (s[ln] == '\n') s[ln] = '\0';

        printf("string [%d] %s\n", i + 1, s);

        if (!strncmp(s, "sjf", 3)) {
            fp->filas[i] = POLITICASJF_criar();
        }
        if (!strncmp(s, "fcfs", 4)) {
            fp->filas[i] = POLITICAFCFS_criar();
        }
        if (!strncmp(s, "random", 6)) {
            fp->filas[i] = POLITICARANDOM_criar();
        }
        if (!strncmp(s, "rr", 2)) {
            tok = strtok(s, "()");
            tok = strtok(NULL, "()");
            fp->filas[i] = POLITICARR_criar(s);
        }
    }
    p->politica = POL_FP;
    //Ligar os callbacks com as rotinas FP
    p->escalonar = FP_escalonar;
    p->tick = FP_tick;
    p->novoProcesso = FP_novoProcesso;
    p->fimProcesso = FP_fimProcesso;
    p->desbloqueado = FP_desbloqueado;
    fp->faixa_max = tam;
    p->param.fp = fp;
    return p;

}

politica_t * POLITICA_criar(FILE * arqProcessos) {
    char* str;

    str = calloc(20, sizeof (char));

    fgets(str, 20, arqProcessos);

    politica_t* p;
    p = malloc(sizeof (politica_t));

    if (!strncmp(str, "sjf", 3)) {
        free(p);
        p = POLITICASJF_criar(arqProcessos);
    }

    if (!strncmp(str, "fcfs", 4)) {
        free(p);
        p = POLITICAFCFS_criar();
    }

    if (!strncmp(str, "random", 6)) {
        free(p);
        p = POLITICARANDOM_criar();
    }

    if (!strncmp(str, "rr", 2)) {
        free(p);
        p = POLITICARR_criar(fgets(str, 20, arqProcessos));
    }

    if (!strncmp(str, "fp", 2)) {
        free(p);
        p = POLITICAFP_criar(arqProcessos);
    }

    free(str);

    return p;
}

void POLITICA_imprimir(politica_t * politica) {
    const char* pol;

    if (politica->politica == POL_FCFS)
        pol = "FCFS";
    if (politica->politica == POL_FP)
        pol = "FP";
    if (politica->politica == POL_RANDOM)
        pol = "RANDOM";
    if (politica->politica == POL_RR)
        pol = "RR";
    if (politica->politica == POL_SJF)
        pol = "SJF";

    printf("política de escalonamento: %s\n", pol);
    if (politica->politica == POL_RR)
        printf("\tquantum: %d\n", politica->param.rr->quantum);

    return;
}