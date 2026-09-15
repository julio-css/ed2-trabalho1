#include "lista.h"

#include <stdlib.h>

/**
 * No da lista encadeada.
 */
typedef struct NoLista
{
    void *item;          /* item guardado (ponteiro para a forma) */
    struct NoLista *prox; /* proximo no */
} NoLista;

/**
 * Cabeca da lista: guarda o primeiro e o ultimo no (para insercao
 * no fim ser O(1)) e a quantidade de itens.
 */
struct Lista
{
    NoLista *cabeca;
    NoLista *cauda;
    int tam;
};

Lista *lista_cria(void)
{
    Lista *l = malloc(sizeof *l);
    if (l == NULL)
        return NULL;
    l->cabeca = NULL;
    l->cauda = NULL;
    l->tam = 0;
    return l;
}

void lista_destroi(Lista *l, void (*liberta)(void *))
{
    NoLista *n;
    if (l == NULL)
        return;
    for (n = l->cabeca; n != NULL;)
    {
        NoLista *prox = n->prox;
        if (liberta)
            liberta(n->item);
        free(n);
        n = prox;
    }
    free(l);
}

void lista_insere(Lista *l, void *item)
{
    NoLista *n;
    if (l == NULL)
        return;
    n = malloc(sizeof *n);
    if (n == NULL)
        return;
    n->item = item;
    n->prox = NULL;
    if (l->cauda != NULL)
        l->cauda->prox = n;
    else
        l->cabeca = n;
    l->cauda = n;
    l->tam++;
}

int lista_tamanho(const Lista *l)
{
    return (l != NULL) ? l->tam : 0;
}

void lista_percorre(const Lista *l, void (*visita)(void *item, void *aux), void *aux)
{
    NoLista *n;
    if (l == NULL)
        return;
    for (n = l->cabeca; n != NULL; n = n->prox)
        visita(n->item, aux);
}