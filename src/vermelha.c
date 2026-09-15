#include "vermelha.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ============================================================
 * ESTRUTURAS INTERNAS (opacas)
 * ============================================================
 */

/**
 * No da arvore Rubro-Negra.
 *
 * Guarda a informacao (ponteiro generico), a cor, os filhos e o MBB
 * do proprio no uniao dos MBBs das sub-arvores.
 */
typedef struct no
{
    void       *info;
    CorNo       cor;
    struct no  *esq;
    struct no  *dir;
    /* MBB (menor retangulo envolvente) que cobre o no e sub-arvores */
    double mbx1, mby1, mbx2, mby2;
} No;

/**
 * Implementacao da arvore (opaca para o .h).
 */
typedef struct
{
    No             *raiz;
    FuncaoComparacao comp;
    FuncaoGetMbb     getmbb;
    /* estatisticas para demonstrar a poda nas buscas por regiao */
    int    contador_intersecoes;
    int    contador_nos_visitados;
} ArvImpl;

/*
 * ============================================================
 * AUXILIARES BASICOS
 * ============================================================
 */

/**
 * ehVermelho – retorna 1 se o no e vermelho (NULL conta como preto).
 */
static int ehVermelho(const No *n)
{
    return n != NULL && n->cor == VERMELHO;
}

/**
 * novoNo – aloca um no vermelho com o dado informado.
 */
static No *criarNo(void *info)
{
    No *n = (No *)malloc(sizeof(No));
    if (n == NULL)
        return NULL;

    n->info = info;
    n->cor  = VERMELHO;
    n->esq  = NULL;
    n->dir  = NULL;
    n->mbx1 = n->mby1 = n->mbx2 = n->mby2 = 0.0;
    return n;
}

/**
 * mbbDaForma – obtem o MBB de um dado via callback.
 */
static void mbbDe(ArvImpl *arv, const void *info,
                  double *x1, double *y1, double *x2, double *y2)
{
    arv->getmbb(info, x1, y1, x2, y2);
}

/**
 * atualizarMbb – recalcula o MBB do no unindo o proprio com os filhos.
 *
 * Deve ser chamada depois de qualquer alteracao estrutural (insercao,
 * remocao, rotacao), conforme exigido pelo enunciado.
 */
static void atualizarMbb(ArvImpl *arv, No *n)
{
    if (n == NULL)
        return;

    double x1, y1, x2, y2;
    mbbDe(arv, n->info, &x1, &y1, &x2, &y2);

    double minx = x1, miny = y1, maxx = x2, maxy = y2;

    if (n->esq != NULL)
    {
        minx = (n->esq->mbx1 < minx) ? n->esq->mbx1 : minx;
        miny = (n->esq->mby1 < miny) ? n->esq->mby1 : miny;
        maxx = (n->esq->mbx2 > maxx) ? n->esq->mbx2 : maxx;
        maxy = (n->esq->mby2 > maxy) ? n->esq->mby2 : maxy;
    }
    if (n->dir != NULL)
    {
        minx = (n->dir->mbx1 < minx) ? n->dir->mbx1 : minx;
        miny = (n->dir->mby1 < miny) ? n->dir->mby1 : miny;
        maxx = (n->dir->mbx2 > maxx) ? n->dir->mbx2 : maxx;
        maxy = (n->dir->mby2 > maxy) ? n->dir->mby2 : maxy;
    }

    n->mbx1 = minx;
    n->mby1 = miny;
    n->mbx2 = maxx;
    n->mby2 = maxy;
}

/*
 * ============================================================
 * ROTACOES (atualizam MBBS dos nos envolvidos)
 * ============================================================
 */

/**
 * rotacionarEsq – rotacao simples a esquerda (mantem ordenacao).
 */
static No *rotacionarEsq(ArvImpl *arv, No *raiz)
{
    No *x = raiz->dir;
    No *t2 = x->esq;

    x->esq = raiz;
    raiz->dir = t2;

    /* troca de cores: o filho sobe mantendo a cor do pai,
       e o pai passa a ser vermelho (rota um link vermelho) */
    x->cor = raiz->cor;
    raiz->cor = VERMELHO;

    /* atualiza MBBS dos nos rotacionados */
    atualizarMbb(arv, raiz);
    atualizarMbb(arv, x);
    return x;
}

/**
 * rotacionarDir – rotacao simples a direita (mantem ordenacao).
 */
static No *rotacionarDir(ArvImpl *arv, No *raiz)
{
    No *x = raiz->esq;
    No *t2 = x->dir;

    x->dir = raiz;
    raiz->esq = t2;

    /* troca de cores (analoga a rotacionarEsq) */
    x->cor = raiz->cor;
    raiz->cor = VERMELHO;

    atualizarMbb(arv, raiz);
    atualizarMbb(arv, x);
    return x;
}

/**
 * flipCores – recolora um no e seus dois filhos (nao muda a ordenacao).
 */
static void flipCores(No *n)
{
    n->cor = (n->cor == VERMELHO) ? PRETO : VERMELHO;
    if (n->esq != NULL)
        n->esq->cor = (n->esq->cor == VERMELHO) ? PRETO : VERMELHO;
    if (n->dir != NULL)
        n->dir->cor = (n->dir->cor == VERMELHO) ? PRETO : VERMELHO;
}

/*
 * ============================================================
 * INSERCAO RECURSIVA (Rubro-Negra Left-Leaning)
 * ============================================================
 */

/**
 * fixUp – restaura as propriedades Rubro-Negro apos insercao/remocao.
 *
 * Aplica, na ordem:
 *   1) se filho direito e vermelho e esquerdo e preto  -> rot. esq
 *   2) se filho esquerdo e vermelho e neto esq-esq vermelho -> rot. dir
 *   3) se ambos filhos vermelhos -> flipCores
 *
 * Sempre atualiza o MBB do no recebido.
 */
static No *fixUp(ArvImpl *arv, No *raiz)
{
    if (raiz == NULL)
        return NULL;

    if (ehVermelho(raiz->dir) && !ehVermelho(raiz->esq))
        raiz = rotacionarEsq(arv, raiz);

    if (ehVermelho(raiz->esq) && ehVermelho(raiz->esq->esq))
        raiz = rotacionarDir(arv, raiz);

    if (ehVermelho(raiz->esq) && ehVermelho(raiz->dir))
        flipCores(raiz);

    atualizarMbb(arv, raiz);
    return raiz;
}

/**
 * inserirNo – insercao recursiva (padrao LLRB).
 */
static No *inserirNo(ArvImpl *arv, No *raiz, void *elemento)
{
    if (raiz == NULL)
    {
        No *novo = criarNo(elemento);
        /* a folha precisa do proprio MBB calculado agora (para a poda) */
        atualizarMbb(arv, novo);
        return novo;
    }

    if (arv->comp(elemento, raiz->info) < 0)
        raiz->esq = inserirNo(arv, raiz->esq, elemento);
    else
        raiz->dir = inserirNo(arv, raiz->dir, elemento);

    return fixUp(arv, raiz);
}

void vermelha_insere(void *arvore, void *elemento)
{
    if (arvore == NULL || elemento == NULL)
        return;

    ArvImpl *arv = (ArvImpl *)arvore;
    arv->raiz = inserirNo(arv, arv->raiz, elemento);
    if (arv->raiz != NULL)
        arv->raiz->cor = PRETO; /* raiz sempre preta */
}

/*
 * ============================================================
 * REMOCAO RECURSIVA (LLRB, com moveRedLeft/moveRedRight)
 * ============================================================
 */

/**
 * moveRedLeft – faz com que o filho esquerdo (ou seu neto) fique
 * vermelho, em preparo para remocao.
 */
static No *moveRedLeft(ArvImpl *arv, No *raiz)
{
    flipCores(raiz);
    if (ehVermelho(raiz->dir->esq))
    {
        raiz->dir = rotacionarDir(arv, raiz->dir);
        raiz = rotacionarEsq(arv, raiz);
        flipCores(raiz);
    }
    atualizarMbb(arv, raiz);
    return raiz;
}

/**
 * moveRedRight – analogo para o filho direito.
 */
static No *moveRedRight(ArvImpl *arv, No *raiz)
{
    flipCores(raiz);
    if (ehVermelho(raiz->esq->esq))
    {
        raiz = rotacionarDir(arv, raiz);
        flipCores(raiz);
    }
    atualizarMbb(arv, raiz);
    return raiz;
}

/**
 * minimoNo – retorna o menor no de uma sub-arvore (mais a esquerda).
 */
static No *minimoNo(No *raiz)
{
    while (raiz->esq != NULL)
        raiz = raiz->esq;
    return raiz;
}

/**
 * removerMin – remove recursivamente o menor no da sub-arvore.
 */
static No *removerMin(ArvImpl *arv, No *raiz)
{
    if (raiz->esq == NULL)
    {
        free(raiz);
        return NULL;
    }
    if (!ehVermelho(raiz->esq) && !ehVermelho(raiz->esq->esq))
        raiz = moveRedLeft(arv, raiz);

    raiz->esq = removerMin(arv, raiz->esq);
    return fixUp(arv, raiz);
}

/**
 * removerNo – remocao recursiva pela chave.
 *
 * Preenche 'removido' com o dado do no removido (primeira ocorrencia).
 */
static No *removerNo(ArvImpl *arv, No *raiz, const void *chave, void **removido)
{
    if (raiz == NULL)
        return NULL;

    if (arv->comp(chave, raiz->info) < 0)
    {
        if (!ehVermelho(raiz->esq) && !ehVermelho(raiz->esq->esq))
            raiz = moveRedLeft(arv, raiz);
        raiz->esq = removerNo(arv, raiz->esq, chave, removido);
    }
    else
    {
        if (ehVermelho(raiz->esq))
            raiz = rotacionarDir(arv, raiz);

        if (arv->comp(chave, raiz->info) == 0 && raiz->dir == NULL)
        {
            *removido = raiz->info;
            free(raiz);
            return NULL;
        }

        if (!ehVermelho(raiz->dir) && !ehVermelho(raiz->dir->esq))
            raiz = moveRedRight(arv, raiz);

        if (arv->comp(chave, raiz->info) == 0)
        {
            No *sucessor = minimoNo(raiz->dir);
            void *alvo = raiz->info;
            raiz->info = sucessor->info;
            raiz->dir = removerMin(arv, raiz->dir);
            *removido = alvo;
        }
        else
        {
            raiz->dir = removerNo(arv, raiz->dir, chave, removido);
        }
    }
    return fixUp(arv, raiz);
}

void *vermelha_remove(void *arvore, const void *chave)
{
    if (arvore == NULL || chave == NULL)
        return NULL;

    ArvImpl *arv = (ArvImpl *)arvore;
    void *removido = NULL;
    arv->raiz = removerNo(arv, arv->raiz, chave, &removido);
    if (arv->raiz != NULL)
        arv->raiz->cor = PRETO;
    return removido;
}

/*
 * ============================================================
 * BUSCA (recursiva) POR CHAVE E POR ID
 * ============================================================
 */

static void *buscarNo(const ArvImpl *arv, const No *raiz, const void *chave)
{
    if (raiz == NULL)
        return NULL;

    int res = arv->comp(chave, raiz->info);
    if (res == 0)
        return raiz->info;
    if (res < 0)
        return buscarNo(arv, raiz->esq, chave);
    return buscarNo(arv, raiz->dir, chave);
}

void *vermelha_busca(void *arvore, const void *chave)
{
    if (arvore == NULL || chave == NULL)
        return NULL;

    ArvImpl *arv = (ArvImpl *)arvore;
    return buscarNo(arv, arv->raiz, chave);
}

/**
 * buscarIdNo – percorre a arvore ate achar o no com o id informado.
 */
static void *buscarIdNo(const No *raiz, int id, FuncaoGetId getid)
{
    if (raiz == NULL)
        return NULL;

    void *achado = buscarIdNo(raiz->esq, id, getid);
    if (achado != NULL)
        return achado;
    if (getid(raiz->info) == id)
        return raiz->info;
    return buscarIdNo(raiz->dir, id, getid);
}

void *vermelha_busca_por_id(void *arvore, int id, FuncaoGetId getid)
{
    if (arvore == NULL || getid == NULL)
        return NULL;

    ArvImpl *arv = (ArvImpl *)arvore;
    return buscarIdNo(arv->raiz, id, getid);
}

void *vermelha_remove_por_id(void *arvore, int id, FuncaoGetId getid)
{
    void *elem = vermelha_busca_por_id(arvore, id, getid);
    if (elem == NULL)
        return NULL;
    /* remove pela chave de ordenacao do proprio elemento */
    return vermelha_remove(arvore, elem);
}

/*
 * ============================================================
 * TRAVESSIA IN-ORDER
 * ============================================================
 */

static void emOrdemNo(const ArvImpl *arv, const No *raiz,
                      FuncaoVisita visita, void *aux)
{
    (void)arv;
    if (raiz == NULL)
        return;
    emOrdemNo(arv, raiz->esq, visita, aux);
    visita(raiz->info, aux);
    emOrdemNo(arv, raiz->dir, visita, aux);
}

void vermelha_em_ordem(void *arvore, FuncaoVisita visita, void *aux)
{
    if (arvore == NULL || visita == NULL)
        return;

    ArvImpl *arv = (ArvImpl *)arvore;
    emOrdemNo(arv, arv->raiz, visita, aux);
}

/*
 * ============================================================
 * BUSCA POR REGIAO COM PODA (MBB)
 * ============================================================
 */

/**
 * buscaRegiaoNo – percorre somente os ramos cujo MBB intersecta a regiao.
 *
 * Poda: se o MBB do no nao intersecta a regiao, a sub-arvore inteira
 * e ignorada (nao desce). Isso e a poda exigida pelo enunciado para
 * os comandos lr, d e mc.
 */
static void buscaRegiaoNo(ArvImpl *arv, const No *raiz, void *ctx,
                          int (*intersect)(double, double, double, double, void *),
                          FuncaoVisita visita, void *aux)
{
    if (raiz == NULL)
        return;

    arv->contador_intersecoes++;
    arv->contador_nos_visitados++;

    /* PODA: se o MBB do no nao intersecta a regiao, nao desce */
    if (!intersect(raiz->mbx1, raiz->mby1, raiz->mbx2, raiz->mby2, ctx))
        return;

    buscaRegiaoNo(arv, raiz->esq, ctx, intersect, visita, aux);
    visita(raiz->info, aux);
    buscaRegiaoNo(arv, raiz->dir, ctx, intersect, visita, aux);
}

void vermelha_busca_regiao(void *arvore, void *ctx,
                           int (*intersect)(double, double, double, double, void *),
                           FuncaoVisita visita, void *aux)
{
    if (arvore == NULL || intersect == NULL || visita == NULL)
        return;

    ArvImpl *arv = (ArvImpl *)arvore;
    buscaRegiaoNo(arv, arv->raiz, ctx, intersect, visita, aux);
}

/*
 * ============================================================
 * INSPECAO: TAMANHO, ALTURA E VALIDADE RUBRO-NEGRO
 * ============================================================
 */

static int contarNo(const No *raiz)
{
    if (raiz == NULL)
        return 0;
    return 1 + contarNo(raiz->esq) + contarNo(raiz->dir);
}

int vermelha_tamanho(void *arvore)
{
    if (arvore == NULL)
        return 0;
    ArvImpl *arv = (ArvImpl *)arvore;
    return contarNo(arv->raiz);
}

static int alturaNo(const No *raiz)
{
    if (raiz == NULL)
        return 0;
    int e = alturaNo(raiz->esq);
    int d = alturaNo(raiz->dir);
    return 1 + ((e > d) ? e : d);
}

int vermelha_altura(void *arvore)
{
    if (arvore == NULL)
        return 0;
    ArvImpl *arv = (ArvImpl *)arvore;
    return alturaNo(arv->raiz);
}

/**
 * verificarPropriedades – checa as propriedades Rubro-Negro.
 *
 * Retorna:
 *   0  se todas as propriedades valem.
 *  -1  se existe no vermelho com filho vermelho.
 *  -2  se alturas negras divergem.
 */
static int verificarPropriedades(const No *raiz, int *alturaNegra)
{
    if (raiz == NULL)
    {
        *alturaNegra = 1;
        return 0;
    }

    int he = 0, hd = 0;
    int re = verificarPropriedades(raiz->esq, &he);
    int rd = verificarPropriedades(raiz->dir, &hd);

    if (re != 0)
        return re;
    if (rd != 0)
        return rd;

    /* no vermelho nao pode ter filho vermelho */
    if (raiz->cor == VERMELHO &&
        (ehVermelho(raiz->esq) || ehVermelho(raiz->dir)))
        return -1;

    /* mesma altura negra nos dois lados */
    if (he != hd)
        return -2;

    *alturaNegra = he + ((raiz->cor == PRETO) ? 1 : 0);
    return 0;
}

int vermelha_valida(void *arvore)
{
    if (arvore == NULL)
        return 0;

    ArvImpl *arv = (ArvImpl *)arvore;
    if (arv->raiz == NULL)
        return 1;

    /* 1) raiz preta */
    if (arv->raiz->cor != PRETO)
        return 0;

    int alturaNegra = 0;
    return verificarPropriedades(arv->raiz, &alturaNegra) == 0;
}

/* estatisticas da poda (para testes/demonstracao) */
void vermelha_reset_estatisticas(void *arvore)
{
    if (arvore == NULL)
        return;
    ArvImpl *arv = (ArvImpl *)arvore;
    arv->contador_intersecoes = 0;
    arv->contador_nos_visitados = 0;
}

int vermelha_get_intersecoes(void *arvore)
{
    ArvImpl *arv = (ArvImpl *)arvore;
    return arv ? arv->contador_intersecoes : 0;
}

/*
 * ============================================================
 * DESTRUICAO
 * ============================================================
 */

static void destruirNo(No *raiz, void (*destruir)(void *))
{
    if (raiz == NULL)
        return;
    destruirNo(raiz->esq, destruir);
    destruirNo(raiz->dir, destruir);
    if (destruir != NULL)
        destruir(raiz->info);
    free(raiz);
}

void vermelha_destroi(void *arvore, void (*destruir)(void *))
{
    if (arvore == NULL)
        return;
    ArvImpl *arv = (ArvImpl *)arvore;
    destruirNo(arv->raiz, destruir);
    free(arv);
}

/*
 * ============================================================
 * EXPORTACAO DOT (estado final da arvore)
 * ============================================================
 */

/**
 * exportaDotNo – escreve os nos e arestas no arquivo dot.
 *
 * A cor do no no arquivo reflete a cor real (RED/BLACK) da RB.
 */
static void exportaDotNo(FILE *fp, const No *raiz,
                         int (*getid)(const void *elem))
{
    if (raiz == NULL)
        return;

    int id = getid ? getid(raiz->info) : (int)(long)(void *)raiz;
    const char *cor = (raiz->cor == VERMELHO) ? "red" : "black";

    fprintf(fp, "  n%d [label=\"%d\", color=%s, fontcolor=%s];\n",
            id, id, cor, cor);

    if (raiz->esq != NULL)
    {
        int idE = getid ? getid(raiz->esq->info) : (int)(long)(void *)raiz->esq;
        fprintf(fp, "  n%d -> n%d;\n", id, idE);
        exportaDotNo(fp, raiz->esq, getid);
    }
    if (raiz->dir != NULL)
    {
        int idD = getid ? getid(raiz->dir->info) : (int)(long)(void *)raiz->dir;
        fprintf(fp, "  n%d -> n%d;\n", id, idD);
        exportaDotNo(fp, raiz->dir, getid);
    }
}

void vermelha_exporta_dot(void *arvore, const char *nome_arquivo,
                          int (*getid)(const void *elem))
{
    if (arvore == NULL || nome_arquivo == NULL)
        return;

    ArvImpl *arv = (ArvImpl *)arvore;
    FILE *fp = fopen(nome_arquivo, "w");
    if (fp == NULL)
        return;

    fprintf(fp, "digraph ArvoreRubroNegra {\n");
    fprintf(fp, "  node [shape=circle, style=filled, fillcolor=white];\n");
    if (arv->raiz != NULL)
        exportaDotNo(fp, arv->raiz, getid);
    fprintf(fp, "}\n");
    fclose(fp);
}

void *vermelha_cria(FuncaoComparacao comp, FuncaoGetMbb getmbb)
{
    if (comp == NULL || getmbb == NULL)
        return NULL;

    ArvImpl *arv = (ArvImpl *)malloc(sizeof(ArvImpl));
    if (arv == NULL)
        return NULL;

    arv->raiz = NULL;
    arv->comp = comp;
    arv->getmbb = getmbb;
    arv->contador_intersecoes = 0;
    arv->contador_nos_visitados = 0;
    return (void *)arv;
}