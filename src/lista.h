#ifndef LISTA_H
#define LISTA_H

/**
 * @defgroup lista Modulo Lista (colecao de itens)
 * @{
 *
 * Lista encadeada simples usada pelas consultas para reunir as formas
 * encontradas (por exemplo na busca por regiao) antes de processa-las.
 * Guarda ponteiros genericos (void *), nao copia os itens.
 */

/** Estrutura opaca da lista (definida apenas em lista.c). */
typedef struct Lista Lista;

/**
 * @brief Cria uma lista vazia.
 * @return Ponteiro para a lista, ou NULL se faltar memoria.
 */
Lista *lista_cria(void);

/**
 * @brief Destroi a lista e libera a memoria dos nos.
 *
 * @param l       Lista a destruir (aceita NULL).
 * @param liberta Libera cada item, se nao for NULL (ex.: forma_destroi).
 */
void lista_destroi(Lista *l, void (*liberta)(void *));

/**
 * @brief Insere um item no fim da lista (custante).
 */
void lista_insere(Lista *l, void *item);

/**
 * @brief Numero de itens guardados na lista.
 */
int lista_tamanho(const Lista *l);

/**
 * @brief Visita cada item, na ordem de insercao, passando 'aux'.
 *
 * @param l     Lista a percorrer.
 * @param visita Funcao chamada para cada item.
 * @param aux   Argumento extra repassado a 'visita'.
 */
void lista_percorre(const Lista *l, void (*visita)(void *item, void *aux), void *aux);

/** @} */

#endif /* LISTA_H */