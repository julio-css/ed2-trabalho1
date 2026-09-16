#include "txt.h"
#include "vermelha.h"

#include <stdio.h>

/**
 * visitaNau – escreve a linha de contabilidade quando a forma e uma nau.
 */
static void visitaNau(void *dado, void *aux)
{
    Forma *f = dado;
    FILE *arq = aux;

    if (f != NULL && forma_get_tipo(f) == FORMA_RETANGULO)
    {
        fprintf(arq, "nau %d: riqueza %.2f energia %.2f\n",
                forma_get_id(f), forma_get_riqueza(f), forma_get_energia(f));
    }
}

void txt_escreve_final(FILE *arq, void *arvore)
{
    if (arq == NULL || arvore == NULL)
        return;
    vermelha_em_ordem(arvore, visitaNau, arq);
}