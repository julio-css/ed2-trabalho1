#ifndef GEO_H
#define GEO_H

#include <stdio.h>
#include "vermelha.h"
#include "forma.h"

/** @defgroup geo Modulo Geo (leitura .geo)
 *  @{
 *
 * Carrega o arquivo .geo e insere as formas na arvore Rubro-Negra.
 *
 * Formato do .geo (cada linha e um comando):
 *   c  id x y r corb corp         circulo
 *   r  id x y w h corb corp       retangulo
 *   l  id x1 y1 x2 y2 cor         linha
 *   t  id x y corb corp a txto    texto (ate fim de linha)
 *   ts familia peso tamanho        estilo de texto (ignorado)
 *   #  ...                        comentario (ignorado)
 *
 * Ordenacao F1 < F2 (regulamento ED2):
 *   F1.X < F2.X,
 *   ou F1.X == F2.X e F1.AREA < F2.AREA,
 *   ou F1.X == F2.X e F1.AREA == F2.AREA e F1.Y < F2.Y.
 */

/**
 * @brief Le o arquivo .geo e insere todas as formas na arvore.
 *
 * @param arq_geo  Arquivo .geo aberto para leitura (ja posicionado).
 * @param arvore   Arvore Rubro-Negra (ja criada, ja com callbacks).
 */
void geo_processa_arquivo(FILE *arq_geo, void *arvore);

/**
 * @brief Comparador de formas segundo a regra ED2 (F1 < F2).
 *
 * Chave primaria: X crescente.
 * Chave secundaria: AREA crescente.
 * Chave terciaria: Y crescente.
 * Desempate final: ID (garante unicidade).
 *
 * Usa epsilon = 1e-9 para comparacao de ponto flutuante.
 */
int geo_compara_forma(const void *f1, const void *f2);

/**
 * @brief Extrai o ID de uma forma (callback FuncaoGetId).
 */
int geo_get_id(const void *elemento);

/**
 * @brief Extrai o MBB de uma forma (callback FuncaoGetMbb).
 */
void geo_get_mbb(const void *elemento,
                 double *x1, double *y1, double *x2, double *y2);

/** @} */

#endif /* GEO_H */
