/*
>>IMPLEMENTADOR: Julio Cesar da Silva Santos
>>DATA DE CRIAÇÃO: 15/09/26
>>LOCAL: U. Estadual de Londrina
*/

#include "texto.h"

Forma *texto_cria(int id, double x, double y,
                  const char *corb, const char *corp,
                  char a, const char *texto)
{
    return forma_cria_texto(id, x, y, corb, corp, a, texto);
}

char texto_get_ancora(const Forma *f)
{
    return forma_get_ancora(f);
}

const char *texto_get_string(const Forma *f)
{
    return forma_get_texto(f);
}

char texto_classifica(const Forma *f)
{
    return forma_classifica_texto(f);
}