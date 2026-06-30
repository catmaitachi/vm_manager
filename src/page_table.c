/**
 * @file page_table.c
 * @brief Tabela de páginas: mapeia páginas lógicas em quadros físicos e mantém
 *        os metadados do LRU aproximado (bit de referência e contador de aging).
 */

#include "page_table.h"
#include "config.h"

static page_table_entry_t page_table[PAGE_TABLE_SIZE];

/**
 * @brief Inicializa todas as entradas como inválidas (sem quadro associado) e
 *        zera os metadados de aging. Deve ser chamada uma vez na inicialização.
 */
void page_table_init(void)
{
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        page_table[i].frame = -1;
        page_table[i].valid = 0;
        page_table[i].reference_bit = 0;
        page_table[i].aging_counter = 0;
    }
}

/**
 * @brief Consulta a tradução de uma página.
 * @param page Número da página (0..PAGE_TABLE_SIZE-1).
 * @return O quadro físico se a página estiver válida (residente); -1 caso
 *         contrário (situação que caracteriza um page fault).
 */
int page_table_lookup(int page)
{
    if (page_table[page].valid) return page_table[page].frame;
    return -1;
}

/**
 * @brief Registra a tradução página->quadro e marca a entrada como válida.
 *        Zera o bit de referência e o contador de aging, pois a página acabou
 *        de ser carregada e inicia seu histórico de uso do zero.
 * @param page  Página a ser registrada.
 * @param frame Quadro físico onde a página foi carregada.
 */
void page_table_update(int page, int frame)
{
    page_table[page].frame = frame;
    page_table[page].valid = 1;
    page_table[page].reference_bit = 0;
    page_table[page].aging_counter = 0;
}

/**
 * @brief Invalida a entrada de uma página (usado quando ela é expulsa da
 *        memória física). Após isto, lookups da página retornam -1.
 * @param page Página a ser invalidada.
 */
void page_table_invalidate(int page)
{
    page_table[page].valid = 0;
    page_table[page].frame = -1;
}

/**
 * @brief Marca a página como referenciada (bit R = 1). Chamado a cada acesso;
 *        o bit é consumido na próxima atualização do aging.
 * @param page Página acessada.
 */
void page_table_set_reference(int page)
{
    page_table[page].reference_bit = 1;
}

/**
 * @brief Atualiza o contador de aging (algoritmo de envelhecimento / LRU
 *        aproximado) de todas as páginas válidas: desloca o contador 1 bit à
 *        direita, insere o bit de referência no bit mais significativo e zera
 *        o bit de referência. Páginas com menor contador são as menos usadas
 *        recentemente.
 */
void page_table_update_aging(void)
{
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {

        if (!page_table[i].valid) continue;

        page_table[i].aging_counter >>= 1;

        if (page_table[i].reference_bit) page_table[i].aging_counter |= 0x80;

        page_table[i].reference_bit = 0;

    }
}

/**
 * @brief Retorna o quadro associado a uma página, com checagem de limites.
 * @param page Página consultada.
 * @return O quadro, ou -1 se a página estiver fora da faixa válida.
 */
int page_table_get_frame(int page)
{
    if (page < 0 || page >= PAGE_TABLE_SIZE) {
        return -1;
    }

    return page_table[page].frame;
}

/**
 * @brief Indica se a página está válida (residente na memória física).
 * @param page Página consultada.
 * @return 1 se válida; 0 se inválida ou fora da faixa.
 */
int page_table_is_valid(int page)
{
    if (page < 0 || page >= PAGE_TABLE_SIZE) {
        return 0;
    }

    return page_table[page].valid;
}

/**
 * @brief Retorna o contador de aging de uma página (usado na escolha da vítima).
 * @param page Página consultada.
 * @return O contador de aging (8 bits), ou 0 se a página estiver fora da faixa.
 */
unsigned char page_table_get_aging_counter(int page)
{
    if (page < 0 || page >= PAGE_TABLE_SIZE) {
        return 0;
    }

    return page_table[page].aging_counter;
}
