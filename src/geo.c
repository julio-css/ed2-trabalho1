#include "geo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circulo.h"
#include "retangulo.h"
#include "linha.h"
#include "texto.h"

/**
 * geo_compara_forma — comparador ED2:
 *   1) F1.X < F2.X
 *   2) F1.AREA < F2.AREA
 *   3) F1.Y < F2.Y
 *   4) F1.ID < F2.ID
 */
int geo_compara_forma(const void *f1, const void *f2)
{
    const Forma *a = (const Forma *)f1;
    const Forma *b = (const Forma *)f2;

    /* 1) X */
    double dx = forma_get_x(a) - forma_get_x(b);
    if (dx < -1e-9) return -1;
    if (dx >  1e-9) return  1;

    /* 2) AREA */
    double da = forma_area(a) - forma_area(b);
    if (da < -1e-9) return -1;
    if (da >  1e-9) return  1;

    /* 3) Y */
    double dy = forma_get_y(a) - forma_get_y(b);
    if (dy < -1e-9) return -1;
    if (dy >  1e-9) return  1;

    /* 4) desempate por id */
    int ia = forma_get_id(a);
    int ib = forma_get_id(b);
    return (ia < ib) ? -1 : (ia > ib) ? 1 : 0;
}

int geo_get_id(const void *elemento)
{
    return forma_get_id((const Forma *)elemento);
}

void geo_get_mbb(const void *elemento,
                 double *x1, double *y1, double *x2, double *y2)
{
    forma_mbb((const Forma *)elemento, x1, y1, x2, y2);
}

void geo_processa_arquivo(FILE *arq_geo, void *arvore)
{
    char cmd[4];
    if (arq_geo == NULL || arvore == NULL)
        return;

    while (fscanf(arq_geo, "%3s", cmd) == 1)
    {
        if (cmd[0] == '#')
        {
            /* comentario: descarta resto da linha */
            char buf[512];
            fgets(buf, sizeof(buf), arq_geo);
            continue;
        }

        if (strcmp(cmd, "c") == 0)
        {
            int id; double x, y, r;
            char corb[32], corp[32];
            if (fscanf(arq_geo, "%d %lf %lf %lf %31s %31s",
                       &id, &x, &y, &r, corb, corp) == 6)
            {
                Forma *f = forma_cria_circulo(id, x, y, r, corb, corp);
                if (f) vermelha_insere(arvore, f);
            }
        }
        else if (strcmp(cmd, "r") == 0)
        {
            int id; double x, y, w, h;
            char corb[32], corp[32];
            if (fscanf(arq_geo, "%d %lf %lf %lf %lf %31s %31s",
                       &id, &x, &y, &w, &h, corb, corp) == 7)
            {
                Forma *f = forma_cria_retangulo(id, x, y, w, h, corb, corp);
                if (f) vermelha_insere(arvore, f);
            }
        }
        else if (strcmp(cmd, "l") == 0)
        {
            int id; double x1, y1, x2, y2;
            char cor[32];
            if (fscanf(arq_geo, "%d %lf %lf %lf %lf %31s",
                       &id, &x1, &y1, &x2, &y2, cor) == 6)
            {
                Forma *f = linha_cria(id, x1, y1, x2, y2, cor);
                if (f) vermelha_insere(arvore, f);
            }
        }
        else if (strcmp(cmd, "t") == 0)
        {
            int id; double x, y;
            char corb[32], corp[32], ancora;
            char conteudo[256];
            if (fscanf(arq_geo, "%d %lf %lf %31s %31s %c ",
                       &id, &x, &y, corb, corp, &ancora) == 6)
            {
                /* texto vai ate o final da linha (pode ter espacos) */
                if (fgets(conteudo, sizeof(conteudo), arq_geo) != NULL)
                {
                    int len = strlen(conteudo);
                    if (len > 0 && conteudo[len - 1] == '\n')
                        conteudo[len - 1] = '\0';
                }
                Forma *f = texto_cria(id, x, y, corb, corp, ancora, conteudo);
                if (f) vermelha_insere(arvore, f);
            }
        }
        else if (strcmp(cmd, "ts") == 0)
        {
            /* estilo de texto: ignorar, apenas consumir os 3 parametros */
            char familia[32], peso[32], tamanho[32];
            fscanf(arq_geo, "%31s %31s %31s", familia, peso, tamanho);
        }
        else
        {
            /* comando desconhecido: descarta resto da linha */
            char buf[512];
            fgets(buf, sizeof(buf), arq_geo);
        }
    }
}
