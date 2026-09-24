#include "unity.h"
#include "fila.h"

/* ==================================================================
 * Testes do modulo fila: criacao, ordem FIFO, tamanho e retirada de
 * fila vazia.
 * ==================================================================
 */

void setUp(void) {}
void tearDown(void) {}

void test_fila_vazia(void)
{
    Fila *f = fila_cria();
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_INT(0, fila_tamanho(f));
    TEST_ASSERT_NULL(fila_retira(f)); /* retirar de vazia retorna NULL */
    fila_destroi(f, NULL);
}

void test_fifo_mantem_ordem(void)
{
    Fila *f = fila_cria();
    int itens[3] = {7, 8, 9};

    for (int i = 0; i < 3; i++)
        fila_insere(f, &itens[i]);
    TEST_ASSERT_EQUAL_INT(3, fila_tamanho(f));

    TEST_ASSERT_EQUAL_PTR(&itens[0], fila_retira(f));
    TEST_ASSERT_EQUAL_PTR(&itens[1], fila_retira(f));
    TEST_ASSERT_EQUAL_PTR(&itens[2], fila_retira(f));
    TEST_ASSERT_EQUAL_INT(0, fila_tamanho(f));
    TEST_ASSERT_NULL(fila_retira(f));
    fila_destroi(f, NULL);
}

void test_insere_e_retira_alternados(void)
{
    Fila *f = fila_cria();
    int a = 1, b = 2, c = 3;

    fila_insere(f, &a);
    TEST_ASSERT_EQUAL_PTR(&a, fila_retira(f));

    fila_insere(f, &b);
    fila_insere(f, &c);
    TEST_ASSERT_EQUAL_PTR(&b, fila_retira(f));
    TEST_ASSERT_EQUAL_PTR(&c, fila_retira(f));
    fila_destroi(f, NULL);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_fila_vazia);
    RUN_TEST(test_fifo_mantem_ordem);
    RUN_TEST(test_insere_e_retira_alternados);
    return UNITY_END();
}
