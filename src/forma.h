#ifndef FORMA_H
#define FORMA_H

/**
 * @defgroup forma Modulo Forma
 * @{
 *
 * Forma geometrica generica armazenada na arvore Rubro-Negra.
 *
 * Tipos de forma suportados (itens da pescaria):
 *   - circulo  : peixe
 *   - retangulo: nau
 *   - linha    : camarao
 *   - texto    : lagosta, moeda ou algas, conforme o conteudo do texto
 *
 * O tipo eh acessado por forma_get_tipo. A estrutura real da forma
 * eh opaca (definida apenas no .c).
 */

/* Capacidades maximas de uma forma */
#define FORMA_MAX_COR    32    /**< tamanho maximo de uma string de cor */
#define FORMA_MAX_TEXTO  256   /**< tamanho maximo do texto de uma forma */

/**
 * @brief Tipos de forma geometrica existentes.
 */
typedef enum
{
    FORMA_CIRCULO = 0,  /**< circulo (peixe) */
    FORMA_RETANGULO,    /**< retangulo (nau) */
    FORMA_LINHA,        /**< linha (camarao) */
    FORMA_TEXTO         /**< texto (lagosta, moeda ou algas) */
} TipoForma;

/**
 * @brief Tipo opaco que representa uma forma geometrica.
 */
typedef struct Forma Forma;

/**
 * @brief Cria um circulo (peixe).
 *
 * @param id       Identificador unico da forma.
 * @param x        Coordenada x do centro.
 * @param y        Coordenada y do centro.
 * @param r        Raio do circulo.
 * @param corb     Cor da borda.
 * @param corp     Cor do preenchimento.
 * @return Ponteiro para a forma criada, ou NULL em falha.
 */
Forma *forma_cria_circulo(int id, double x, double y, double r,
                          const char *corb, const char *corp);

/**
 * @brief Cria um retangulo (nau).
 *
 * @param id  Identificador unico da forma.
 * @param x   Coordenada x do canto inferior esquerdo.
 * @param y   Coordenada y do canto inferior esquerdo.
 * @param w   Largura do retangulo.
 * @param h   Altura do retangulo.
 * @param corb Cor da borda.
 * @param corp Cor do preenchimento.
 * @return Ponteiro para a forma criada, ou NULL em falha.
 */
Forma *forma_cria_retangulo(int id, double x, double y, double w, double h,
                            const char *corb, const char *corp);

/**
 * @brief Cria uma linha (camarao).
 *
 * @param id  Identificador unico da forma.
 * @param x1  Coordenada x da primeira extremidade.
 * @param y1  Coordenada y da primeira extremidade.
 * @param x2  Coordenada x da segunda extremidade.
 * @param y2  Coordenada y da segunda extremidade.
 * @param cor Cor da linha.
 * @return Ponteiro para a forma criada, ou NULL em falha.
 */
Forma *forma_cria_linha(int id, double x1, double y1, double x2, double y2,
                        const char *cor);

/**
 * @brief Cria um texto (lagosta, moeda ou algas).
 *
 * @param id    Identificador unico da forma.
 * @param x     Coordenada x da ancora.
 * @param y     Coordenada y da ancora.
 * @param corb  Cor da borda.
 * @param corp  Cor do preenchimento.
 * @param a     Posicao da ancora: 'i' inicio, 'm' meio, 'f' fim.
 * @param texto Conteudo do texto (ate o fim da linha).
 * @return Ponteiro para a forma criada, ou NULL em falha.
 */
Forma *forma_cria_texto(int id, double x, double y,
                        const char *corb, const char *corp,
                        char a, const char *texto);

/**
 * @brief Libera a memoria de uma forma.
 *
 * @param forma Forma a ser destruida. Nao faz nada se NULL.
 */
void forma_destroi(Forma *forma);

/**
 * @brief Obtem o identificador da forma.
 */
int forma_get_id(const Forma *f);

/**
 * @brief Obtem o tipo da forma.
 */
TipoForma forma_get_tipo(const Forma *f);

/**
 * @brief Obtem a coordenada x da ancora.
 */
double forma_get_x(const Forma *f);

/**
 * @brief Obtem a coordenada y da ancora.
 */
double forma_get_y(const Forma *f);

/**
 * @brief Obtem a coordenada x2 (extremidade final de uma linha).
 */
double forma_get_x2(const Forma *f);

/**
 * @brief Obtem a coordenada y2 (extremidade final de uma linha).
 */
double forma_get_y2(const Forma *f);

/**
 * @brief Obtem a largura da forma (retangulo).
 */
double forma_get_largura(const Forma *f);

/**
 * @brief Obtem a altura da forma (retangulo).
 */
double forma_get_altura(const Forma *f);

/**
 * @brief Obtem o raio da forma (circulo).
 */
double forma_get_raio(const Forma *f);

/**
 * @brief Obtem a cor da borda.
 */
const char *forma_get_cor_borda(const Forma *f);

/**
 * @brief Obtem a cor do preenchimento.
 */
const char *forma_get_cor_preench(const Forma *f);

/**
 * @brief Obtem a ancora do texto ('i', 'm' ou 'f').
 */
char forma_get_ancora(const Forma *f);

/**
 * @brief Obtem o texto armazenado.
 */
const char *forma_get_texto(const Forma *f);

/**
 * @brief Define a coordenada x da ancora.
 */
void forma_set_x(Forma *f, double x);

/**
 * @brief Define a coordenada y da ancora.
 */
void forma_set_y(Forma *f, double y);

/**
 * @brief Obtem a energia da nau (retangulo).
 */
double forma_get_energia(const Forma *f);

/**
 * @brief Obtem a riqueza acumulada pela nau (retangulo).
 */
double forma_get_riqueza(const Forma *f);

/**
 * @brief Define o nivel de energia da nau.
 */
void forma_set_energia(Forma *f, double energia);

/**
 * @brief Soma um valor a riqueza da nau.
 */
void forma_add_riqueza(Forma *f, double valor);

/**
 * @brief Calcula a area da forma.
 *
 * Circulo: pi * r^2. Retangulo: w * h. Linha e texto: 0.
 */
double forma_area(const Forma *f);

/**
 * @brief Verifica se um ponto (x, y) esta dentro da forma.
 *
 * Usado para selecao por regiao e captura por rede.
 */
int forma_contem_ponto(const Forma *f, double x, double y);

/**
 * @brief Calcula o retangulo envolvente minimo (MBB) da forma.
 *
 * Retorna via ponteiros a caixa [x1, x2] x [y1, y2] que cobre a forma.
 * Esses valores servem para compor o mbb de um no da arvore Rubro-Negra.
 */
void forma_mbb(const Forma *f, double *x1, double *y1, double *x2, double *y2);

/**
 * @brief Classifica o conteudo de um texto.
 *
 * @return 'L' lagosta, 'M' moeda, 'A' alga/detrito, 'T' texto genérico.
 */
char forma_classifica_texto(const Forma *f);

/**
 * @brief Cria uma copia (clone) de uma forma.
 *
 * @return Nova forma com os mesmos dados, ou NULL em falha.
 */
Forma *forma_clona(const Forma *f);

/** @} */

#endif /* FORMA_H */