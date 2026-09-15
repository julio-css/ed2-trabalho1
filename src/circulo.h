#ifndef CIRCULO_H
#define CIRCULO_H

#include "forma.h"

/**
 * @defgroup circulo Modulo Circulo (Peixe)
 * @{
 *
 * Wrappers especificos de um circulo, que representa um peixe.
 *
 * A ancora do circulo e o centro; o raio define o tamanho.
 */

/**
 * @brief Cria um circulo (peixe).
 *
 * @see forma_cria_circulo
 */
Forma *circulo_cria(int id, double x, double y, double r,
                    const char *corb, const char *corp);

/**
 * @brief Obtem o raio do circulo.
 */
double circulo_get_raio(const Forma *f);

/**
 * @brief Calcula a area do circulo (pi * r^2).
 */
double circulo_area(const Forma *f);

/**
 * @brief Verifica se um ponto esta dentro do circulo.
 */
int circulo_contem_ponto(const Forma *f, double x, double y);

/**
 * @brief Obtem a coordenada x do centro.
 */
double circulo_get_x(const Forma *f);

/**
 * @brief Obtem a coordenada y do centro.
 */
double circulo_get_y(const Forma *f);

/** @} */

#endif /* CIRCULO_H */