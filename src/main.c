#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "geo.h"
#include "svg.h"
#include "dot.h"
#include "txt.h"
#include "vermelha.h"
#include "forma.h"
#include "qry.h"

/**
 * Diretorio onde estao os arquivos de entrada (BED) e de saida (BSD).
 */
static char bed[512] = ".";
static char bsd[512] = ".";
static char arq_geo[512] = "";
static char arq_qry[512] = "";

static const char *caminho_entrada(const char *nome)
{
    static char buf[1100];
    snprintf(buf, sizeof(buf), "%s/%s", bed, nome);
    return buf;
}

static const char *caminho_saida(const char *nome)
{
    static char buf[1100];
    snprintf(buf, sizeof(buf), "%s/%s", bsd, nome);
    return buf;
}

/**
 * soNome — grava em 'saida' o nome-base de 'nome': sem path
 * (aceita separadores '/' ou '\\') e sem extensao.
 * Ex.: ".\\testes\\t001.geo" -> "t001".
 */
static void soNome(const char *nome, char *saida, size_t tam)
{
    snprintf(saida, tam, "%s", nome);

    char *sep = strrchr(saida, '/');
    char *barra = strrchr(saida, '\\');
    if (barra != NULL && (sep == NULL || barra > sep))
        sep = barra;
    if (sep != NULL)
        memmove(saida, sep + 1, strlen(sep));

    char *pd = strrchr(saida, '.');
    if (pd != NULL)
        *pd = '\0';
}

int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc)
            strcpy(bed, argv[++i]);
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
            strcpy(bsd, argv[++i]);
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)
            strcpy(arq_geo, argv[++i]);
        else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc)
            strcpy(arq_qry, argv[++i]);
    }

    if (arq_geo[0] == '\0' || bsd[0] == '\0')
    {
        fprintf(stderr, "uso: ted -e BED -f arq.geo -o BSD [-q arq.qry]\n");
        return 1;
    }

    /* arvore Rubro-Negra com as chamadas passadas */
    void *arvore = vermelha_cria(geo_compara_forma, geo_get_mbb);
    if (arvore == NULL)
        return 1;

    /* le o arquivo .geo e popula a arvore */
    FILE *geo = fopen(caminho_entrada(arq_geo), "r");
    if (geo == NULL)
    {
        fprintf(stderr, "erro: nao achei %s\n", caminho_entrada(arq_geo));
        return 1;
    }
    geo_processa_arquivo(geo, arvore);
    fclose(geo);

    char base[512], base_qry[512];
    soNome(arq_geo, base, sizeof(base));

    /* arq.svg: estado inicial, antes de qualquer consulta */
    {
        char nome_svg[1100];
        snprintf(nome_svg, sizeof(nome_svg), "%s.svg", base);
        FILE *arq = fopen(caminho_saida(nome_svg), "w");
        if (arq != NULL)
        {
            double larg, alt;
            svg_calcula_dimensoes(arvore, &larg, &alt);
            svg_abre(arq, larg, alt);
            svg_desenha_tudo(arq, arvore);
            svg_fecha(arq);
            fclose(arq);
        }
    }

    Lista *anotacoes = NULL;

    if (arq_qry[0] != '\0')
    {
        FILE *qry = fopen(caminho_entrada(arq_qry), "r");
        if (qry == NULL)
        {
            fprintf(stderr, "erro: nao achei %s\n", caminho_entrada(arq_qry));
            return 1;
        }
        soNome(arq_qry, base_qry, sizeof(base_qry));

        /* base-baseqry.txt: resultados das consultas + contabilidade final */
        {
            char nome_txt[1100];
            snprintf(nome_txt, sizeof(nome_txt), "%s-%s.txt", base, base_qry);
            FILE *txt = fopen(caminho_saida(nome_txt), "w");
            if (txt != NULL)
            {
                anotacoes = qry_processa(qry, arvore, txt);
                txt_escreve_final(txt, arvore);
                fclose(txt);
            }
        }
        fclose(qry);

        /* base-baseqry.svg: estado final + contornos de energia + anotacoes */
        {
            char nome_svg[1100];
            snprintf(nome_svg, sizeof(nome_svg), "%s-%s.svg", base, base_qry);
            FILE *sq = fopen(caminho_saida(nome_svg), "w");
            if (sq != NULL)
            {
                double larg, alt;
                svg_calcula_dimensoes(arvore, &larg, &alt);
                svg_abre(sq, larg, alt);
                svg_desenha_final(sq, arvore);
                qry_desenha_anotacoes(sq, anotacoes);
                svg_fecha(sq);
                fclose(sq);
            }
        }
    }

    /* arvore final: gera o .dot para visualizar a rubro-negra */
    {
        char nome_dot[1100];
        snprintf(nome_dot, sizeof(nome_dot), "%s.dot", base);
        dot_exporta(arvore, geo_get_id, caminho_saida(nome_dot));
    }

    qry_libera_anotacoes(anotacoes);
    vermelha_destroi(arvore, (void (*)(void *))forma_destroi);
    return 0;
}
