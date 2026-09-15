#include "fila.h"

#include <stdlib.h>

/**
 * No da fila encadeada.
 */
typedef struct NoFila
{
    void *item;           /* item guardado (consulta, forma, etc.) */
    struct NoFila *prox;  /* proximo no */
} NoFila;

/**
 * Fila com frente e tras (insere no fim, retira do inicio,
 * ambas em O(1)) e a quantidade de itens.
 */
struct Fila
{
    NoFila *frente;
    NoFila *tras;
    int tam;
};

Fila *fila_cria(void)
{
    Fila *f = malloc(sizeof *f);
    if (f == NULL)
        return NULL;
    f->frente = NULL;
    f->tras = NULL;
    f->tam = 0;
    return f;
}

void fila_destroi(Fila *f, void (*liberta)(void *))
{
    NoFila *n;
    if (f == NULL)
        return;
    for (n = f->frente; n != NULL;)
    {
        NoFila *prox = n->prox;
        if (liberta)
            liberta(n->item);
        free(n);
        n = prox;
    }
    free(f);
}

void fila_insere(Fila *f, void *item)
{
    NoFila *n;
    if (f == NULL)
        return;
    n = malloc(sizeof *n);
    if (n == NULL)
        return;
    n->item = item;
    n->prox = NULL;
    if (f->tras != NULL)
        f->tras->prox = n;
    else
        f->frente = n;
    f->tras = n;
    f->tam++;
}

void *fila_retira(Fila *f)
{
    NoFila *n;
    void *item;
    if (f == NULL || f->frente == NULL)
        return NULL;
    n = f->frente;
    item = n->item;
    f->frente = n->prox;
    if (f->frente == NULL)
        f->tras = NULL;
    free(n);
    f->tam--;
    return item;
}

int fila_tamanho(const Fila *f)
{
    return (f != NULL) ? f->tam : 0;
}