#include "retangulo.h"

#include <math.h>

Forma *retangulo_cria(int id, double x, double y, double w, double h,
                      const char *corb, const char *corp)
{
    return forma_cria_retangulo(id, x, y, w, h, corb, corp);
}

double retangulo_get_largura(const Forma *f)
{
    return forma_get_largura(f);
}

double retangulo_get_altura(const Forma *f)
{
    return forma_get_altura(f);
}

double retangulo_area(const Forma *f)
{
    return forma_area(f);
}

double retangulo_y_pp(const Forma *f)
{
    return forma_get_y(f);
}

double retangulo_y_pr(const Forma *f)
{
    return forma_get_y(f) + forma_get_altura(f);
}

double retangulo_x_eb(const Forma *f)
{
    return forma_get_x(f);
}

double retangulo_x_bb(const Forma *f)
{
    return forma_get_x(f) + forma_get_largura(f);
}

void retangulo_canhao_pp(const Forma *f, double *px, double *py)
{
    *px = forma_get_x(f) + forma_get_largura(f) / 2.0;
    *py = forma_get_y(f);
}

void retangulo_canhao_pr(const Forma *f, double *px, double *py)
{
    *px = forma_get_x(f) + forma_get_largura(f) / 2.0;
    *py = forma_get_y(f) + forma_get_altura(f);
}

void retangulo_canhao_eb(const Forma *f, double *px, double *py)
{
    *px = forma_get_x(f);
    *py = forma_get_y(f) + forma_get_altura(f) / 2.0;
}

void retangulo_canhao_bb(const Forma *f, double *px, double *py)
{
    *px = forma_get_x(f) + forma_get_largura(f);
    *py = forma_get_y(f) + forma_get_altura(f) / 2.0;
}

int retangulo_contem_ponto(const Forma *f, double x, double y)
{
    return forma_contem_ponto(f, x, y);
}