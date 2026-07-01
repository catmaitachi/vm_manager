/**
 * @file statistics.c
 * @brief Contadores da simulação e cálculo/impressão das estatísticas finais
 *        (taxa de page fault e taxa de acerto do TLB).
 */

#include <stdio.h>

#include "statistics.h"

static int total_addresses = 0;
static int page_faults = 0;
static int tlb_hits = 0;

/**
 * @brief Zera todos os contadores. Deve ser chamada uma vez na inicialização.
 */
void statistics_init(void)
{
    total_addresses = 0;
    page_faults = 0;
    tlb_hits = 0;
}

/**
 * @brief Incrementa o total de endereços traduzidos (1 por endereço lido).
 */
void count_address(void)
{
    total_addresses++;
}

/**
 * @brief Incrementa o contador de faltas de página.
 */
void count_page_fault(void)
{
    page_faults++;
}

/**
 * @brief Incrementa o contador de acertos do TLB.
 */
void count_tlb_hit(void)
{
    tlb_hits++;
}

/** @brief Retorna o total de endereços traduzidos. */
int get_total_addresses(void)
{
    return total_addresses;
}

/** @brief Retorna o número de faltas de página. */
int get_page_faults(void)
{
    return page_faults;
}

/** @brief Retorna o número de acertos do TLB. */
int get_tlb_hits(void)
{
    return tlb_hits;
}

/**
 * @brief Calcula e imprime as estatísticas finais. As taxas são frações de
 *        total_addresses (com proteção contra divisão por zero) e seguem o
 *        formato de saída esperado pelo projeto.
 */
void print_statistics(void)
{
    double page_fault_rate = 0.0;
    double tlb_hit_rate = 0.0;

    if (total_addresses > 0) {
        page_fault_rate = (double) page_faults / total_addresses;
        tlb_hit_rate = (double) tlb_hits / total_addresses;
    }

    printf("Number of Translated Addresses = %d\n", total_addresses);
    printf("Page Faults = %d\n", page_faults);
    printf("Page Fault Rate = %.3f\n", page_fault_rate);
    printf("TLB Hits = %d\n", tlb_hits);
    printf("TLB Hit Rate = %.3f\n", tlb_hit_rate);
}

