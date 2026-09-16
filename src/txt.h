#ifndef TXT_H
#define TXT_H

#include <stdio.h>

/**
 * @defgroup txt Modulo TXT (contabilidade da pescaria)
 * @{
 *
 * Escreve o arquivo de saida .txt com a contabilidade final
 * das naus (riqueza e energia) ao fim do processamento do .qry.
 * Percorre a arvore final em ordem, olhando apenas os retangulos (naus).
 */

#include "forma.h"

/**
 * @brief Escreve a contabilidade final num arquivo.
 *
 * Para cada nau (retangulo) em ordem, escreve uma linha com seu id,
 * a riqueza acumulada e o nivel final de energia.
 *
 * @param arq    Arquivo aberto para escrita.
 * @param arvore Arvore Rubro-Negra com as formas restantes.
 */
void txt_escreve_final(FILE *arq, void *arvore);

/** @} */

#endif /* TXT_H */