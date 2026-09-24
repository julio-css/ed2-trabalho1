#ifndef QRY_H
#define QRY_H

#include <stdio.h>
#include "lista.h"

/**
 * @defgroup qry Modulo Qry (consultas da pescaria)
 * @{
 *
 * Le o arquivo .qry, enfileira os comandos e processa cada um em
 * ordem contra a arvore Rubro-Negra carregada do .geo. Escreve o
 * resultado textual de cada consulta num arquivo .txt e coleta
 * anotacoes de desenho (regioes tracejadas, marcadores) para serem
 * desenhadas no .svg final.
 *
 * Comandos suportados (enunciado ED2 - Trabalho I Pesca Pirata):
 *   e  i j v         energiza naus com id em [i,j] com o nivel v
 *   mv i dx dy       desloca a forma i de dx,dy (nau gasta energia)
 *   lr i lado d w h  lanca rede w x h a distancia d do lado da nau i
 *   d  i lado d      dispara canhao da nau i, alcance d
 *   mc dx dy x y w h translada peixes dentro da regiao (x,y,w,h)
 *
 * Energia: mv gasta sqrt(dx^2+dy^2)/5; rede gasta A/25 + d/5 (A=w*h);
 * canhao gasta d.
 *
 * As estruturas internas deste modulo sao opacas (definidas apenas
 * em qry.c), conforme a regra do enunciado.
 */

/**
 * @brief Processa o arquivo .qry contra a arvore.
 *
 * Le o .qry linha a linha, enfileira cada comando, depois processa os
 * comandos na ordem em que aparecem aplicando-os na arvore. Para cada
 * comando valido escreve no arquivo 'txt' a linha original precedida
 * de "[*] " e, em seguida, o resultado textual.
 *
 * @param arq_qry Arquivo .qry aberto para leitura (ja posicionado).
 * @param arvore  Arvore Rubro-Negra com as formas do .geo.
 * @param txt     Arquivo aberto para escrita do resultado (pode ser NULL).
 * @return Lista de anotacoes de desenho (item interno do modulo), vazia
 *         se nenhum comando foi processado, ou NULL se faltar memoria.
 */
Lista *qry_processa(FILE *arq_qry, void *arvore, FILE *txt);

/**
 * @brief Desenha as anotacoes coletadas no arquivo SVG.
 *
 * @param arq_svg   Arquivo SVG aberto (cabecalho <svg> ja escrito).
 * @param anotacoes Lista retornada por qry_processa (pode ser NULL).
 */
void qry_desenha_anotacoes(FILE *arq_svg, const Lista *anotacoes);

/**
 * @brief Libera a lista de anotacoes e seus itens.
 *
 * @param anotacoes Lista criada por qry_processa (pode ser NULL).
 */
void qry_libera_anotacoes(Lista *anotacoes);

/** @} */

#endif /* QRY_H */
