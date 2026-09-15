#ifndef LINHA_H
#define LINHA_H

#include "forma.h"

/**
 * @defgroup linha Modulo Linha (Camarao)
 * @{
 *
 * Wrappers especificos de uma linha, que representa um camarao.
 *
 * A ancora da linha e a extremidade de menor X (menor Y em caso de
 * empate).
 */

/**
 * @brief Cria uma linha (camarao).
 *
 * @see forma_cria_linha
 */
Forma *linha_cria(int id, double x1, double y1, double x2, double y2,
                  const char *cor);

/**
 * @brief Obtem a coordenada x da extremidade oposta a ancora.
 */
double linha_get_x2(const Forma *f);

/**
 * @brief Obtem a coordenada y da extremidade oposta a ancora.
 */
double linha_get_y2(const Forma *f);

/**
 * @brief Calcula o comprimento da linha (distancia euclidiana).
 */
double linha_comprimento(const Forma *f);

/**
 * @brief Obtem a coordenada x da ancora.
 */
double linha_get_x(const Forma *f);

/**
 * @brief Obtem a coordenada y da ancora.
 */
double linha_get_y(const Forma *f);

/** @} */

#endif /* LINHA_H */