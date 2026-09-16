#include "forma.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*
 * ============================================================
 * ESTRUTURAS INTERNAS (opacas, visiveis apenas neste .c)
 * ============================================================
 */

/**
 * Dados de um circulo (peixe).
 */
typedef struct
{
    double raio;
} DadosCirculo;

/**
 * Dados de um retangulo (nau).
 */
typedef struct
{
    double largura;
    double altura;
} DadosRetangulo;

/**
 * Dados de uma linha (camarao).
 */
typedef struct
{
    double x2;
    double y2;
} DadosLinha;

/**
 * Dados de um texto (lagosta/moeda/algas).
 */
typedef struct
{
    char ancora;
    char texto[FORMA_MAX_TEXTO];
} DadosTexto;

/**
 * Forma generica opaca do modulo.
 *
 * A coordenada ancora (x,y) depende do tipo:
 *   - circulo   : centro
 *   - retangulo : canto inferior esquerdo
 *   - linha     : extremidade com menor X (menor Y em empate)
 *   - texto     : posicao da ancora ('i', 'm' ou 'f')
 */
struct Forma
{
    int id;
    double x;
    double y;
    char cor_borda[FORMA_MAX_COR];
    char cor_preench[FORMA_MAX_COR];
    double energia; /* nivel de energia da nau (retangulo) */
    double riqueza; /* riqueza acumulada da nau (retangulo) */
    TipoForma tipo;
    union
    {
        DadosCirculo circulo;
        DadosRetangulo retangulo;
        DadosLinha linha;
        DadosTexto texto;
    } dados;
};

/*
 * ============================================================
 * HELPER UTILITARIO
 * ============================================================
 */

/**
 * copia_string – copia uma string para um buffer com tamanho limitado.
 *
 * Usa strncpy e garante terminador nulo, evitando estouro de buffer.
 */
static void copia_string(char *dest, size_t n, const char *fonte)
{
    if (fonte == NULL)
    {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, fonte, n - 1);
    dest[n - 1] = '\0';
}

/*
 * ============================================================
 * FABRICAS (CONSTRUTORES)
 * ============================================================
 */

Forma *forma_cria_circulo(int id, double x, double y, double r,
                          const char *corb, const char *corp)
{
    Forma *f = (Forma *)malloc(sizeof(Forma));
    if (f == NULL)
        return NULL;

    f->id = id;
    f->x = x;
    f->y = y;
    f->tipo = FORMA_CIRCULO;
    copia_string(f->cor_borda, FORMA_MAX_COR, corb);
    copia_string(f->cor_preench, FORMA_MAX_COR, corp);
    f->dados.circulo.raio = r;
    f->energia = 0.0;
    f->riqueza = 0.0;

    return f;
}

Forma *forma_cria_retangulo(int id, double x, double y, double w, double h,
                            const char *corb, const char *corp)
{
    Forma *f = (Forma *)malloc(sizeof(Forma));
    if (f == NULL)
        return NULL;

    f->id = id;
    f->x = x;
    f->y = y;
    f->tipo = FORMA_RETANGULO;
    copia_string(f->cor_borda, FORMA_MAX_COR, corb);
    copia_string(f->cor_preench, FORMA_MAX_COR, corp);
    f->dados.retangulo.largura = w;
    f->dados.retangulo.altura = h;
    f->energia = 0.0;
    f->riqueza = 0.0;

    return f;
}

Forma *forma_cria_linha(int id, double x1, double y1, double x2, double y2,
                        const char *cor)
{
    Forma *f = (Forma *)malloc(sizeof(Forma));
    if (f == NULL)
        return NULL;

    f->id = id;
    f->tipo = FORMA_LINHA;
    copia_string(f->cor_borda, FORMA_MAX_COR, cor);
    copia_string(f->cor_preench, FORMA_MAX_COR, "");
    f->energia = 0.0;
    f->riqueza = 0.0;

    /* ancora = extremidade com menor X (menor Y em empate) */
    if ((x1 < x2) || (fabs(x1 - x2) < 1e-9 && y1 <= y2))
    {
        f->x = x1;
        f->y = y1;
        f->dados.linha.x2 = x2;
        f->dados.linha.y2 = y2;
    }
    else
    {
        f->x = x2;
        f->y = y2;
        f->dados.linha.x2 = x1;
        f->dados.linha.y2 = y1;
    }

    return f;
}

Forma *forma_cria_texto(int id, double x, double y,
                        const char *corb, const char *corp,
                        char a, const char *texto)
{
    Forma *f = (Forma *)malloc(sizeof(Forma));
    if (f == NULL)
        return NULL;

    f->id = id;
    f->x = x;
    f->y = y;
    f->tipo = FORMA_TEXTO;
    copia_string(f->cor_borda, FORMA_MAX_COR, corb);
    copia_string(f->cor_preench, FORMA_MAX_COR, corp);
    f->dados.texto.ancora = a;
    copia_string(f->dados.texto.texto, FORMA_MAX_TEXTO, texto);
    f->energia = 0.0;
    f->riqueza = 0.0;

    return f;
}

void forma_destroi(Forma *forma)
{
    free(forma);
}

/*
 * ============================================================
 * GETTERS
 * ============================================================
 */

int forma_get_id(const Forma *f)
{
    return f->id;
}

TipoForma forma_get_tipo(const Forma *f)
{
    return f->tipo;
}

double forma_get_x(const Forma *f)
{
    return f->x;
}

double forma_get_y(const Forma *f)
{
    return f->y;
}

double forma_get_x2(const Forma *f)
{
    return f->dados.linha.x2;
}

double forma_get_y2(const Forma *f)
{
    return f->dados.linha.y2;
}

double forma_get_largura(const Forma *f)
{
    return f->dados.retangulo.largura;
}

double forma_get_altura(const Forma *f)
{
    return f->dados.retangulo.altura;
}

double forma_get_raio(const Forma *f)
{
    return f->dados.circulo.raio;
}

const char *forma_get_cor_borda(const Forma *f)
{
    return f->cor_borda;
}

const char *forma_get_cor_preench(const Forma *f)
{
    return f->cor_preench;
}

char forma_get_ancora(const Forma *f)
{
    return f->dados.texto.ancora;
}

const char *forma_get_texto(const Forma *f)
{
    return f->dados.texto.texto;
}

/*
 * ============================================================
 * SETTERS
 * ============================================================
 */

void forma_set_x(Forma *f, double x)
{
    f->x = x;
}

void forma_set_y(Forma *f, double y)
{
    f->y = y;
}

double forma_get_energia(const Forma *f)
{
    return f->energia;
}

double forma_get_riqueza(const Forma *f)
{
    return f->riqueza;
}

void forma_set_energia(Forma *f, double energia)
{
    f->energia = energia;
}

void forma_add_riqueza(Forma *f, double valor)
{
    f->riqueza += valor;
}

/*
 * ============================================================
 * CALCULOS GEOMETRICOS
 * ============================================================
 */

double forma_area(const Forma *f)
{
    switch (f->tipo)
    {
        case FORMA_CIRCULO:
            return M_PI * f->dados.circulo.raio * f->dados.circulo.raio;
        case FORMA_RETANGULO:
            return f->dados.retangulo.largura * f->dados.retangulo.altura;
        case FORMA_LINHA:
        case FORMA_TEXTO:
        default:
            return 0.0;
    }
}

int forma_contem_ponto(const Forma *f, double px, double py)
{
    switch (f->tipo)
    {
        case FORMA_CIRCULO:
        {
            double dx = px - f->x;
            double dy = py - f->y;
            return (dx * dx + dy * dy) <= f->dados.circulo.raio * f->dados.circulo.raio;
        }
        case FORMA_RETANGULO:
            return px >= f->x && px <= f->x + f->dados.retangulo.largura &&
                   py >= f->y && py <= f->y + f->dados.retangulo.altura;
        case FORMA_LINHA:
        default:
            /* linhas e textos so tem ancora; nenhum ponto interno */
            return 0;
    }
}

void forma_mbb(const Forma *f, double *x1, double *y1, double *x2, double *y2)
{
    switch (f->tipo)
    {
        case FORMA_CIRCULO:
        {
            double r = f->dados.circulo.raio;
            *x1 = f->x - r;
            *y1 = f->y - r;
            *x2 = f->x + r;
            *y2 = f->y + r;
            break;
        }
        case FORMA_RETANGULO:
            *x1 = f->x;
            *y1 = f->y;
            *x2 = f->x + f->dados.retangulo.largura;
            *y2 = f->y + f->dados.retangulo.altura;
            break;
        case FORMA_LINHA:
        {
            double ax = (f->x < f->dados.linha.x2) ? f->x : f->dados.linha.x2;
            double bx = (f->x < f->dados.linha.x2) ? f->dados.linha.x2 : f->x;
            double ay = (f->y < f->dados.linha.y2) ? f->y : f->dados.linha.y2;
            double by = (f->y < f->dados.linha.y2) ? f->dados.linha.y2 : f->y;
            *x1 = ax;
            *y1 = ay;
            *x2 = bx;
            *y2 = by;
            break;
        }
        case FORMA_TEXTO:
        {
            /* considera uma caixa de referencia ao redor da ancora */
            double w = 30.0, h = 12.0;
            *x1 = f->x;
            *y1 = f->y;
            *x2 = f->x + w;
            *y2 = f->y + h;
            break;
        }
        default:
            *x1 = f->x;
            *y1 = f->y;
            *x2 = f->x;
            *y2 = f->y;
            break;
    }
}

char forma_classifica_texto(const Forma *f)
{
    const char *t = f->dados.texto.texto;
    if (t == NULL)
        return 'A';

    /* lagosta: texto >-|-< */
    if (strcmp(t, ">-|-<") == 0)
        return 'L';
    /* moeda: texto $ */
    if (strcmp(t, "$") == 0)
        return 'M';
    /* qualquer outro texto: algas/detritos */
    return 'A';
}

/*
 * ============================================================
 * CLONE
 * ============================================================
 */

Forma *forma_clona(const Forma *f)
{
    if (f == NULL)
        return NULL;

    Forma *n = (Forma *)malloc(sizeof(Forma));
    if (n == NULL)
        return NULL;

    *n = *f;
    return n;
}