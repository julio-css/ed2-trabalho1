#ifndef SVG_H
#define SVG_H

#include <stdio.h>
#include "forma.h"
#include "vermelha.h"

/** @defgroup svg Modulo SVG (desenho das formas)
 *  @{
 *
 * Gera o arquivo .svg inicial (arq.svg) com todas as formas da arvore.
 *
 * O SVG usa y crescendo para BAIXO. O plano cartesiano do trabalho
 * tem y crescendo para CIMA. A conversao e feita internamente
 * somando-se a margem (a forma eh "desenhada de cabeca para baixo"
 * em relacao ao plano usual, como descrito no enunciado).
 */

/**
 * @brief Calcula as dimensoes necessarias para o SVG (maior x e y),
 *        somando uma margem fixa em cada lado.
 *
 * @param arvore   Arvore com as formas (ou vazia).
 * @param largura  Ponteiro que recebe a largura calculada.
 * @param altura   Ponteiro que recebe a altura calculada.
 */
void svg_calcula_dimensoes(void *arvore, double *largura, double *altura);

/**
 * @brief Escreve o cabecalho do arquivo SVG.
 *
 * @param arq     Arquivo aberto para escrita.
 * @param largura Largura do canvas.
 * @param altura  Altura do canvas.
 */
void svg_abre(FILE *arq, double largura, double altura);

/**
 * @brief Escreve o fechamento do arquivo SVG.
 */
void svg_fecha(FILE *arq);

/**
 * @brief Desenha uma forma individual no arquivo SVG.
 *
 * Suporta circulo, retangulo, linha e texto.
 */
void svg_desenha_forma(FILE *arq, const Forma *f);

/**
 * @brief Desenha todas as formas da arvore no arquivo SVG (em-ordem).
 *
 * @param arq     Arquivo aberto.
 * @param arvore  Arvore Rubro-Negra com as formas.
 */
void svg_desenha_tudo(FILE *arq, void *arvore);

/**
 * @brief Desenha as formas restantes no SVG final, aplicando as cores
 *        de contorno das naus conforme o nivel final de energia.
 */
void svg_desenha_final(FILE *arq, void *arvore);

/** @} */

#endif /* SVG_H */