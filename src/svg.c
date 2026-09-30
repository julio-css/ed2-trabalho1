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
    fprintf(arq, "  <circle id=\"%d\" cx=\"%g\" cy=\"%g\" r=\"%g\" "
                 "fill=\"%s\" stroke=\"%s\" stroke-width=\"1\"/>\n",
            forma_get_id(f), cx, cy, r,
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
    fprintf(arq, "  <rect id=\"%d\" x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" "
                 "fill=\"%s\" stroke=\"%s\" stroke-width=\"1\"/>\n",
            forma_get_id(f), x, y, w, h,
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
    fprintf(arq, "  <line id=\"%d\" x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" "
                 "stroke=\"%s\" stroke-width=\"1\"/>\n",
            forma_get_id(f), x1, y1, x2, y2, forma_get_cor_borda(f));
}

/**
 * escapaXml – copia 's' para 'arq' escapando os caracteres reservados do
 * XML. Sem isso o '>-|-<' das formas de texto torna o .svg invalido
 * (o '<' abre uma tag e o parser quebra).
 */
static void escapaXml(FILE *arq, const char *s)
{
    if (s == NULL)
        return;

    for (; *s != '\0'; s++)
    {
        switch (*s)
        {
        case '&':  fputs("&amp;", arq);  break;
        case '<':  fputs("&lt;", arq);   break;
        case '>':  fputs("&gt;", arq);   break;
        case '"':  fputs("&quot;", arq); break;
        case '\'': fputs("&apos;", arq); break;
        default:   fputc(*s, arq);       break;
        }
    }
}

/**
 * desenhaTexto – desenha um texto (lagosta/moeda/algas).
 */
static void desenhaTexto(FILE *arq, const Forma *f)
{
    double x = converteX(forma_get_x(f));
    double y = converteY(forma_get_y(f));
    fprintf(arq, "  <text id=\"%d\" x=\"%g\" y=\"%g\" fill=\"%s\" stroke=\"%s\"> ",
            forma_get_id(f), x, y,
            forma_get_cor_preench(f), forma_get_cor_borda(f));
    escapaXml(arq, forma_get_texto(f));
    fprintf(arq, " </text>\n");
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

/**
 * corContornoEnergia – devolve a cor e a largura do contorno da nau
 * conforme o nivel final de energia (tabela do enunciado).
 */
static void corContornoEnergia(double e, const char **cor, double *larg)
{
    if (e <= 0.0)
    {
        *cor = "#484537";   /* energia 0.0 */
        *larg = 2;
    }
    else if (e < 100.0)
    {
        *cor = "#FFCC00";   /* 0.0 < e < 100.0 */
        *larg = 2;
    }
    else if (e < 250.0)
    {
        *cor = "#217821";   /* 100.0 <= e < 250.0 */
        *larg = 2;
    }
    else
    {
        *cor = "#800066";   /* 250.0 <= e */
        *larg = 3;
    }
}

/**
 * desenhaFormaFinal – desenha a forma no svg final, aplicando o
 * contorno de energia nas naus (retangulos).
 */
static void desenhaFormaFinal(FILE *arq, const Forma *f)
{
    if (forma_get_tipo(f) == FORMA_RETANGULO)
    {
        const char *cor;
        double larg;
        double x  = converteX(forma_get_x(f));
        double y  = converteY(forma_get_y(f));
        double w  = forma_get_largura(f);
        double h  = forma_get_altura(f);

        corContornoEnergia(forma_get_energia(f), &cor, &larg);
        fprintf(arq, "  <rect id=\"%d\" x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" "
                     "fill=\"%s\" stroke=\"%s\" stroke-width=\"%g\"/>\n",
                forma_get_id(f), x, y, w, h,
                forma_get_cor_preench(f), cor, larg);
    }
    else
    {
        svg_desenha_forma(arq, f);
    }
}

void svg_desenha_final(FILE *arq, void *arvore)
{
    void visita(void *f, void *aux)
    {
        FILE *a = aux;
        desenhaFormaFinal(a, (Forma *)f);
    }
    vermelha_em_ordem(arvore, visita, arq);
}

/**
 * desenhaAnel – circulo concentrico tracejado de anotacao.
 * r=1 e r=3 em vermelho, r=2 em amarelo (padrao do gabarito).
 */
static void desenhaAnel(FILE *arq, double x, double y, double raio,
                        const char *cor)
{
    fprintf(arq, "  <circle cx=\"%g\" cy=\"%g\" r=\"%g\" fill=\"none\" "
                 "stroke=\"%s\" stroke-opacity=\"0.5\" stroke-width=\"1\"/>\n",
            converteX(x), converteY(y), raio, cor);
}

/**
 * desenhaTrilha – segmento tracejado entre dois pontos, o rotulo no meio
 * e os tres aneis concentricos em cada ponta.
 */
void svg_desenha_trilha(FILE *arq, double x1, double y1, double x2, double y2,
                        const char *rotulo, const char *tracejado)
{
    static const struct { double raio; const char *cor; } aneis[3] = {
        { 1.0, "red" }, { 2.0, "yellow" }, { 3.0, "red" }
    };
    double cx = converteX(x1), cy = converteY(y1);
    double dx = converteX(x2), dy = converteY(y2);
    int i;

    fprintf(arq, "  <line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" "
                 "stroke=\"red\" stroke-width=\"1\" stroke-opacity=\"0.9\" "
                 "stroke-dasharray=\"%s\"/>\n",
            cx, cy, dx, dy, tracejado);

    if (rotulo != NULL && rotulo[0] != '\0')
        fprintf(arq, "  <text x=\"%g\" y=\"%g\" fill=\"black\" stroke=\"red\" "
                     "font-size=\"6pt\">%s</text>\n",
                (cx + dx) / 2.0, (cy + dy) / 2.0, rotulo);

    for (i = 0; i < 3; i++)
    {
        desenhaAnel(arq, x1, y1, aneis[i].raio, aneis[i].cor);
        desenhaAnel(arq, x2, y2, aneis[i].raio, aneis[i].cor);
    }
}

/**
 * svg_desenha_estrela – estrela de 12 vertices (doze pontas) no ponto de
 * impacto do canhao, preenchida de amarelo e contornada de vermelho.
 */
void svg_desenha_estrela(FILE *arq, double x, double y)
{
    double cx = converteX(x), cy = converteY(y);
    double r1 = 3.5, r2 = 1.6;
    int i, primeiro = 1;

    fprintf(arq, "  <path d=\"M");
    for (i = 0; i < 24; i++)
    {
        /* alterna raio externo/interno a cada 15 graus */
        double ang = i * 15.0 * 3.14159265358979323846 / 180.0;
        double r = (i % 2 == 0) ? r1 : r2;
        fprintf(arq, "%s %g %g", primeiro ? "" : "L",
                cx + r * cos(ang), cy + r * sin(ang));
        primeiro = 0;
    }
    fprintf(arq, " Z\" fill=\"yellow\" stroke=\"red\" stroke-width=\"1\"/>\n");
}

void svg_desenha_regiao(FILE *arq, double x1, double y1,
                        double x2, double y2)
{
    fprintf(arq, "  <rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" "
                 "fill=\"none\" stroke=\"black\" stroke-width=\"1\" "
                 "stroke-dasharray=\"4 4\"/>\n",
            converteX(x1), converteY(y1), x2 - x1, y2 - y1);
}

void svg_desenha_rede(FILE *arq, double x1, double y1, double x2, double y2)
{
    fprintf(arq, "  <rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" "
                 "fill=\"#f4d7d7\" stroke=\"red\" stroke-width=\"1\" "
                 "fill-opacity=\"0.3\" stroke-dasharray=\"1\"/>\n",
            converteX(x1), converteY(y1), x2 - x1, y2 - y1);
}

void svg_desenha_marcador(FILE *arq, double x, double y, char tipo)
{
    double cx = converteX(x);
    double cy = converteY(y);

    switch (tipo)
    {
        case '*':   /* asterisco (impacto do canhao) */
            fprintf(arq, "  <text x=\"%g\" y=\"%g\" fill=\"black\" "
                         "font-size=\"14\">*</text>\n", cx, cy);
            break;
        case 'x':   /* cruz (nau destruida) */
            fprintf(arq, "  <text x=\"%g\" y=\"%g\" fill=\"black\" "
                         "font-size=\"14\">x</text>\n", cx, cy);
            break;
        case 'o':   /* circulo amarelo (rede sem energia) */
            fprintf(arq, "  <circle cx=\"%g\" cy=\"%g\" r=\"4\" "
                         "fill=\"none\" stroke=\"yellow\" "
                         "stroke-width=\"2\"/>\n", cx, cy);
            break;
        case 'q':   /* quadrado amarelo (canhao sem energia) */
            fprintf(arq, "  <rect x=\"%g\" y=\"%g\" width=\"8\" height=\"8\" "
                         "fill=\"none\" stroke=\"yellow\" "
                         "stroke-width=\"2\"/>\n", cx - 4, cy - 4);
            break;
        default:
            break;
    }
}
