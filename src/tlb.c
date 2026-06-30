/**
 * @file tlb.c
 * @brief Translation Lookaside Buffer: cache pequeno e rápido das traduções
 *        página->quadro mais recentes, com política de substituição FIFO.
 */

#include "tlb.h"
#include "config.h"

static tlb_entry_t tlb[TLB_SIZE];

/*
 * Índice da próxima posição a ser substituída.
 * Essa variável implementa FIFO no TLB.
 */
static int fifo_next = 0;

/**
 * @brief Inicializa o TLB com todas as entradas inválidas e reinicia o
 *        ponteiro FIFO. Deve ser chamada uma vez na inicialização.
 */
void tlb_init(void)
{
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].page = -1;
        tlb[i].frame = -1;
        tlb[i].valid = 0;
    }

    fifo_next = 0;
}

/**
 * @brief Procura uma página no TLB (busca linear).
 * @param page Página procurada.
 * @return O quadro em caso de acerto (TLB hit); -1 em caso de falha (TLB miss).
 */
int tlb_lookup(int page)
{
    for (int i = 0; i < TLB_SIZE; i++) {

        if (tlb[i].valid && tlb[i].page == page)
            return tlb[i].frame;

    }

    return -1;
}

/**
 * @brief Insere/atualiza uma tradução no TLB seguindo a prioridade: (1) se a
 *        página já existe, apenas atualiza o quadro; (2) se há entrada livre,
 *        usa-a; (3) se está cheio, substitui a entrada apontada por fifo_next
 *        (política FIFO) e avança o ponteiro de forma circular.
 * @param page  Página a registrar.
 * @param frame Quadro associado à página.
 */
void tlb_insert(int page, int frame)
{
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page == page) {
            tlb[i].frame = frame;
            return;
        }
    }

    for (int i = 0; i < TLB_SIZE; i++) {
        if (!tlb[i].valid) {
            tlb[i].page = page;
            tlb[i].frame = frame;
            tlb[i].valid = 1;
            return;
        }
    }

    tlb[fifo_next].page = page;
    tlb[fifo_next].frame = frame;
    tlb[fifo_next].valid = 1;
    fifo_next = (fifo_next + 1) % TLB_SIZE;
}

/**
 * @brief Invalida a entrada de uma página no TLB, se presente. Usado quando a
 *        página é expulsa da memória física, para o TLB não apontar para um
 *        quadro cujo conteúdo foi substituído.
 * @param page Página a remover do TLB.
 */
void tlb_remove(int page)
{
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page == page) {
            tlb[i].valid = 0;
            return;
        }
    }
}

/**
 * @brief Esvazia completamente o TLB (reinicializa todas as entradas).
 */
void tlb_clear(void)
{
    tlb_init();
}
