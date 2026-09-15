#include "circulo.h"

#include <math.h>

Forma *circulo_cria(int id, double x, double y, double r,
                    const char *corb, const char *corp)
{
    return forma_cria_circulo(id, x, y, r, corb, corp);
}

double circulo_get_raio(const Forma *f)
{
    return forma_get_raio(f);
}

double circulo_area(const Forma *f)
{
    return forma_area(f);
}

int circulo_contem_ponto(const Forma *f, double x, double y)
{
    return forma_contem_ponto(f, x, y);
}

double circulo_get_x(const Forma *f)
{
    return forma_get_x(f);
}

double circulo_get_y(const Forma *f)
{
    return forma_get_y(f);
}