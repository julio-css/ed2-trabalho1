#include "unity.h"
#include "forma.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ==================================================================
 * Testes do modulo forma: fabricas, area, MBB, ponto-contido,
 * energia/riqueza, classificacao de texto e clonagem.
 * ==================================================================
 */

void setUp(void) {}
void tearDown(void) {}

void test_circulo_dados_e_area(void)
{
    Forma *f = forma_cria_circulo(1, 10.0, 20.0, 5.0, "red", "blue");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_INT(1, forma_get_id(f));
    TEST_ASSERT_EQUAL(FORMA_CIRCULO, forma_get_tipo(f));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 10.0, forma_get_x(f));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 20.0, forma_get_y(f));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 5.0, forma_get_raio(f));
    TEST_ASSERT_EQUAL_STRING("red", forma_get_cor_borda(f));
    TEST_ASSERT_EQUAL_STRING("blue", forma_get_cor_preench(f));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, M_PI * 25.0, forma_area(f));
    forma_destroi(f);
}

void test_retangulo_mbb_e_contem_ponto(void)
{
    Forma *f = forma_cria_retangulo(2, 0.0, 0.0, 30.0, 10.0, "k", "k");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 300.0, forma_area(f));

    double x1, y1, x2, y2;
    forma_mbb(f, &x1, &y1, &x2, &y2);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, x1);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, y1);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 30.0, x2);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 10.0, y2);

    TEST_ASSERT_TRUE(forma_contem_ponto(f, 15.0, 5.0));
    TEST_ASSERT_FALSE(forma_contem_ponto(f, 31.0, 5.0));
    forma_destroi(f);
}

void test_nau_energia_e_riqueza(void)
{
    Forma *f = forma_cria_retangulo(3, 0, 0, 10, 10, "k", "k");
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, forma_get_energia(f));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, forma_get_riqueza(f));

    forma_set_energia(f, 250.0);
    forma_add_riqueza(f, 20.5);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 250.0, forma_get_energia(f));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 20.5, forma_get_riqueza(f));
    forma_destroi(f);
}

void test_classifica_texto(void)
{
    Forma *lag = forma_cria_texto(1, 0, 0, "k", "k", 'i', ">-|-<");
    Forma *moeda = forma_cria_texto(2, 0, 0, "k", "k", 'i', "$");
    Forma *alga = forma_cria_texto(3, 0, 0, "k", "k", 'i', "algas");
    TEST_ASSERT_EQUAL('L', forma_classifica_texto(lag));
    TEST_ASSERT_EQUAL('M', forma_classifica_texto(moeda));
    TEST_ASSERT_EQUAL('A', forma_classifica_texto(alga));
    forma_destroi(lag);
    forma_destroi(moeda);
    forma_destroi(alga);
}

void test_clona_mantem_dados(void)
{
    Forma *f = forma_cria_circulo(9, 5.0, -5.0, 2.0, "a", "b");
    Forma *c = forma_clona(f);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_TRUE(f != c);
    TEST_ASSERT_EQUAL_INT(9, forma_get_id(c));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 5.0, forma_get_x(c));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, -5.0, forma_get_y(c));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 2.0, forma_get_raio(c));
    forma_destroi(f);
    forma_destroi(c);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_circulo_dados_e_area);
    RUN_TEST(test_retangulo_mbb_e_contem_ponto);
    RUN_TEST(test_nau_energia_e_riqueza);
    RUN_TEST(test_classifica_texto);
    RUN_TEST(test_clona_mantem_dados);
    return UNITY_END();
}
