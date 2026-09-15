#include "unity.h"
#include "vermelha.h"
#include "forma.h"

#include <stdio.h>
#include <string.h>

/* ==================================================================
 * Ajudantes de comparacao/MBB para a arvore (usam formas reais)
 * ==================================================================
 */

/**
 * comparadorPorId - ordena formas pelo id (ordem crescente).
 */
static int comparadorPorId(const void *a, const void *b)
{
    int ida = forma_get_id((const Forma *)a);
    int idb = forma_get_id((const Forma *)b);
    if (ida < idb)
        return -1;
    if (ida > idb)
        return 1;
    return 0;
}

/**
 * mbbDeForma - extrai o MBB de uma forma (callback da arvore).
 */
static void mbbDeForma(const void *e, double *x1, double *y1,
                       double *x2, double *y2)
{
    forma_mbb((const Forma *)e, x1, y1, x2, y2);
}

/**
 * reflexMbbTrue - funcao de intersecao que aceita tudo (sem poda).
 */
static int reflexMbbTrue(double x1, double y1, double x2, double y2, void *ctx)
{
    (void)x1; (void)y1; (void)x2; (void)y2; (void)ctx;
    return 1;
}

/**
 * mbbSemiSempreMenosMenor3 - bloqueta qualquer MBB com x < 0.
 *
 * Com isso, ramos cujo MBB tenha max-x negativo sao podados (id 3).
 */
static int mbbSemiSempreMenosMenor3(double x1, double y1, double x2, double y2, void *ctx)
{
    (void)x1; (void)y1; (void)y2; (void)ctx;
    /* libera somente MBBS cujo canto direito esta em x >= 0 */
    return x2 >= 0.0;
}

/**
 * visitaConta - funcao de visita que conta quantos nos recebeu.
 */
static void visitaConta(void *forma, void *aux)
{
    int *contador = (int *)aux;
    (void)forma;
    (*contador)++;
}

/**
 * visitaColetaId - funcao de visita que junta os ids numa lista.
 */
static void visitaColetaId(void *forma, void *aux)
{
    int *ids = (int *)aux;
    while (*ids != -1)
        ids++;
    *ids = forma_get_id((Forma *)forma);
}

/* ==================================================================
 * Infraestrutura do teste
 * ==================================================================
 */

static void *arvore;

void setUp(void)
{
    arvore = vermelha_cria(comparadorPorId, mbbDeForma);
    TEST_ASSERT_NOT_NULL(arvore);
}

void tearDown(void)
{
    vermelha_destroi(arvore, (void (*)(void *))forma_destroi);
    arvore = NULL;
}

/* ==================================================================
 * Testes
 * ==================================================================
 */

void test_cria_vazia(void)
{
    TEST_ASSERT_EQUAL_INT(0, vermelha_tamanho(arvore));
    TEST_ASSERT_EQUAL_INT(0, vermelha_altura(arvore));
    TEST_ASSERT_EQUAL_INT(1, vermelha_valida(arvore));
}

void test_insere_e_conta(void)
{
    vermelha_insere(arvore, forma_cria_circulo(1, 10, 20, 5, "red", "blue"));
    vermelha_insere(arvore, forma_cria_circulo(2, 30, 40, 5, "red", "blue"));
    TEST_ASSERT_EQUAL_INT(2, vermelha_tamanho(arvore));
    TEST_ASSERT_EQUAL_INT(1, vermelha_valida(arvore));
}

void test_insere_desordenado_ordena(void)
{
    vermelha_insere(arvore, forma_cria_circulo(5, 0, 0, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(2, 0, 0, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(8, 0, 0, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(3, 0, 0, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(1, 0, 0, 1, "k", "k"));

    TEST_ASSERT_EQUAL_INT(5, vermelha_tamanho(arvore));
    TEST_ASSERT_EQUAL_INT(1, vermelha_valida(arvore));
}

void test_insere_varios_valida_rb(void)
{
    int i;
    for (i = 0; i < 40; i++)
        vermelha_insere(arvore, forma_cria_circulo(i, i * 3, i * 7, 2, "k", "k"));

    TEST_ASSERT_EQUAL_INT(40, vermelha_tamanho(arvore));
    TEST_ASSERT_EQUAL_INT(1, vermelha_valida(arvore));
}

void test_busca_por_id(void)
{
    vermelha_insere(arvore, forma_cria_circulo(1, 10, 20, 5, "red", "blue"));
    vermelha_insere(arvore, forma_cria_circulo(2, 30, 40, 5, "green", "yellow"));

    Forma *f = (Forma *)vermelha_busca_por_id(arvore, 2, (FuncaoGetId)forma_get_id);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_INT(2, forma_get_id(f));

    TEST_ASSERT_NULL(vermelha_busca_por_id(arvore, 99, (FuncaoGetId)forma_get_id));
}

void test_em_ordem(void)
{
    vermelha_insere(arvore, forma_cria_circulo(5, 0, 0, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(2, 0, 0, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(8, 0, 0, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(3, 0, 0, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(1, 0, 0, 1, "k", "k"));

    /* id esperado em ordem: 1,2,3,5,8 */
    int ids[6];
    memset(ids, -1, sizeof(ids));
    vermelha_em_ordem(arvore, visitaColetaId, ids);
    TEST_ASSERT_EQUAL_INT_ARRAY(((int[]){1, 2, 3, 5, 8}), ids, 5);
}

void test_busca_regiao_conta_todos(void)
{
    vermelha_insere(arvore, forma_cria_circulo(1, 0, 0, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(2, 100, 100, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(3, -50, -50, 1, "k", "k"));

    /* sem poda (tudo intersecta): deve visitar os 3 */
    vermelha_reset_estatisticas(arvore);
    int contador = 0;
    vermelha_busca_regiao(arvore, NULL, reflexMbbTrue, visitaConta, &contador);
    TEST_ASSERT_EQUAL_INT(3, contador);
}

void test_busca_regiao_poda_por_mbb(void)
{
    /* formas bem espalhadas: o MBB da raiz cobre os 3 */
    vermelha_insere(arvore, forma_cria_circulo(1, 0, 0, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(2, 100, 100, 1, "k", "k"));
    vermelha_insere(arvore, forma_cria_circulo(3, -50, -50, 1, "k", "k"));

    /* a regiao que so pega ID 1 e 2 nao pode visitar id 3 */
    int contador = 0;
    vermelha_busca_regiao(arvore, NULL, mbbSemiSempreMenosMenor3, visitaConta, &contador);
    TEST_ASSERT_EQUAL_INT(2, contador);
}

void test_remove_por_id(void)
{
    vermelha_insere(arvore, forma_cria_circulo(1, 10, 20, 5, "red", "blue"));
    vermelha_insere(arvore, forma_cria_circulo(2, 30, 40, 5, "green", "yellow"));
    vermelha_insere(arvore, forma_cria_circulo(3, 50, 60, 5, "blue", "red"));

    Forma *removida = (Forma *)vermelha_remove_por_id(arvore, 2, (FuncaoGetId)forma_get_id);
    TEST_ASSERT_NOT_NULL(removida);
    TEST_ASSERT_EQUAL_INT(2, forma_get_id(removida));
    TEST_ASSERT_EQUAL_INT(2, vermelha_tamanho(arvore));
    TEST_ASSERT_EQUAL_INT(1, vermelha_valida(arvore));
    TEST_ASSERT_NULL(vermelha_busca_por_id(arvore, 2, (FuncaoGetId)forma_get_id));
    forma_destroi(removida);
}

void test_exporta_dot(void)
{
    vermelha_insere(arvore, forma_cria_circulo(1, 10, 20, 5, "red", "blue"));
    vermelha_insere(arvore, forma_cria_circulo(2, 30, 40, 5, "green", "yellow"));
    vermelha_insere(arvore, forma_cria_circulo(3, 50, 60, 5, "blue", "red"));

    const char *caminho = "teste_vermelha.dot";
    vermelha_exporta_dot(arvore, caminho, (FuncaoGetId)forma_get_id);

    FILE *fp = fopen(caminho, "r");
    TEST_ASSERT_NOT_NULL(fp);
    int temNos = 0;
    char linha[256];
    while (fgets(linha, sizeof(linha), fp) != NULL)
    {
        if (strstr(linha, "label=") != NULL)
            temNos++;
    }
    fclose(fp);
    TEST_ASSERT_EQUAL_INT(3, temNos);
    remove(caminho);
}

void test_valida_apos_muitas_operacoes(void)
{
    int i;
    for (i = 0; i < 30; i++)
        vermelha_insere(arvore, forma_cria_circulo(i, i, i, 1, "k", "k"));

    /* remove alguns */
    for (i = 0; i < 15; i++)
        forma_destroi((Forma *)vermelha_remove_por_id(arvore, i, (FuncaoGetId)forma_get_id));

    TEST_ASSERT_EQUAL_INT(15, vermelha_tamanho(arvore));
    TEST_ASSERT_EQUAL_INT(1, vermelha_valida(arvore));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_cria_vazia);
    RUN_TEST(test_insere_e_conta);
    RUN_TEST(test_insere_desordenado_ordena);
    RUN_TEST(test_insere_varios_valida_rb);
    RUN_TEST(test_busca_por_id);
    RUN_TEST(test_em_ordem);
    RUN_TEST(test_busca_regiao_conta_todos);
    RUN_TEST(test_busca_regiao_poda_por_mbb);
    RUN_TEST(test_remove_por_id);
    RUN_TEST(test_exporta_dot);
    RUN_TEST(test_valida_apos_muitas_operacoes);
    return UNITY_END();
}