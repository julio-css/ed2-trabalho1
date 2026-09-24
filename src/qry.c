#include "qry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "forma.h"
#include "vermelha.h"
#include "retangulo.h"
#include "fila.h"
#include "geo.h"
#include "svg.h"

/*
 * ============================================================
 * ESTRUTURAS INTERNAS (opacas)
 * ============================================================
 */

/** Anotacao de desenho coletada durante o processamento. */
typedef struct
{
    char   tipo;           /* 'R' regiao, '*' impacto, 'x' nau destruida,
                              'o' circulo amarelo, 'q' quadrado amarelo */
    double x1, y1, x2, y2; /* coordenadas no plano (antes da conversao) */
} AnotacaoSvg;

/** Comando decodificado de uma linha do .qry (linha original preservada). */
typedef struct
{
    char   linha[512];
    char   cmd[16];
    char   lado[4];
    int    i, j;
    double v, dx, dy, d, w, h, x, y;
} ComandoQry;

/** Regiao retangular no plano. */
typedef struct
{
    double x1, y1, x2, y2;
} Regiao;

/** Contexto da captura da rede (lr). */
typedef struct
{
    Lista *capturados;
    double riqueza_total;
    double energia_ganha;
    int    nai_id;
    FILE  *txt;
    Regiao regiao;
} ContextoCaptura;

/** Contexto da translacao de peixes (mc). */
typedef struct
{
    Lista *peixes;
    Regiao regiao;
} ContextoPeixes;

/** Contexto para reinserir peixes movidos. */
typedef struct
{
    void  *arvore;
    double dx, dy;
} ContextoMove;

/** Contexto da busca do canhao (d). */
typedef struct
{
    double x1, y1, x2, y2; /* caixa em torno do raio (para a poda) */
    double rx1, ry1;       /* origem do raio (canhao) */
    double rx2, ry2;       /* extremo do raio */
    int    nai_id;         /* nau atiradora (nunca atingida) */
    int    menorId;        /* -1 se nenhuma nau atingida */
    double menorDist;
    double impactox, impactoy;
} ContextoCanhao;

/** Contexto da energizacao (e). */
typedef struct
{
    int i, j;
    double v;
    int n;
} ContextoE;

/*
 * ============================================================
 * AUXILIARES GEOMETRICOS
 * ============================================================
 */

/** true se duas caixas se sobrepoem no plano. */
static int caixasIntersectam(double a1, double b1, double a2, double b2,
                             double c1, double d1, double c2, double d2)
{
    return !(a2 < c1 || c2 < a1 || b2 < d1 || d2 < b1);
}

/** caixa onde cai a rede lancada da nau, do lado, a distancia dist. */
static void regiaoDaRede(const Forma *nau, const char *lado, double dist,
                         double w, double h, Regiao *r)
{
    double nx = forma_get_x(nau);
    double ny = forma_get_y(nau);
    double L  = forma_get_largura(nau);
    double H  = forma_get_altura(nau);

    if (strcmp(lado, "PP") == 0)
    {
        r->x1 = nx;             r->x2 = nx + w;
        r->y1 = ny - dist - h;  r->y2 = ny - dist;
    }
    else if (strcmp(lado, "PR") == 0)
    {
        r->x1 = nx;             r->x2 = nx + w;
        r->y1 = ny + H + dist;  r->y2 = ny + H + dist + h;
    }
    else if (strcmp(lado, "EB") == 0)
    {
        r->x1 = nx - dist - w;  r->x2 = nx - dist;
        r->y1 = ny;             r->y2 = ny + h;
    }
    else /* BB */
    {
        r->x1 = nx + L + dist;  r->x2 = nx + L + dist + w;
        r->y1 = ny;             r->y2 = ny + h;
    }
}

/** ponto medio do lado (posicao do canhao). */
static void canhaoDoLado(const Forma *nau, const char *lado,
                         double *px, double *py)
{
    if (strcmp(lado, "PP") == 0)
        retangulo_canhao_pp(nau, px, py);
    else if (strcmp(lado, "PR") == 0)
        retangulo_canhao_pr(nau, px, py);
    else if (strcmp(lado, "EB") == 0)
        retangulo_canhao_eb(nau, px, py);
    else
        retangulo_canhao_bb(nau, px, py);
}

static AnotacaoSvg *novaAnotacao(char tipo, double x1, double y1,
                                 double x2, double y2)
{
    AnotacaoSvg *a = malloc(sizeof(*a));
    if (a == NULL)
        return NULL;
    a->tipo = tipo;
    a->x1 = x1; a->y1 = y1; a->x2 = x2; a->y2 = y2;
    return a;
}

/*
 * ============================================================
 * CAPTURA DA REDE (lr) — busca por regiao com poda
 * ============================================================
 */

static int mbbIntersecta(double mx1, double my1, double mx2, double my2,
                         void *ctx)
{
    Regiao *r = ctx;
    return caixasIntersectam(mx1, my1, mx2, my2,
                             r->x1, r->y1, r->x2, r->y2);
}

static void capturaNaRegiao(void *dado, void *aux)
{
    Forma *f = dado;
    ContextoCaptura *c = aux;

    if (forma_get_id(f) == c->nai_id)
        return;
    if (forma_get_tipo(f) == FORMA_RETANGULO)
        return;

    double ax = forma_get_x(f), ay = forma_get_y(f);
    Regiao r = c->regiao;
    if (ax >= r.x1 - 1e-9 && ax <= r.x2 + 1e-9 &&
        ay >= r.y1 - 1e-9 && ay <= r.y2 + 1e-9)
        lista_insere(c->capturados, f);
}

static void visitaCapturado(void *item, void *aux)
{
    Forma *f = item;
    ContextoCaptura *c = aux;
    double valor = 0.0;
    const char *nome = "texto (alga)";

    switch (forma_get_tipo(f))
    {
        case FORMA_CIRCULO: valor = 5.0;  nome = "circulo (peixe)"; break;
        case FORMA_LINHA:   valor = 1.0;  nome = "linha (camarao)"; break;
        case FORMA_TEXTO:
        {
            char cls = forma_classifica_texto(f);
            if (cls == 'L')      { valor = 20.0; nome = "texto (lagosta)"; }
            else if (cls == 'M') { nome = "texto (moeda)"; }
            else                 { nome = "texto (alga)"; }
            break;
        }
        default: break;
    }

    if (forma_get_tipo(f) == FORMA_TEXTO && forma_classifica_texto(f) == 'M')
    {
        c->energia_ganha += 2.5;
        if (c->txt != NULL)
            fprintf(c->txt, "  id %d: %s energia +2.50\n",
                    forma_get_id(f), nome);
    }
    else
    {
        c->riqueza_total += valor;
        if (c->txt != NULL)
            fprintf(c->txt, "  id %d: %s valor M$%.2f\n",
                    forma_get_id(f), nome, valor);
    }
}

static void removeCapturado(void *item, void *aux)
{
    Forma *f = item;
    void *arvore = aux;
    void *removida = vermelha_remove_por_id(arvore, forma_get_id(f),
                                            geo_get_id);
    if (removida != NULL)
        forma_destroi(removida);
}

/*
 * ============================================================
 * CANHAO (d) — raio e distancia de entrada
 * ============================================================
 */

static void caixaDoRaio(double x1, double y1, double x2, double y2,
                        Regiao *r)
{
    double eps = 0.01;
    if (x1 == x2) /* raio vertical */
    {
        double ymin = (y1 < y2) ? y1 : y2;
        double ymax = (y1 < y2) ? y2 : y1;
        r->x1 = x1 - eps; r->x2 = x1 + eps;
        r->y1 = ymin - eps; r->y2 = ymax + eps;
    }
    else /* raio horizontal */
    {
        double xmin = (x1 < x2) ? x1 : x2;
        double xmax = (x1 < x2) ? x2 : x1;
        r->x1 = xmin - eps; r->x2 = xmax + eps;
        r->y1 = y1 - eps; r->y2 = y1 + eps;
    }
}

/** distancia do ponto de entrada de uma caixa no raio; -1 se nao cruza. */
static double distanciaDoRaio(const ContextoCanhao *c,
                              double mx1, double my1, double mx2, double my2)
{
    double eps = 1e-6;
    if (c->rx1 == c->rx2) /* raio vertical */
    {
        if (c->rx1 < mx1 - eps || c->rx1 > mx2 + eps)
            return -1.0;
        double ymin = (c->ry1 < c->ry2) ? c->ry1 : c->ry2;
        double ymax = (c->ry1 < c->ry2) ? c->ry2 : c->ry1;
        if (ymax < my1 - eps || ymin > my2 + eps)
            return -1.0;
        double entrada = (c->ry2 > c->ry1) ? my1 : my2;
        return fabs(entrada - c->ry1);
    }
    else /* raio horizontal */
    {
        if (c->ry1 < my1 - eps || c->ry1 > my2 + eps)
            return -1.0;
        double xmin = (c->rx1 < c->rx2) ? c->rx1 : c->rx2;
        double xmax = (c->rx1 < c->rx2) ? c->rx2 : c->rx1;
        if (xmax < mx1 - eps || xmin > mx2 + eps)
            return -1.0;
        double entrada = (c->rx2 > c->rx1) ? mx1 : mx2;
        return fabs(entrada - c->rx1);
    }
}

static int mbbIntersectaRaio(double mx1, double my1, double mx2, double my2,
                             void *ctx)
{
    ContextoCanhao *c = ctx;
    return caixasIntersectam(mx1, my1, mx2, my2,
                             c->x1, c->y1, c->x2, c->y2);
}

/** acha a nau (retangulo) mais proxima cujo MBB cruza o raio. */
static void avaliaNauNoRaio(void *dado, void *aux)
{
    Forma *f = dado;
    ContextoCanhao *c = aux;

    if (forma_get_tipo(f) != FORMA_RETANGULO)
        return;
    if (forma_get_id(f) == c->nai_id)
        return;

    double mx1, my1, mx2, my2;
    forma_mbb(f, &mx1, &my1, &mx2, &my2);

    double dist = distanciaDoRaio(c, mx1, my1, mx2, my2);
    if (dist >= 0 && (c->menorId < 0 || dist < c->menorDist))
    {
        c->menorId = forma_get_id(f);
        c->menorDist = dist;
        double len = hypot(c->rx2 - c->rx1, c->ry2 - c->ry1);
        if (len > 0)
        {
            c->impactox = c->rx1 + (c->rx2 - c->rx1) * (dist / len);
            c->impactoy = c->ry1 + (c->ry2 - c->ry1) * (dist / len);
        }
        else
        {
            c->impactox = c->rx1;
            c->impactoy = c->ry1;
        }
    }
}

/*
 * ============================================================
 * PEIXES (mc) — coletar e transladar
 * ============================================================
 */

static void coletaPeixe(void *dado, void *aux)
{
    Forma *f = dado;
    ContextoPeixes *c = aux;
    if (forma_get_tipo(f) != FORMA_CIRCULO)
        return;
    double ax = forma_get_x(f), ay = forma_get_y(f);
    if (ax >= c->regiao.x1 - 1e-9 && ax <= c->regiao.x2 + 1e-9 &&
        ay >= c->regiao.y1 - 1e-9 && ay <= c->regiao.y2 + 1e-9)
        lista_insere(c->peixes, f);
}

static void movePeixe(void *item, void *aux)
{
    Forma *f = item;
    ContextoMove *m = aux;
    (void)vermelha_remove_por_id(m->arvore, forma_get_id(f), geo_get_id);
    forma_set_x(f, forma_get_x(f) + m->dx);
    forma_set_y(f, forma_get_y(f) + m->dy);
    vermelha_insere(m->arvore, f);
}

/*
 * ============================================================
 * ENERGIZACAO (e)
 * ============================================================
 */

static void energizaNau(void *dado, void *aux)
{
    Forma *f = dado;
    ContextoE *c = aux;
    if (forma_get_tipo(f) == FORMA_RETANGULO &&
        forma_get_id(f) >= c->i && forma_get_id(f) <= c->j)
    {
        forma_set_energia(f, c->v);
        c->n++;
    }
}

/*
 * ============================================================
 * EXECUTORES
 * ============================================================
 */

static void executaE(ComandoQry *q, void *arvore, FILE *txt)
{
    ContextoE ce;
    ce.i = q->i; ce.j = q->j; ce.v = q->v; ce.n = 0;
    vermelha_em_ordem(arvore, energizaNau, &ce);
    if (txt != NULL)
        fprintf(txt, "naus energizadas entre %d e %d: %d\n",
                ce.i, ce.j, ce.n);
}

static void executaMv(ComandoQry *q, void *arvore, FILE *txt)
{
    Forma *f = vermelha_busca_por_id(arvore, q->i, geo_get_id);
    if (f == NULL)
    {
        if (txt != NULL)
            fprintf(txt, "forma %d nao encontrada\n", q->i);
        return;
    }

    double x0 = forma_get_x(f), y0 = forma_get_y(f);
    double dist = sqrt(q->dx * q->dx + q->dy * q->dy);
    double custo = dist / 5.0;

    if (forma_get_tipo(f) == FORMA_RETANGULO)
    {
        if (forma_get_energia(f) < custo)
        {
            if (txt != NULL)
                fprintf(txt, "forma %d nao movida: energia insuficiente "
                             "(%.3f < %.3f)\n",
                        q->i, forma_get_energia(f), custo);
            return;
        }
        forma_set_energia(f, forma_get_energia(f) - custo);
    }

    /* remove e reinsere para manter a arvore ordenada (X, area, Y) */
    (void)vermelha_remove_por_id(arvore, q->i, geo_get_id);
    forma_set_x(f, x0 + q->dx);
    forma_set_y(f, y0 + q->dy);
    vermelha_insere(arvore, f);

    if (txt != NULL)
    {
        fprintf(txt, "forma %d movida de (%.3f, %.3f) para (%.3f, %.3f)\n",
                q->i, x0, y0, forma_get_x(f), forma_get_y(f));
        if (forma_get_tipo(f) == FORMA_RETANGULO)
            fprintf(txt, "custo de energia: %.3f\n", custo);
    }
}

static void executaLr(ComandoQry *q, void *arvore, FILE *txt, Lista *anot)
{
    Forma *nau = vermelha_busca_por_id(arvore, q->i, geo_get_id);
    if (nau == NULL || forma_get_tipo(nau) != FORMA_RETANGULO)
    {
        if (txt != NULL)
            fprintf(txt, "nau %d nao encontrada\n", q->i);
        return;
    }

    double custo = (q->w * q->h) / 25.0 + q->d / 5.0;
    double energia_inicial = forma_get_energia(nau);

    if (energia_inicial < custo)
    {
        double cx = 0, cy = 0;
        canhaoDoLado(nau, q->lado, &cx, &cy);
        if (txt != NULL)
            fprintf(txt, "nau %d sem energia suficiente (%.3f < %.3f) "
                         "para lancar a rede\n",
                    q->i, energia_inicial, custo);
        AnotacaoSvg *a = novaAnotacao('o', cx, cy, 0, 0);
        if (a != NULL)
            lista_insere(anot, a);
        return;
    }

    Regiao rede;
    regiaoDaRede(nau, q->lado, q->d, q->w, q->h, &rede);

    if (txt != NULL)
        fprintf(txt, "rede lancada da nau %d lado %s em "
                     "(%.3f, %.3f, w=%.3f, h=%.3f)\n",
                q->i, q->lado, rede.x1, rede.y1, q->w, q->h);

    AnotacaoSvg *ra = novaAnotacao('R', rede.x1, rede.y1, rede.x2, rede.y2);
    if (ra != NULL)
        lista_insere(anot, ra);

    ContextoCaptura ctx;
    ctx.capturados = lista_cria();
    ctx.riqueza_total = 0.0;
    ctx.energia_ganha = 0.0;
    ctx.nai_id = q->i;
    ctx.txt = txt;
    ctx.regiao = rede;

    vermelha_busca_regiao(arvore, &rede, mbbIntersecta, capturaNaRegiao, &ctx);

    forma_set_energia(nau, energia_inicial - custo);

    if (txt != NULL && lista_tamanho(ctx.capturados) > 0)
    {
        fprintf(txt, "capturados:\n");
        lista_percorre(ctx.capturados, visitaCapturado, &ctx);
        fprintf(txt, "total desta captura: M$%.2f\n", ctx.riqueza_total);
    }

    lista_percorre(ctx.capturados, removeCapturado, arvore);
    lista_destroi(ctx.capturados, NULL);

    forma_add_riqueza(nau, ctx.riqueza_total);
    forma_set_energia(nau, forma_get_energia(nau) + ctx.energia_ganha);

    if (txt != NULL)
        fprintf(txt, "captura acumulada: M$%.2f\n", forma_get_riqueza(nau));
    if (txt != NULL)
        fprintf(txt, "energia antes: %.3f energia depois: %.3f\n",
                energia_inicial, forma_get_energia(nau));
}

static void executaD(ComandoQry *q, void *arvore, FILE *txt, Lista *anot)
{
    Forma *nau = vermelha_busca_por_id(arvore, q->i, geo_get_id);
    if (nau == NULL || forma_get_tipo(nau) != FORMA_RETANGULO)
    {
        if (txt != NULL)
            fprintf(txt, "nau %d nao encontrada\n", q->i);
        return;
    }

    double custo = q->d;
    double energia_inicial = forma_get_energia(nau);

    if (energia_inicial < custo)
    {
        double cx = 0, cy = 0;
        canhaoDoLado(nau, q->lado, &cx, &cy);
        if (txt != NULL)
            fprintf(txt, "nau %d sem energia suficiente (%.3f < %.3f) "
                         "para disparar\n",
                    q->i, energia_inicial, custo);
        AnotacaoSvg *a = novaAnotacao('q', cx, cy, 0, 0);
        if (a != NULL)
            lista_insere(anot, a);
        return;
    }

    double nx = forma_get_x(nau), ny = forma_get_y(nau);
    double L = forma_get_largura(nau), H = forma_get_altura(nau);

    ContextoCanhao cc;
    memset(&cc, 0, sizeof(cc));

    if (strcmp(q->lado, "PP") == 0)
    {
        cc.rx1 = nx + L / 2; cc.ry1 = ny;
        cc.rx2 = nx + L / 2; cc.ry2 = ny - q->d;
    }
    else if (strcmp(q->lado, "PR") == 0)
    {
        cc.rx1 = nx + L / 2; cc.ry1 = ny + H;
        cc.rx2 = nx + L / 2; cc.ry2 = ny + H + q->d;
    }
    else if (strcmp(q->lado, "EB") == 0)
    {
        cc.rx1 = nx;         cc.ry1 = ny + H / 2;
        cc.rx2 = nx - q->d;  cc.ry2 = ny + H / 2;
    }
    else /* BB */
    {
        cc.rx1 = nx + L;     cc.ry1 = ny + H / 2;
        cc.rx2 = nx + L + q->d; cc.ry2 = ny + H / 2;
    }

    Regiao raioBox;
    caixaDoRaio(cc.rx1, cc.ry1, cc.rx2, cc.ry2, &raioBox);
    cc.x1 = raioBox.x1; cc.y1 = raioBox.y1;
    cc.x2 = raioBox.x2; cc.y2 = raioBox.y2;
    cc.nai_id = q->i;
    cc.menorId = -1;

    vermelha_busca_regiao(arvore, &cc, mbbIntersectaRaio, avaliaNauNoRaio, &cc);

    forma_set_energia(nau, energia_inicial - custo);

    double px, py;
    if (cc.menorId < 0)
    {
        px = cc.rx2;
        py = cc.ry2;
    }
    else
    {
        px = cc.impactox;
        py = cc.impactoy;
    }

    if (txt != NULL)
        fprintf(txt, "ponto de impacto em (%.3f, %.3f)\n", px, py);

    if (cc.menorId >= 0)
    {
        double tx = 0, ty = 0;
        Forma *alvo = vermelha_remove_por_id(arvore, cc.menorId, geo_get_id);
        if (alvo != NULL)
        {
            tx = forma_get_x(alvo);
            ty = forma_get_y(alvo);
            double riq = forma_get_riqueza(alvo);
            forma_add_riqueza(nau, riq);
            if (txt != NULL)
                fprintf(txt, "nau %d atingida: riqueza M$%.2f capturada\n",
                        cc.menorId, riq);
            AnotacaoSvg *x = novaAnotacao('x', tx, ty, 0, 0);
            if (x != NULL)
                lista_insere(anot, x);
            forma_destroi(alvo);
        }
    }

    if (txt != NULL)
        fprintf(txt, "energia antes: %.3f energia depois: %.3f\n",
                energia_inicial, forma_get_energia(nau));

    AnotacaoSvg *ast = novaAnotacao('*', px, py, 0, 0);
    if (ast != NULL)
        lista_insere(anot, ast);
}

static void executaMc(ComandoQry *q, void *arvore, FILE *txt, Lista *anot)
{
    Regiao reg;
    reg.x1 = q->x;         reg.y1 = q->y;
    reg.x2 = q->x + q->w;  reg.y2 = q->y + q->h;

    ContextoPeixes cp;
    cp.peixes = lista_cria();
    cp.regiao = reg;

    vermelha_busca_regiao(arvore, &reg, mbbIntersecta, coletaPeixe, &cp);

    ContextoMove cm;
    cm.arvore = arvore;
    cm.dx = q->dx;
    cm.dy = q->dy;
    lista_percorre(cp.peixes, movePeixe, &cm);

    int n = lista_tamanho(cp.peixes);
    lista_destroi(cp.peixes, NULL);

    if (txt != NULL)
        fprintf(txt, "peixes transladados: %d\n", n);

    AnotacaoSvg *orig = novaAnotacao('R', reg.x1, reg.y1, reg.x2, reg.y2);
    if (orig != NULL)
        lista_insere(anot, orig);
    AnotacaoSvg *dest = novaAnotacao('R', reg.x1 + q->dx, reg.y1 + q->dy,
                                     reg.x2 + q->dx, reg.y2 + q->dy);
    if (dest != NULL)
        lista_insere(anot, dest);
}

/*
 * ============================================================
 * LEITURA DO .qry
 * ============================================================
 */

/** decodifica uma linha; retorna 1 se for um comando conhecido. */
static int parseiaComando(char *linha, ComandoQry *q)
{
    memset(q, 0, sizeof(*q));
    strncpy(q->linha, linha, sizeof(q->linha) - 1);
    q->linha[sizeof(q->linha) - 1] = '\0';

    if (sscanf(linha, "%15s", q->cmd) != 1)
        return 0;

    if (strcmp(q->cmd, "e") == 0)
        return sscanf(linha, "e %d %d %lf", &q->i, &q->j, &q->v) == 3;
    if (strcmp(q->cmd, "mv") == 0)
        return sscanf(linha, "mv %d %lf %lf", &q->i, &q->dx, &q->dy) == 3;
    if (strcmp(q->cmd, "lr") == 0)
        return sscanf(linha, "lr %d %3s %lf %lf %lf",
                      &q->i, q->lado, &q->d, &q->w, &q->h) == 5;
    if (strcmp(q->cmd, "d") == 0)
        return sscanf(linha, "d %d %3s %lf", &q->i, q->lado, &q->d) == 3;
    if (strcmp(q->cmd, "mc") == 0)
        return sscanf(linha, "mc %lf %lf %lf %lf %lf %lf",
                      &q->dx, &q->dy, &q->x, &q->y, &q->w, &q->h) == 6;
    return 0;
}

/*
 * ============================================================
 * API PUBLICA
 * ============================================================
 */

Lista *qry_processa(FILE *arq_qry, void *arvore, FILE *txt)
{
    if (arq_qry == NULL || arvore == NULL)
        return NULL;

    Lista *anotacoes = lista_cria();
    if (anotacoes == NULL)
        return NULL;

    Fila *fila = fila_cria();
    if (fila == NULL)
    {
        lista_destroi(anotacoes, NULL);
        return NULL;
    }

    /* 1a passada: enfileirar os comandos validos */
    char linha[512];
    while (fgets(linha, sizeof(linha), arq_qry) != NULL)
    {
        size_t len = strlen(linha);
        while (len > 0 && (linha[len - 1] == '\n' || linha[len - 1] == '\r'))
            linha[--len] = '\0';
        if (len == 0 || linha[0] == '#')
            continue;

        ComandoQry *q = malloc(sizeof(*q));
        if (q == NULL)
            continue;
        if (!parseiaComando(linha, q))
        {
            free(q);
            continue;
        }
        fila_insere(fila, q);
    }

    /* 2a passada: processar na ordem do arquivo */
    ComandoQry *q;
    while ((q = fila_retira(fila)) != NULL)
    {
        if (txt != NULL)
            fprintf(txt, "[*] %s\n", q->linha);

        if (strcmp(q->cmd, "e") == 0)
            executaE(q, arvore, txt);
        else if (strcmp(q->cmd, "mv") == 0)
            executaMv(q, arvore, txt);
        else if (strcmp(q->cmd, "lr") == 0)
            executaLr(q, arvore, txt, anotacoes);
        else if (strcmp(q->cmd, "d") == 0)
            executaD(q, arvore, txt, anotacoes);
        else if (strcmp(q->cmd, "mc") == 0)
            executaMc(q, arvore, txt, anotacoes);

        free(q);
    }

    fila_destroi(fila, NULL);
    return anotacoes;
}

static void desenhaAnotacao(void *item, void *aux)
{
    AnotacaoSvg *a = item;
    FILE *arq = aux;
    if (a->tipo == 'R')
        svg_desenha_regiao(arq, a->x1, a->y1, a->x2, a->y2);
    else
        svg_desenha_marcador(arq, a->x1, a->y1, a->tipo);
}

void qry_desenha_anotacoes(FILE *arq_svg, const Lista *anotacoes)
{
    if (arq_svg == NULL || anotacoes == NULL)
        return;
    lista_percorre(anotacoes, desenhaAnotacao, arq_svg);
}

void qry_libera_anotacoes(Lista *anotacoes)
{
    lista_destroi(anotacoes, free);
}
