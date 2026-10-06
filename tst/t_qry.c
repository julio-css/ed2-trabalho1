#include "unity.h"
#include "geo.h"
#include "qry.h"
#include "vermelha.h"
#include "forma.h"
#include "lista.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================================================================
 * Testes do modulo qry: processamento dos comandos e, mv, lr, d e mc
 * sobre uma arvore carregada de um .geo temporario.
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

static void escreveTmp(const char *nome, const char *conteudo)
{
    FILE *fp = fopen(nome, "w");
    TEST_ASSERT_NOT_NULL(fp);
    fputs(conteudo, fp);
    fclose(fp);
}

static void carregaGeo(const char *conteudo)
{
    escreveTmp("tmp_qry_geo.geo", conteudo);
    FILE *fp = fopen("tmp_qry_geo.geo", "r");
    TEST_ASSERT_NOT_NULL(fp);
    geo_processa_arquivo(fp, arvore);
    fclose(fp);
}

static FILE *abreQry(const char *conteudo)
{
    escreveTmp("tmp_qry_cons.qry", conteudo);
    return fopen("tmp_qry_cons.qry", "r");
}

static Forma *busca(int id)
{
    return (Forma *)vermelha_busca_por_id(arvore, id, geo_get_id);
}

/* roda o .qry e escreve o resultado em tmp_qry_saida.txt */
static Lista *rodaQry(const char *conteudo)
{
    FILE *q = abreQry(conteudo);
    TEST_ASSERT_NOT_NULL(q);
    FILE *txt = fopen("tmp_qry_saida.txt", "w");
    TEST_ASSERT_NOT_NULL(txt);
    Lista *anot = qry_processa(q, arvore, txt);
    fclose(txt);
    fclose(q);
    return anot;
}

/* ---------- e: energizar naus no intervalo ---------- */

void test_e_energiza_intervalo(void)
{
    carregaGeo(
        "r 1 0 0 100 50 black white\n"
        "r 2 200 0 100 50 black white\n"
        "r 3 400 0 100 50 black white\n"
        "c 4 600 0 10 red blue\n");
    Lista *anot = rodaQry("e 2 3 250\n");
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, forma_get_energia(busca(1)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 250.0, forma_get_energia(busca(2)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 250.0, forma_get_energia(busca(3)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, forma_get_energia(busca(4)));
}

void test_e_intervalo_invertido(void)
{
    carregaGeo("r 1 0 0 50 50 black white\n");
    Lista *anot = rodaQry("e 2 1 100\n");
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, forma_get_energia(busca(1)));
}

/* ---------- mv: deslocar formas ---------- */

void test_mv_desloca_circulo_livre(void)
{
    carregaGeo(
        "r 1 0 0 100 50 black white\n"
        "c 2 50 50 10 red blue\n");
    Lista *anot = rodaQry("mv 2 15 5\n");
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 65.0, forma_get_x(busca(2)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 55.0, forma_get_y(busca(2)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, forma_get_x(busca(1)));
}

void test_mv_nau_gasta_energia_e_ordem_da_fila(void)
{
    carregaGeo("r 1 0 0 100 50 black white\n");
    /* 'e' vem antes de 'mv': a fila respeita a ordem do .qry */
    Lista *anot = rodaQry("e 1 1 100\nmv 1 10 0\n");
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    Forma *nau = busca(1);
    /* custo = sqrt(10^2 + 0^2)/5 = 2.0 -> energia 98 */
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 98.0, forma_get_energia(nau));
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 10.0, forma_get_x(nau));
    TEST_ASSERT_EQUAL_INT(1, vermelha_tamanho(arvore));
}

void test_mv_sem_energia_nao_move(void)
{
    carregaGeo("r 1 0 0 100 50 black white\n");
    Lista *anot = rodaQry("mv 1 10 0\n"); /* energia 0 < custo 2 */
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, forma_get_x(busca(1)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, forma_get_y(busca(1)));
}

/* ---------- lr: lancar rede ---------- */

void test_lr_captura_valores_e_remove(void)
{
    carregaGeo(
        "r 1 0 0 100 50 black white\n"
        "c 2 5 -5 3 red blue\n"            /* peixe na rede (M$5) */
        "t 3 10 -5 black white i $\n"      /* moeda (energia +2.5) */
        "t 4 15 -5 black white i >-|-<\n"  /* lagosta (M$20) */
        "t 5 18 -5 black white i alga\n"); /* alga (M$0) */
    Lista *anot = rodaQry("e 1 1 100\nlr 1 PP 10 20 20\n");
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    /* rede: a borda de lancamento fica a 'dist' da nau e a rede cresce de
       volta em direcao a ela -- x in [0,20], y in [-10,10]. Captura peixe,
       moeda, lagosta e alga. Nau 1 permanece. */
    TEST_ASSERT_EQUAL_INT(1, vermelha_tamanho(arvore));
    Forma *nau = busca(1);
    TEST_ASSERT_NOT_NULL(nau);
    /* custo = (20*20)/25 * (10/5) = 16 * 2 = 32; a moeda vale M$0 e nao
       credita energia, como no gabarito: energia = 100 - 32 = 68 */
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 68.0, forma_get_energia(nau));
    /* riqueza = 20 (lagosta) + 5 (peixe) = 25 */
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 25.0, forma_get_riqueza(nau));
    TEST_ASSERT_NULL(busca(2));
    TEST_ASSERT_NULL(busca(3));
    TEST_ASSERT_NULL(busca(4));
    TEST_ASSERT_NULL(busca(5));
}

void test_lr_sem_energia_lanca_com_energia_negativa(void)
{
    carregaGeo(
        "r 1 0 0 100 50 black white\n"
        "c 2 5 -20 3 red blue\n");
    /* o gabarito lanca a rede sem energia: os rotulos "lr (588.75,784.00)" e
     * "lr (-195.25,882.00)" existem nos arquivos oficiais. */
    Lista *anot = rodaQry("lr 1 PP 10 20 20\n"); /* energia 0, custo 32 */
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    TEST_ASSERT_EQUAL_INT(2, vermelha_tamanho(arvore)); /* nada capturado */
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, -32.0, forma_get_energia(busca(1)));
}

/* ---------- d: canhao ---------- */

void test_d_destroi_alvo_e_captura_riqueza(void)
{
    carregaGeo(
        "r 1 0 0 50 50 black white\n"
        "r 2 10 60 30 30 black white\n");
    Forma *alvo = busca(2);
    forma_add_riqueza(alvo, 40.0);

    /* canhao PR da nau 1: de (25,50) subindo 60 -> atinge nau 2 (y=60) */
    Lista *anot = rodaQry("e 1 1 100\nd 1 PR 60\n");
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    TEST_ASSERT_EQUAL_INT(1, vermelha_tamanho(arvore));
    Forma *nau = busca(1);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 40.0, forma_get_energia(nau)); /* 100-60 */
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 40.0, forma_get_riqueza(nau));
    TEST_ASSERT_NULL(busca(2));
}

void test_d_energia_exata_dispara(void)
{
    carregaGeo(
        "r 1 0 0 50 50 black white\n"
        "r 2 10 60 30 30 black white\n");
    /* energia 60 == custo 60: deve disparar normalmente */
    Lista *anot = rodaQry("e 1 1 60\nd 1 PR 60\n");
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    Forma *nau = busca(1);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, forma_get_energia(nau));
    TEST_ASSERT_EQUAL_INT(1, vermelha_tamanho(arvore));
}

void test_d_sem_alvo_so_gasta_energia(void)
{
    carregaGeo("r 1 0 0 50 50 black white\n");
    Lista *anot = rodaQry("e 1 1 100\nd 1 PR 60\n");
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    Forma *nau = busca(1);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 40.0, forma_get_energia(nau));
    TEST_ASSERT_EQUAL_INT(1, vermelha_tamanho(arvore));
}

/* ---------- mc: transladar peixes ---------- */

void test_mc_translada_so_circulos_da_regiao(void)
{
    carregaGeo(
        "c 1 5 5 2 red blue\n"
        "c 2 15 10 2 red blue\n"
        "c 3 50 50 2 red blue\n"       /* fora da regiao (0..20, 0..20) */
        "r 4 5 5 10 10 black white\n");/* retangulo dentro: nao e peixe */
    Lista *anot = rodaQry("mc 10 5 0 0 20 20\n");
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 15.0, forma_get_x(busca(1)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 10.0, forma_get_y(busca(1)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 25.0, forma_get_x(busca(2)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 15.0, forma_get_y(busca(2)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 50.0, forma_get_x(busca(3)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 50.0, forma_get_y(busca(3)));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 5.0, forma_get_x(busca(4)));
}

/* ---------- robustez do arquivo .qry ---------- */

void test_qry_vazio_retorna_lista_vazia(void)
{
    carregaGeo("r 1 0 0 50 50 black white\n");
    Lista *anot = rodaQry("# so comentario\n\n");
    TEST_ASSERT_NOT_NULL(anot);
    TEST_ASSERT_EQUAL_INT(0, lista_tamanho(anot));
    qry_libera_anotacoes(anot);
}

void test_linha_invalida_ignorada(void)
{
    carregaGeo("r 1 0 0 50 50 black white\n");
    Lista *anot = rodaQry("zzz 1 2\nmv 1 10 0\n"); /* 'zzz' desconhecido */
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    /* nau sem energia nao foi movida; nada travou */
    TEST_ASSERT_EQUAL_INT(1, vermelha_tamanho(arvore));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, forma_get_x(busca(1)));
}

void test_txt_registra_consultas(void)
{
    carregaGeo("r 1 0 0 50 50 black white\n");
    Lista *anot = rodaQry("e 1 1 30\n");
    if (anot != NULL)
        qry_libera_anotacoes(anot);

    FILE *rd = fopen("tmp_qry_saida.txt", "r");
    TEST_ASSERT_NOT_NULL(rd);
    char linha[256];
    TEST_ASSERT_NOT_NULL(fgets(linha, sizeof(linha), rd));
    TEST_ASSERT_EQUAL_STRING("[*] e 1 1 30\n", linha);
    fclose(rd);
}

void test_anotacoes_geradas(void)
{
    carregaGeo("r 1 0 0 50 50 black white\n");
    /* o gabarito dispara mesmo sem energia e nao marca tiro sem carga:
     * a trilha e a estrela do impacto, 2 anotacoes. */
    Lista *anot = rodaQry("d 1 PR 60\n");
    TEST_ASSERT_NOT_NULL(anot);
    TEST_ASSERT_EQUAL_INT(2, lista_tamanho(anot));
    qry_libera_anotacoes(anot);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_e_energiza_intervalo);
    RUN_TEST(test_e_intervalo_invertido);
    RUN_TEST(test_mv_desloca_circulo_livre);
    RUN_TEST(test_mv_nau_gasta_energia_e_ordem_da_fila);
    RUN_TEST(test_mv_sem_energia_nao_move);
    RUN_TEST(test_lr_captura_valores_e_remove);
    RUN_TEST(test_lr_sem_energia_lanca_com_energia_negativa);
    RUN_TEST(test_d_destroi_alvo_e_captura_riqueza);
    RUN_TEST(test_d_energia_exata_dispara);
    RUN_TEST(test_d_sem_alvo_so_gasta_energia);
    RUN_TEST(test_mc_translada_so_circulos_da_regiao);
    RUN_TEST(test_qry_vazio_retorna_lista_vazia);
    RUN_TEST(test_linha_invalida_ignorada);
    RUN_TEST(test_txt_registra_consultas);
    RUN_TEST(test_anotacoes_geradas);
    return UNITY_END();
}
