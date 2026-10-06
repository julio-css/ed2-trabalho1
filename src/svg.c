#include "svg.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

/**
 * MARGEM – espaco de respiro ao redor das formas no canvas.
 */
#define MARGEM 20.0

/**
 * converteX – o gabarito NAO aplica margem nas coordenadas.
 */
static double converteX(double x)
{
    return x;
}

/**
 * converteY – o gabarito NAO aplica margem nas coordenadas.
 */
static double converteY(double y)
{
    return y;
}

/**
 * arredonda – corta o valor em N casas decimais.
 *
 * O gabarito imprime as coordenadas assim (56.523591 vira "56.524"), entao
 * e' arredondamento, nao truncamento. Retangulos e circulos usam 3 casas;
 * textos e linhas, 6.
 */
static double arredonda(double v, int casas)
{
    double f = pow(10.0, casas);
    return (v < 0.0 ? -1.0 : 1.0) * floor(fabs(v) * f + 0.5) / f;
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

    /* gabarito nao adiciona margem extra nas dimensoes */
    *largura = maxx;
    *altura  = maxy;
}

void svg_abre(FILE *arq, double largura, double altura)
{
    fprintf(arq, "<?xml version='1.0' encoding='utf-8'?>\n"
                 "<svg xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                 "xmlns=\"http://www.w3.org/2000/svg\" "
                 "viewBox=\"%.6f %.6f %.6f %.6f\">\n"
                 "   <g id=\"fig\">\n",
            -5.0, -5.0, largura + 10.0, altura + 10.0);
}

void svg_fecha(FILE *arq)
{
    fprintf(arq, "   </g>\n</svg>\n");
}

/**
 * desenhaCirculo – desenha um circulo (peixe).
 *
 * O gabarito escreve a MEDIDA (r) antes da POSICAO (cx, cy) e fecha a
 * espessura em "1.0px"; a ordem dos atributos e comparada no texto.
 */
static void desenhaCirculo(FILE *arq, const Forma *f)
{
    double cx = converteX(forma_get_x(f));
    double cy = converteY(forma_get_y(f));
    double r  = forma_get_raio(f);
    fprintf(arq, "  <circle id=\"%d\" r=\"%.3f\" cx=\"%.3f\" cy=\"%.3f\" "
                 " fill=\"%s\" stroke=\"%s\" fill-opacity=\"0.5\" "
                 "stroke-width=\"1.0px\"/>\n",
            forma_get_id(f), arredonda(r, 3), arredonda(cx, 3), arredonda(cy, 3),
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
    fprintf(arq, "  <rect id=\"%d\" width=\"%.3f\" height=\"%.3f\" x=\"%.3f\" y=\"%.3f\" "
                 " fill=\"%s\" stroke=\"%s\" fill-opacity=\"0.5\" "
                 "stroke-width=\"1.0px\"/>\n",
            forma_get_id(f), arredonda(w, 3), arredonda(h, 3),
            arredonda(x, 3), arredonda(y, 3),
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
    fprintf(arq, "  <line id=\"%d\" x1=\"%.6f\" y1=\"%.6f\" x2=\"%.6f\" y2=\"%.6f\" "
                 "stroke=\"%s\" stroke-width=\"2\" stroke-opacity=\"1.000000\"/>\n",
            forma_get_id(f), arredonda(x1, 6), arredonda(y1, 6),
            arredonda(x2, 6), arredonda(y2, 6),
            forma_get_cor_borda(f));
}

/**
 * desenhaTexto – desenha um texto (lagosta/moeda/algas).
 */
static void desenhaTexto(FILE *arq, const Forma *f)
{
    double x = converteX(forma_get_x(f));
    double y = converteY(forma_get_y(f));
    const char *txt = forma_get_texto(f);

    /* o gabarito escreve o id DEPOIS da posicao e ancora o texto no meio */
    fprintf(arq, "  <text x=\"%.6f\" y=\"%.6f\" id=\"%d\" text-anchor=\"middle\" "
                 "fill=\"%s\" stroke=\"%s\" "
                 "font-family=\"serif\" font-size=\"12pt\" font-weight=\"normal\">",
            arredonda(x, 6), arredonda(y, 6), forma_get_id(f),
            forma_get_cor_preench(f), forma_get_cor_borda(f));

    /* o gabarito faz escape atomico do texto (&, <, >), sem CDATA */
    fprintf(arq, " ");
    if (txt != NULL)
        for (const char *p = txt; *p; p++)
        {
            if (*p == '&')
                fprintf(arq, "&amp;");
            else if (*p == '<')
                fprintf(arq, "&lt;");
            else if (*p == '>')
                fprintf(arq, "&gt;");
            else
                fputc(*p, arq);
        }
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

/**
 * desenhaPorTipo – desenha todas as formas de um tipo, em-ordem na arvore.
 *
 * O gabarito agrupa por tipo (retangulo, texto, circulo, linha) em vez de
 * percorrer a arvore em-ordem, entao a gente repete a varredura 4 vezes.
 */
static void desenhaPorTipo(FILE *arq, void *arvore, TipoForma t)
{
    void visita(void *f, void *aux)
    {
        FILE *a = aux;
        if (forma_get_tipo((Forma *)f) == t)
            svg_desenha_forma(a, (Forma *)f);
    }
    vermelha_em_ordem(arvore, visita, arq);
}

void svg_desenha_tudo(FILE *arq, void *arvore)
{
    desenhaPorTipo(arq, arvore, FORMA_RETANGULO);
    desenhaPorTipo(arq, arvore, FORMA_TEXTO);
    desenhaPorTipo(arq, arvore, FORMA_CIRCULO);
    desenhaPorTipo(arq, arvore, FORMA_LINHA);
}

/**
 * desenhaAnel – circulo concentrico tracejado de anotacao.
 * r=1 e r=3 em vermelho, r=2 em amarelo (padrao do gabarito).
 */
static void desenhaAnel(FILE *arq, double x, double y, double raio,
                        const char *cor)
{
    fprintf(arq, "  <circle cx=\"%f\" cy=\"%f\" r=\"%f\" fill=\"none\" "
                 "stroke=\"%s\" stroke-opacity=\"0.5\" stroke-width=\"1\"/>\n",
            converteX(x), converteY(y), raio, cor);
}

/**
 * desenhaTrilha – segmento tracejado entre dois pontos, o rotulo e os tres
 * aneis concentricos em cada ponta.
 *
 * O rotulo fica num ponto calculado a partir da origem e do comprimento da
 * trilha (ver svg_rotulo_trilha), e os aneis saem em r=1,2,3 pela ordem
 * vermelha/amarela/vermelha.
 */
void svg_desenha_trilha(FILE *arq, double x1, double y1, double x2, double y2,
                        const char *rotulo, const char *tracejado,
                        const char *tamRotulo)
{
    static const struct { double raio; const char *cor; } aneis[3] = {
        { 1.0, "red" }, { 2.0, "yellow" }, { 3.0, "red" }
    };
    double cx = converteX(x1), cy = converteY(y1);
    double dx = converteX(x2), dy = converteY(y2);
    int i;

    fprintf(arq, "  <line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" "
                 "stroke=\"red\" stroke-width=\"1\" stroke-opacity=\"0.900000\" "
                 "stroke-dasharray=\"%s\"/>\n",
            cx, cy, dx, dy, tracejado);

    if (rotulo != NULL && rotulo[0] != '\0')
        fprintf(arq, "  <text x=\"%f\" y=\"%f\" fill=\"black\" stroke=\"red\" "
                     "font-size=\"%s\">%s</text>\n",
                (cx + dx) / 2.0, (cy + dy) / 2.0,
                tamRotulo != NULL ? tamRotulo : "6pt", rotulo);

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
    double r1 = 3.5, r2 = 1.05;
    int i, primeiro = 1;

    fprintf(arq, "  <path d=\"M");
    for (i = 0; i < 12; i++)
    {
        /* alterna raio externo/interno a cada 30 graus */
        double ang = i * 30.0 * 3.14159265358979323846 / 180.0;
        double r = (i % 2 == 0) ? r1 : r2;
        fprintf(arq, "%s %g %g", primeiro ? "" : "L",
                cx + r * cos(ang), cy + r * sin(ang));
        primeiro = 0;
    }
    fprintf(arq, " Z\" stroke=\"red\" stroke-width=\"1px\" fill=\"yellow\"/>\n");
}

void svg_desenha_regiao(FILE *arq, double x1, double y1,
                        double x2, double y2)
{
    fprintf(arq, "  <rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" "
                 "rx=\"0.000000\" ry=\"0.000000\" fill=\"#f4d7d7\" "
                 "stroke=\"red\" stroke-width=\"1\" fill-opacity=\"0.300000\" "
                 "stroke-dasharray=\"1\"/>\n",
            converteX(x1), converteY(y1), x2 - x1, y2 - y1);
}

void svg_desenha_rede(FILE *arq, double x1, double y1, double x2, double y2)
{
    fprintf(arq, "  <rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" "
                 "rx=\"0.000000\" ry=\"0.000000\" fill=\"#f4d7d7\" "
                 "stroke=\"red\" stroke-width=\"1\" fill-opacity=\"0.100000\" "
                 "stroke-dasharray=\"1\"/>\n",
            converteX(x1), converteY(y1), x2 - x1, y2 - y1);
}

void svg_desenha_marcador(FILE *arq, double x, double y, char tipo)
{
    double cx = converteX(x);
    double cy = converteY(y);

    switch (tipo)
    {
        case '*':   /* asterisco (impacto do canhao) */
            fprintf(arq, "  <text x=\"%f\" y=\"%f\" fill=\"black\" "
                         "font-size=\"14\">*</text>\n", cx, cy);
            break;
        case 'x':   /* cruz (nau destruida) */
            fprintf(arq, "  <text x=\"%f\" y=\"%f\" fill=\"black\" "
                         "font-size=\"14\">x</text>\n", cx, cy);
            break;
        case 'o':   /* circulo amarelo (rede sem energia) */
            fprintf(arq, "  <circle cx=\"%f\" cy=\"%f\" r=\"4\" "
                         "fill=\"none\" stroke=\"yellow\" "
                         "stroke-width=\"2\"/>\n", cx, cy);
            break;
        case 'q':   /* quadrado amarelo (canhao sem energia) */
            fprintf(arq, "  <rect x=\"%f\" y=\"%f\" width=\"8\" height=\"8\" "
                         "fill=\"none\" stroke=\"yellow\" "
                         "stroke-width=\"2\"/>\n", cx - 4, cy - 4);
            break;
        default:
            break;
    }
}
