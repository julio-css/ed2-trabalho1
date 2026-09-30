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

/**
 * @brief Desenha um retangulo com bordas tracejadas (regiao da rede,
 *        regioes de origem/destino de mc).
 *
 * @param arq   Arquivo aberto.
 * @param x1,y1 Canto inferior esquerdo da regiao (plano do .geo).
 * @param x2,y2 Canto superior direito da regiao.
 */
void svg_desenha_regiao(FILE *arq, double x1, double y1,
                        double x2, double y2);

/**
 * @brief Desenha um marcador pontual no plano do .geo.
 *
 * @param tipo '*' asterisco (ponto de impacto);
 *             'x' cruz (nau destruida);
 *             'o' circulo amarelo (rede sem energia);
 *             'q' quadrado amarelo (canhao sem energia).
 */
void svg_desenha_marcador(FILE *arq, double x, double y, char tipo);

/**
 * @brief Desenha a trilha de um comando (mv/lr/d/mc).
 *
 * Segmento tracejado de (x1,y1) a (x2,y2), o rotulo centralizado e tres
 * aneis concentricos (r=1 vermelho, r=2 amarelo, r=3 vermelho) em cada uma
 * das duas pontas.
 *
 * @param rotulo     Texto sobre a trilha; NULL ou "" desenha sem rotulo.
 * @param tracejado  padrao do tracejado: o 'mv' usa "1", os demais "1.5".
 */
void svg_desenha_trilha(FILE *arq, double x1, double y1, double x2, double y2,
                        const char *rotulo, const char *tracejado);

/**
 * @brief Desenha a estrela de 12 pontas no ponto de impacto do canhao.
 */
void svg_desenha_estrela(FILE *arq, double x, double y);

/**
 * @brief Desenha a regiao da rede lancada (lr) como retangulo tracejado
 *        vermelho com preenchimento translucido.
 */
void svg_desenha_rede(FILE *arq, double x1, double y1, double x2, double y2);

/** @} */

#endif /* SVG_H */