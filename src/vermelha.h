#ifndef VERMELHA_H
#define VERMELHA_H

/** @defgroup vermelha Modulo Arvore Rubro-Negra
 *  @{
 *
 * Arvore Rubro-Negra recursiva que armazena as formas geometricas
 * (generico, operando sobre ponteiros void*).
 *
 * Diferenciais exigidos pelo enunciado:
 *   - Inserção, remoção e busca são recursivas.
 *   - Cada no armazena o MBB (menor retangulo envolvente) que cobre
 *     a info do proprio no e de toda a sua sub-arvore.
 *   - As buscas por regiao podam ramos que nao podem conter nos
 *     dentro da regiao de interesse, usando o MBB.
 *   - Apos cada rotacao/insercao/remocao o MBB dos nos e atualizado.
 *
 * A estrutura interna e opaca (definida apenas em vermelha.c).
 */

/* Cor de cada no da arvore Rubro-Negra */
typedef enum
{
    VERMELHO = 0, /**< no vermelho */
    PRETO    = 1  /**< no preto */
} CorNo;

/**
 * @brief Funcao de comparacao entre dois elementos.
 *
 * Deve retornar:
 *   < 0 se elem1 vem antes de elem2 (F1 < F2)
 *   0   se sao equivalentes
 *   > 0 se elem1 vem depois de elem2
 */
typedef int (*FuncaoComparacao)(const void *elem1, const void *elem2);

/**
 * @brief Funcao para obter o id de um elemento (formas).
 */
typedef int (*FuncaoGetId)(const void *elem);

/**
 * @brief Funcao para obter o MBB (v1, v2, v3, v4) de um elemento.
 *
 * v1, v2 : limite da caixa (x, y) do canto inferior esquerdo
 * v3, v4 : limite da caixa (x, y) do canto superior direito
 */
typedef void (*FuncaoGetMbb)(const void *elem,
                             double *v1, double *v2, double *v3, double *v4);

/**
 * @brief Funcao de visita aplicada a cada elemento numa travessia.
 */
typedef void (*FuncaoVisita)(void *dado, void *aux);

/**
 * @brief Funcao predicado de regiao (para busca com poda).
 *
 * Retorna 1 se o elemento deve entrar na regiao, 0 caso contrario.
 */
typedef int (*FuncaoPredicado)(const void *elem, void *ctx);

/* FABRICA */
/**
 * @brief Cria uma arvore Rubro-Negra vazia.
 *
 * @param comp    Funcao de comparacao (obrigatoria).
 * @param getmbb  Funcao que extrai o MBB de um elemento.
 * @return Ponteiro opaco para a arvore, ou NULL em falha.
 */
void *vermelha_cria(FuncaoComparacao comp, FuncaoGetMbb getmbb);

/**
 * @brief Destroi a arvore, liberando todos os nos.
 *
 * @param arvore Arvore Rubro-Negra.
 * @param destruir Funcao que libera os dados de cada no (pode ser NULL).
 */
void vermelha_destroi(void *arvore, void (*destruir)(void *));

/* INSERCAO */
/**
 * @brief Insere um elemento na arvore de forma recursiva,
 *        mantendo o balanceamento Rubro-Negro e atualizando o MBB.
 *
 * @param arvore  Arvore Rubro-Negra.
 * @param elemento Dado a ser inserido (forma).
 */
void vermelha_insere(void *arvore, void *elemento);

/* REMOCAO */
/**
 * @brief Remove um elemento da arvore de forma recursiva,
 *        pelo criterio de comparacao.
 *
 * @param arvore   Arvore Rubro-Negra.
 * @param chave    Chave/forma usada na busca (posicionada pela comparacao).
 * @return O dado removido (para liberar/contabilizar), ou NULL se ausente.
 */
void *vermelha_remove(void *arvore, const void *chave);

/**
 * @brief Remove um elemento pelo ID inteiro da forma.
 *
 * @param arvore Arvore Rubro-Negra.
 * @param id     Identificador da forma.
 * @return O dado removido, ou NULL se nao encontrado.
 */
void *vermelha_remove_por_id(void *arvore, int id, FuncaoGetId getid);

/* BUSCA */
/**
 * @brief Busca um elemento pela chave (comparacao).
 *
 * @return Ponteiro para o dado, ou NULL se ausente.
 */
void *vermelha_busca(void *arvore, const void *chave);

/**
 * @brief Busca um elemento pelo ID inteiro.
 *
 * @return Ponteiro para o dado, ou NULL se ausente.
 */
void *vermelha_busca_por_id(void *arvore, int id, FuncaoGetId getid);

/* TRAVESSIA */
/**
 * @brief Percorre a arvore em ordem simetrica (in-order).
 *
 * @param arvore  Arvore Rubro-Negra.
 * @param visita  Funcao de visita.
 * @param aux     Auxiliar passado para a funcao de visita.
 */
void vermelha_em_ordem(void *arvore, FuncaoVisita visita, void *aux);

/**
 * @brief Percorre os nos (infos) que possam intersectar a regiao.
 *
 * REALIZA PODA: usa o MBB de cada no para nao descer em ramos cujo
 * MBB nao intersecta a regiao dada (ctx).
 *
 * @param arvore  Arvore Rubro-Negra.
 * @param ctx     Regiao/contexto (passado ao predicado e ao teste de intersecao).
 * @param intersect FUNCAO que testa se um MBB intersecta a regiao.
 * @param visita  Funcao de visita (recebe o dado e o aux).
 * @param aux     Auxiliar passado para a visita.
 */
void vermelha_busca_regiao(void *arvore, void *ctx,
                           int (*intersect)(double, double, double, double, void *),
                           FuncaoVisita visita, void *aux);

/* INSPECAO */
/**
 * @brief Retorna a quantidade de nos da arvore.
 */
int vermelha_tamanho(void *arvore);

/**
 * @brief Retorna a altura (numero de nos) da arvore.
 */
int vermelha_altura(void *arvore);

/**
 * @brief Verifica (para testes) se a arvore respeita as propriedades
 *        Rubro-Negro: raiz preta, nos vermelhos com filhos pretos,
 *        mesma altura negra em todos os caminhos.
 *
 * @return 1 se valida, 0 caso contrario.
 */
int vermelha_valida(void *arvore);

/* EXPORTACAO DOT */
/**
 * @brief Gera um arquivo dot com o estado atual da arvore (nos pintados
 *        de vermelho/preto conforme a RB real).
 *
 * @param arvore         Arvore Rubro-Negra.
 * @param nome_arquivo   Caminho do arquivo .dot de saida.
 * @param getid          Funcao que extrai o id de cada no (opcional).
 */
void vermelha_exporta_dot(void *arvore, const char *nome_arquivo,
                          int (*getid)(const void *elem)/* opcional */);

/* ESTATISTICAS DA PODA (auxiliar para testes/demonstracao) */
/**
 * @brief Zera os contadores de nos visitados nas buscas por regiao.
 *
 * Usado para demonstrar que a poda por MBB visita menos nos.
 */
void vermelha_reset_estatisticas(void *arvore);

/**
 * @brief Quantos nos foram visitados na ultima busca por regiao
 *        (somente os que tiveram o MBB testado contra a regiao).
 */
int vermelha_get_intersecoes(void *arvore);

/** @} */

#endif /* VERMELHA_H */