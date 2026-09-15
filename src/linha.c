#include "linha.h"

#include <math.h>

Forma *linha_cria(int id, double x1, double y1, double x2, double y2,
                  const char *cor)
{
    return forma_cria_linha(id, x1, y1, x2, y2, cor);
}

double linha_get_x2(const Forma *f)
{
    return forma_get_x2(f);
}

double linha_get_y2(const Forma *f)
{
    return forma_get_y2(f);
}

double linha_comprimento(const Forma *f)
{
    double dx = forma_get_x(f) - forma_get_x2(f);
    double dy = forma_get_y(f) - forma_get_y2(f);
    return sqrt(dx * dx + dy * dy);
}

double linha_get_x(const Forma *f)
{
    return forma_get_x(f);
}

double linha_get_y(const Forma *f)
{
    return forma_get_y(f);
}