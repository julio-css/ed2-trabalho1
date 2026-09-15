#ifndef TEXTO_H
#define TEXTO_H

#include "forma.h"

/**
 * @defgroup texto Modulo Texto (Lagosta/Moeda/Algas)
 * @{
 *
 * Wrappers especificos de um texto.
 *
 * O conteudo do texto classifica o item:
 *   - ">-|-<"  : lagosta
 *   - "$"      : moeda
 *   - outros   : algas/detritos (sem valor)
 */

/**
 * @brief Cria um texto.
 *
 * @see forma_cria_texto
 */
Forma *texto_cria(int id, double x, double y,
                  const char *corb, const char *corp,
                  char a, const char *texto);

/**
 * @brief Obtem a posicao da ancora ('i', 'm' ou 'f').
 */
char texto_get_ancora(const Forma *f);

/**
 * @brief Obtem o conteudo do texto.
 */
const char *texto_get_string(const Forma *f);

/**
 * @brief Classifica o texto: 'L' lagosta, 'M' moeda, 'A' alga.
 */
char texto_classifica(const Forma *f);

/** @} */

#endif /* TEXTO_H */