#include "dot.h"

#include "vermelha.h"

void dot_exporta(void *arvore, int (*getid)(const void *elem), const char *arquivo)
{
    vermelha_exporta_dot(arvore, arquivo, getid);
}