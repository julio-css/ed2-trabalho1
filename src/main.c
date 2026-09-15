#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "geo.h"
#include "svg.h"
#include "dot.h"
#include "vermelha.h"

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

    /* arvore Rubro-Negra com chamadas passadas */
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

    /* nome base sem extensao (arq.geo -> arq) */
    char base[512];
    strcpy(base, arq_geo);
    char *dot = strrchr(base, '.');
    if (dot)
        *dot = '\0';

    /* gera o arq.svg inicial com todas as formas */
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

    (void)arq_qry; /* consultas serao tratadas na etapa 7 */

    /* arvore final: gera o .dot para visualizar a rubro-negra */
    {
        char nome_dot[1100];
        snprintf(nome_dot, sizeof(nome_dot), "%s.dot", base);
        dot_exporta(arvore, geo_get_id, caminho_saida(nome_dot));
    }

    vermelha_destroi(arvore, (void (*)(void *))forma_destroi);
    return 0;
}