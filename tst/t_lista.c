#include "unity.h"
#include "lista.h"

/* ==================================================================
 * Testes do modulo lista: criacao, insercao, tamanho, percurso e
 * destruicao com liberacao de itens.
 * ==================================================================
 */

static int g_ordem[16];
static int g_n;
static int g_liberados;

static void coletaInt(void *item, void *aux)
{
    (void)aux;
    g_ordem[g_n++] = *(int *)item;
}

static void contaLibera(void *item)
{
    (void)item;
    g_liberados++;
}

void setUp(void) {}
void tearDown(void) {}

void test_cria_lista_vazia(void)
{
    Lista *l = lista_cria();
    TEST_ASSERT_NOT_NULL(l);
    TEST_ASSERT_EQUAL_INT(0, lista_tamanho(l));
    lista_destroi(l, NULL);
}

void test_insere_e_conta(void)
{
    Lista *l = lista_cria();
    int a = 1, b = 2, c = 3;
    lista_insere(l, &a);
    lista_insere(l, &b);
    lista_insere(l, &c);
    TEST_ASSERT_EQUAL_INT(3, lista_tamanho(l));
    lista_destroi(l, NULL);
}

void test_percorre_na_ordem_de_insercao(void)
{
    Lista *l = lista_cria();
    int itens[3] = {10, 20, 30};

    g_n = 0;
    for (int i = 0; i < 3; i++)
        lista_insere(l, &itens[i]);

    lista_percorre(l, coletaInt, NULL);
    TEST_ASSERT_EQUAL_INT(3, g_n);
    TEST_ASSERT_EQUAL_INT(10, g_ordem[0]);
    TEST_ASSERT_EQUAL_INT(20, g_ordem[1]);
    TEST_ASSERT_EQUAL_INT(30, g_ordem[2]);
    lista_destroi(l, NULL);
}

void test_destroi_libera_itens(void)
{
    Lista *l = lista_cria();
    int a = 1, b = 2;

    g_liberados = 0;
    lista_insere(l, &a);
    lista_insere(l, &b);
    TEST_ASSERT_EQUAL_INT(2, lista_tamanho(l));

    lista_destroi(l, contaLibera);
    TEST_ASSERT_EQUAL_INT(2, g_liberados);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_cria_lista_vazia);
    RUN_TEST(test_insere_e_conta);
    RUN_TEST(test_percorre_na_ordem_de_insercao);
    RUN_TEST(test_destroi_libera_itens);
    return UNITY_END();
}
