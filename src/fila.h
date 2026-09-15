#ifndef FILA_H
#define FILA_H

/**
 * @defgroup fila Modulo Fila (processamento em ordem)
 * @{
 *
 * Fila encadeada (FIFO) usada para processar as consultas do .qry na
 * ordem em que aparecem. Guarda ponteiros genericos (void *),
 * nao copia os itens.
 */

/** Estrutura opaca da fila (definida apenas em fila.c). */
typedef struct Fila Fila;

/**
 * @brief Cria uma fila vazia.
 * @return Ponteiro para a fila, ou NULL se faltar memoria.
 */
Fila *fila_cria(void);

/**
 * @brief Destroi a fila e libera a memoria dos nos.
 *
 * @param f       Fila a destruir (aceita NULL).
 * @param liberta Libera cada item, se nao for NULL.
 */
void fila_destroi(Fila *f, void (*liberta)(void *));

/**
 * @brief Coloca um item no fim da fila (enfileirar).
 */
void fila_insere(Fila *f, void *item);

/**
 * @brief Retira o item da frente da fila (desenfileirar).
 *
 * @return O item removido, ou NULL se vazia.
 */
void *fila_retira(Fila *f);

/**
 * @brief Numero de itens guardados na fila.
 */
int fila_tamanho(const Fila *f);

/** @} */

#endif /* FILA_H */