#include "svg.h"

#include <stdio.h>
#include <math.h>

/**
 * MARGEM – espaco de respiro ao redor das formas no canvas.
 */
#define MARGEM 20.0

/**
 * converteX – translada x somando a margem.
 */
static double converteX(double x)
{
    return x + MARGEM;
}

/**
 * converteY – no SVG o y cresce para baixo; as coordenadas do .geo
 * ja estao no sistema de tela, entao apenas somamos a margem.
 */
static double converteY(double y)
{
    return y + MARGEM;
}

void svg_calcula_dimensoes(void *arvore, double *largura, double *altura)
{
    double maxx = 10, maxy = 10;
    /* percorre a arvore achando o maior x e y (em-ordem) */

    void visita(void *f, void *aux)
    {
        double x1, y1, x2, y2;
        forma_mbb((Forma *)f, &x1, &y1, &x2, &y2);
        double *m = aux;
        if (x2 > m[0]) m[0] = x2;
        if (y2 > m[1]) m[1] = y2;
    }

    double m[2] = {0, 0};
    vermelha_em_ordem(arvore, visita, m);
    maxx = m[0];
    maxy = m[1];

    *largura = maxx + 2 * MARGEM;
    *altura  = maxy + 2 * MARGEM;
}

void svg_abre(FILE *arq, double largura, double altura)
{
    fprintf(arq, "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                 "width=\"%g\" height=\"%g\" "
                 "viewBox=\"0 0 %g %g\">\n",
            largura, altura, largura, altura);
}

void svg_fecha(FILE *arq)
{
    fprintf(arq, "</svg>\n");
}

/**
 * desenhaCirculo – desenha um circulo (peixe).
 */
static void desenhaCirculo(FILE *arq, const Forma *f)
{
    double cx = converteX(forma_get_x(f));
    double cy = converteY(forma_get_y(f));
    double r  = forma_get_raio(f);
    fprintf(arq, "  <circle cx=\"%g\" cy=\"%g\" r=\"%g\" "
                 "fill=\"%s\" stroke=\"%s\" stroke-width=\"1\"/>\n",
            cx, cy, r,
            forma_get_cor_preench(f), forma_get_cor_borda(f));
}

/**
 * desenhaRetangulo – desenha um retangulo (nau).
 */
static void desenhaRetangulo(FILE *arq, const Forma *f)
{
    double x = converteX(forma_get_x(f));
    double y = converteY(forma_get_y(f));
    double w = forma_get_largura(f);
    double h = forma_get_altura(f);
    fprintf(arq, "  <rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" "
                 "fill=\"%s\" stroke=\"%s\" stroke-width=\"1\"/>\n",
            x, y, w, h,
            forma_get_cor_preench(f), forma_get_cor_borda(f));
}

/**
 * desenhaLinha – desenha uma linha (camarao).
 */
static void desenhaLinha(FILE *arq, const Forma *f)
{
    double x1 = converteX(forma_get_x(f));
    double y1 = converteY(forma_get_y(f));
    double x2 = converteX(forma_get_x2(f));
    double y2 = converteY(forma_get_y2(f));
    fprintf(arq, "  <line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" "
                 "stroke=\"%s\" stroke-width=\"1\"/>\n",
            x1, y1, x2, y2, forma_get_cor_borda(f));
}

/**
 * desenhaTexto – desenha um texto (lagosta/moeda/algas).
 */
static void desenhaTexto(FILE *arq, const Forma *f)
{
    double x = converteX(forma_get_x(f));
    double y = converteY(forma_get_y(f));
    fprintf(arq, "  <text x=\"%g\" y=\"%g\" fill=\"%s\" stroke=\"%s\">%s</text>\n",
            x, y,
            forma_get_cor_preench(f), forma_get_cor_borda(f),
            forma_get_texto(f));
}

void svg_desenha_forma(FILE *arq, const Forma *f)
{
    switch (forma_get_tipo(f))
    {
    case FORMA_CIRCULO:
        desenhaCirculo(arq, f);
        break;
    case FORMA_RETANGULO:
        desenhaRetangulo(arq, f);
        break;
    case FORMA_LINHA:
        desenhaLinha(arq, f);
        break;
    case FORMA_TEXTO:
        desenhaTexto(arq, f);
        break;
    }
}

void svg_desenha_tudo(FILE *arq, void *arvore)
{
    void visita(void *f, void *aux)
    {
        FILE *a = aux;
        svg_desenha_forma(a, (Forma *)f);
    }
    vermelha_em_ordem(arvore, visita, arq);
}