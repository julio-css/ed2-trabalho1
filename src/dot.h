#ifndef DOT_H
#define DOT_H

/**
 * @defgroup dot Modulo Dot (visualizacao da arvore)
 * @{
 *
 * Gera o arquivo .dot com a arvore Rubro-Negra final
 * (nos pintados de vermelho/preto), usando a exportacao do modulo
 * vermelha. O dot pode ser aberto no Graphviz.
 */

/**
 * @brief Gera o .dot da arvore final num arquivo.
 *
 * @param arvore   Arvore Rubro-Negra com as formas.
 * @param getid    Extrai o id de cada forma (rotulo dos nos).
 * @param arquivo  Caminho do arquivo .dot a escrever (arq.dot).
 */
void dot_exporta(void *arvore, int (*getid)(const void *elem), const char *arquivo);

/** @} */

#endif /* DOT_H */