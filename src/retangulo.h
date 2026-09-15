#ifndef RETANGULO_H
#define RETANGULO_H

#include "forma.h"

/**
 * @defgroup retangulo Modulo Retangulo (Nau)
 * @{
 *
 * Wrappers especificos de um retangulo, que representa uma nau.
 *
 * Alem dos dados geometricos base da forma, uma nau tem energia,
 * riqueza acumulada e lados/canhoes identificados por siglas:
 *   - PP : popa      (menor Y)
 *   - PR : proa      (maior Y)
 *   - EB : estibordo (menor X)
 *   - BB : bombordo  (maior X)
 * Os canhoes ficam nos pontos medios de cada lado.
 */

/**
 * @brief Cria um retangulo (nau).
 *
 * @see forma_cria_retangulo
 */
Forma *retangulo_cria(int id, double x, double y, double w, double h,
                      const char *corb, const char *corp);

/**
 * @brief Obtem a largura do retangulo.
 */
double retangulo_get_largura(const Forma *f);

/**
 * @brief Obtem a altura do retangulo.
 */
double retangulo_get_altura(const Forma *f);

/**
 * @brief Calcula a area do retangulo.
 */
double retangulo_area(const Forma *f);

/**
 * @brief Coordenada Y do lado PP (popa, menor Y).
 */
double retangulo_y_pp(const Forma *f);

/**
 * @brief Coordenada Y do lado PR (proa, maior Y).
 */
double retangulo_y_pr(const Forma *f);

/**
 * @brief Coordenada X do lado EB (estibordo, menor X).
 */
double retangulo_x_eb(const Forma *f);

/**
 * @brief Coordenada X do lado BB (bombordo, maior X).
 */
double retangulo_x_bb(const Forma *f);

/**
 * @brief Ponto medio do lado PP (canhao de popa).
 *
 * @param f   Retangulo/nau.
 * @param px  Saida: coordenada x do ponto medio.
 * @param py  Saida: coordenada y do ponto medio.
 */
void retangulo_canhao_pp(const Forma *f, double *px, double *py);

/**
 * @brief Ponto medio do lado PR (canhao de proa).
 */
void retangulo_canhao_pr(const Forma *f, double *px, double *py);

/**
 * @brief Ponto medio do lado EB (canhao de estibordo).
 */
void retangulo_canhao_eb(const Forma *f, double *px, double *py);

/**
 * @brief Ponto medio do lado BB (canhao de bombordo).
 */
void retangulo_canhao_bb(const Forma *f, double *px, double *py);

/**
 * @brief Verifica se um ponto esta dentro do retangulo.
 */
int retangulo_contem_ponto(const Forma *f, double x, double y);

/** @} */

#endif /* RETANGULO_H */