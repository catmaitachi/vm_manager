/**
 * @file memory.c
 * @brief Memória física e backing store: mantém os quadros, carrega páginas sob
 *        demanda do BACKING_STORE.bin e implementa a substituição de páginas.
 */

#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "config.h"
#include "page_table.h"
#include "tlb.h"

static signed char physical_memory[NUM_FRAMES][FRAME_SIZE];

/*
 * Indica qual página está carregada em cada quadro.
 * Valor -1 indica quadro livre.
 */
static int frame_to_page[NUM_FRAMES];

static FILE *backing = NULL;

/**
 * @brief Inicializa a memória física: zera os quadros e marca todos como livres
 *        (frame_to_page = -1). Guarda o ponteiro do backing store já aberto.
 * @param backing_store Arquivo BACKING_STORE.bin aberto em modo binário.
 */
void memory_init(FILE *backing_store)
{
    backing = backing_store;

    for (int i = 0; i < NUM_FRAMES; i++) {
        frame_to_page[i] = -1;

        for (int j = 0; j < FRAME_SIZE; j++) {
            physical_memory[i][j] = 0;
        }
    }
}

/**
 * @brief Procura o primeiro quadro livre (busca linear).
 * @return O índice do quadro livre, ou -1 se a memória estiver cheia.
 */
static int find_free_frame(void)
{
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (frame_to_page[i] == -1) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Trata um page fault (paginação por demanda). Obtém um quadro livre ou,
 *        se a memória estiver cheia, escolhe uma vítima (LRU aproximado),
 *        invalida-a na tabela e no TLB e reaproveita seu quadro. Em seguida lê
 *        a página pedida do backing store para o quadro e atualiza a tabela.
 * @param page Página que causou a falta.
 * @return O quadro onde a página foi carregada.
 */
int handle_page_fault(int page)
{

    if (backing == NULL) {
        fprintf(stderr, "Erro interno: BACKING_STORE nao inicializado.\n");
        exit(1);
    }

    int frame = find_free_frame();

    if (frame == -1) {

        int victim_page = select_victim_page();
        frame = page_table_get_frame(victim_page);

        page_table_invalidate(victim_page);
        tlb_remove(victim_page);

    }

    fseek(backing, page * FRAME_SIZE, SEEK_SET);
    fread(physical_memory[frame], sizeof(signed char), FRAME_SIZE, backing);

    frame_to_page[frame] = page;
    page_table_update(page, frame);

    return frame;
}

/**
 * @brief Seleciona a página vítima para substituição (LRU aproximado): entre as
 *        páginas residentes, a de menor contador de aging. Em empate, vence a
 *        de menor índice de quadro (critério determinístico).
 * @return A página vítima, ou -1 se não houver páginas residentes.
 */
int select_victim_page(void)
{
    int victim = -1;
    unsigned char min_counter = 0;

    for (int frame = 0; frame < NUM_FRAMES; frame++) {

        int page = frame_to_page[frame];
        if (page == -1) continue;

        unsigned char counter = page_table_get_aging_counter(page);

        if (victim == -1 || counter < min_counter) {
            victim = page;
            min_counter = counter;
        }

    }

    return victim;

}

/**
 * @brief Lê o byte armazenado em uma posição da memória física.
 * @param frame  Quadro físico.
 * @param offset Deslocamento dentro do quadro (0..FRAME_SIZE-1).
 * @return O byte (signed char) naquela posição.
 */
signed char read_memory(int frame, int offset)
{
    return physical_memory[frame][offset];
}

/**
 * @brief Informa qual página está carregada em um quadro.
 * @param frame Quadro consultado.
 * @return A página residente, ou -1 se o quadro estiver livre ou fora da faixa.
 */
int get_page_loaded_in_frame(int frame)
{
    if (frame < 0 || frame >= NUM_FRAMES) {
        return -1;
    }

    return frame_to_page[frame];
}
