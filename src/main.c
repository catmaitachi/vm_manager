/**
 * @file main.c
 * @brief Ponto de entrada do simulador de memória virtual. Lê endereços lógicos
 *        da entrada padrão, traduz cada um (TLB -> tabela de páginas -> page
 *        fault), atualiza o aging e imprime o endereço físico e o byte lido.
 */

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "tlb.h"
#include "page_table.h"
#include "memory.h"
#include "statistics.h"

/**
 * @brief Laço principal da simulação.
 *
 * Abre o backing store, inicializa os módulos e, para cada endereço lógico:
 * extrai página e offset (16 bits), consulta o TLB e, em miss, a tabela de
 * páginas, tratando page fault quando necessário; em seguida marca a referência,
 * envelhece os contadores, calcula o endereço físico e imprime o byte. Ao final,
 * imprime as estatísticas.
 *
 * @return 0 em sucesso; 1 se o BACKING_STORE.bin não puder ser aberto.
 */
int main(void)
{
    FILE *backing = fopen(BACKING_STORE_PATH, "rb");

    if (backing == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", BACKING_STORE_PATH);
        fprintf(stderr, "Execute antes: cd data && python3 generate_data.py\n");
        return 1;
    }

    page_table_init();
    tlb_init();
    memory_init(backing);
    statistics_init();

    int logical_address;

    while (scanf("%d", &logical_address) == 1) {
        count_address();

        logical_address = logical_address & 0xFFFF;
        int page = (logical_address >> 8) & 0xFF;
        int offset = logical_address & 0xFF;

        int frame = tlb_lookup(page);

        if (frame != -1) {
            count_tlb_hit();
        } else {
            frame = page_table_lookup(page);

            if (frame == -1) {
                count_page_fault();
                frame = handle_page_fault(page);
            }

            tlb_insert(page, frame);
        }

        page_table_set_reference(page);
        page_table_update_aging();

        int physical_address = frame * PAGE_SIZE + offset;
        signed char value = read_memory(frame, offset);

        printf("Logical address: %d Physical address: %d Value: %d\n",
               logical_address,
               physical_address,
               value);
    }

    print_statistics();

    fclose(backing);

    return 0;
}
