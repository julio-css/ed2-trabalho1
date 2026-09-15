#include "unity.h"
#include "geo.h"
#include "vermelha.h"
#include "forma.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ==================================================================
 * Testes do modulo geo: leitura do .geo e ordenacao F1 < F2
 * ==================================================================
 */

static void *arvore;

void setUp(void)
{
    arvore = vermelha_cria(geo_compara_forma, geo_get_mbb);
    TEST_ASSERT_NOT_NULL(arvore);
}

void tearDown(void)
{
    vermelha_destroi(arvore, (void (*)(void *))forma_destroi);
    arvore = NULL;
}

/**
 * escreveTmpGeo – grava um arquivo .geo temporario com o conteudo dado.
 */
static const char *escreveTmpGeo(const char *conteudo)
{
    const char *caminho = "tmp_geo_teste.geo";
    FILE *fp = fopen(caminho, "w");
    TEST_ASSERT_NOT_NULL(fp);
    fputs(conteudo, fp);
    fclose(fp);
    return caminho;
}

/**
 * coletaId – visitor em-ordem: preenche um vetor de ids.
 */
static void coletaId(void *f, void *aux)
{
    int *ids = aux;
    int i = 0;
    while (ids[i] != -1)
        i++;
    ids[i] = forma_get_id((Forma *)f);
}

/* ---------- ordenacao F1 < F2 ---------- */

void test_comparador_por_x(void)
{
    Forma *a = forma_cria_circulo(1, 10, 0, 1, "k", "k");
    Forma *b = forma_cria_circulo(2, 20, 0, 1, "k", "k");
    /* F1.X < F2.X => a < b */
    TEST_ASSERT_TRUE(geo_compara_forma(a, b) < 0);
    TEST_ASSERT_TRUE(geo_compara_forma(b, a) > 0);
    forma_destroi(a);
    forma_destroi(b);
}

void test_comparador_empate_x_area(void)
{
    Forma *a = forma_cria_circulo(1, 10, 0, 1, "k", "k");   /* area = pi */
    Forma *b = forma_cria_circulo(2, 10, 0, 2, "k", "k");   /* area = 4*pi */
    /* X iguais, AREA menor => a < b */
    TEST_ASSERT_TRUE(geo_compara_forma(a, b) < 0);
    TEST_ASSERT_TRUE(geo_compara_forma(b, a) > 0);
    forma_destroi(a);
    forma_destroi(b);
}

void test_comparador_empate_area_y(void)
{
    /* retangulos com mesma area (2x4 e 4x2) colocados no mesmo X */
    Forma *a = forma_cria_retangulo(1, 10, 5, 2, 4, "k", "k");   /* area=8, y=5 */
    Forma *b = forma_cria_retangulo(2, 10, 9, 4, 2, "k", "k");   /* area=8, y=9 */
    /* X e AREA iguais, Y menor => a < b */
    TEST_ASSERT_TRUE(geo_compara_forma(a, b) < 0);
    TEST_ASSERT_TRUE(geo_compara_forma(b, a) > 0);
    forma_destroi(a);
    forma_destroi(b);
}

void test_comparador_empate_total_id(void)
{
    Forma *a = forma_cria_circulo(1, 10, 5, 1, "k", "k");
    Forma *b = forma_cria_circulo(2, 10, 5, 1, "k", "k");
    TEST_ASSERT_TRUE(geo_compara_forma(a, b) < 0);   /* id 1 < 2 */
    TEST_ASSERT_TRUE(geo_compara_forma(b, a) > 0);
    forma_destroi(a);
    forma_destroi(b);
}

/* ---------- leitura do arquivo .geo ---------- */

void test_le_arquivo_basico(void)
{
    const char *arq = escreveTmpGeo(
        "c  1 100 50 10 red blue\n"
        "r  2 200 60 40 30 green yellow\n"
        "l  3 0 0 50 80 black\n");
    FILE *fp = fopen(arq, "r");
    TEST_ASSERT_NOT_NULL(fp);
    geo_processa_arquivo(fp, arvore);
    fclose(fp);

    TEST_ASSERT_EQUAL_INT(3, vermelha_tamanho(arvore));
    TEST_ASSERT_EQUAL_INT(1, vermelha_valida(arvore));

    /* todos os ids presentes */
    TEST_ASSERT_NOT_NULL(vermelha_busca_por_id(arvore, 1, geo_get_id));
    TEST_ASSERT_NOT_NULL(vermelha_busca_por_id(arvore, 2, geo_get_id));
    TEST_ASSERT_NOT_NULL(vermelha_busca_por_id(arvore, 3, geo_get_id));
}

void test_le_texto_com_espacos(void)
{
    const char *arq = escreveTmpGeo(
        "t  7 300 100 black white i >-|-<\n"
        "t  8 310 110 black white f >-|-< oi\n");
    FILE *fp = fopen(arq, "r");
    TEST_ASSERT_NOT_NULL(fp);
    geo_processa_arquivo(fp, arvore);
    fclose(fp);

    Forma *f7 = (Forma *)vermelha_busca_por_id(arvore, 7, geo_get_id);
    Forma *f8 = (Forma *)vermelha_busca_por_id(arvore, 8, geo_get_id);
    TEST_ASSERT_NOT_NULL(f7);
    TEST_ASSERT_NOT_NULL(f8);
    /* conteudo do texto: primeiro e ">-|-<", segundo e ">-|-< oi" */
    TEST_ASSERT_EQUAL_STRING(">-|-<", forma_get_texto(f7));
    TEST_ASSERT_EQUAL_STRING(">-|-< oi", forma_get_texto(f8));
}

void test_ignora_comentarios_e_ts(void)
{
    const char *arq = escreveTmpGeo(
        "# comentario que nao deve entrar\n"
        "ts Arial bold 24\n"
        "c  9 0 0 1 red blue\n");
    FILE *fp = fopen(arq, "r");
    TEST_ASSERT_NOT_NULL(fp);
    geo_processa_arquivo(fp, arvore);
    fclose(fp);

    TEST_ASSERT_EQUAL_INT(1, vermelha_tamanho(arvore));
    TEST_ASSERT_NOT_NULL(vermelha_busca_por_id(arvore, 9, geo_get_id));
}

void test_arvore_ordena_corretamente(void)
{
    /* ordem F1 < F2: X crescente. Vamos desenhar X fora de ordem no arquivo */
    const char *arq = escreveTmpGeo(
        "c  1 500 0 1 a b\n"     /* maior X */
        "c  2 100 0 1 a b\n"     /* menor X */
        "c  3 300 0 1 a b\n");
    FILE *fp = fopen(arq, "r");
    TEST_ASSERT_NOT_NULL(fp);
    geo_processa_arquivo(fp, arvore);
    fclose(fp);

    /* em-ordem deve visitar por X: 2, 3, 1 */
    int ids[4];
    memset(ids, -1, sizeof(ids));
    vermelha_em_ordem(arvore, coletaId, ids);
    TEST_ASSERT_EQUAL_INT_ARRAY(((int[]){2, 3, 1}), ids, 3);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_comparador_por_x);
    RUN_TEST(test_comparador_empate_x_area);
    RUN_TEST(test_comparador_empate_area_y);
    RUN_TEST(test_comparador_empate_total_id);
    RUN_TEST(test_le_arquivo_basico);
    RUN_TEST(test_le_texto_com_espacos);
    RUN_TEST(test_ignora_comentarios_e_ts);
    RUN_TEST(test_arvore_ordena_corretamente);
    return UNITY_END();
}