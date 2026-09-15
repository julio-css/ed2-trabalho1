# >>IMPLEMENTADOR: Julio Cesar da Silva Santos
# >>DATA DE CRIAÇÃO: 15/09/26
# >>LOCAL: U. Estadual de Londrina

# ======================================================================
# Pesca Pirata - Trabalho de Estrutura de Dados II (ED2 - 2026)
# Makefile de compilacao do programa 'ted'
#
# Uso:
#   make saida     cria o diretorio ../output (obrigatorio pelo enunciado)
#   make ted       compila o executavel principal
#   make tst_*     compila e roda um teste unitario (ex.: make tst_vermelha)
#   make tstall    compila e roda todos os testes unitarios
#   make clean     remove artefatos de build
#
# Observacao: nesta maquina 'make' pode nao estar no PATH; use o alias
#   alias make='mingw32-make'
# ou invoque diretamente 'mingw32-make'.
# ======================================================================

# --- configuracao de compilacao (padrao C99 + flags exigidas) ---
CC      = gcc
CFLAGS  = -std=c99 -fstack-protector-all -ggdb -O0 -Wall
LDLIBS  = -lm

PROJ_NAME = ted

SRC   = .
TST   = ../tst
UNITY = ../unity
OUT   = ../output
INC   = -I$(SRC) -I$(UNITY)

# --- modulos (ordem de link) ---
OBJS = forma.o   \
       retangulo.o \
       circulo.o \
       linha.o   \
       texto.o   \
       lista.o   \
       fila.o    \
       vermelha.o \
       geo.o     \
       qry.o     \
       svg.o     \
       txt.o     \
       dot.o

# --- target 'saida': cria ./output no mesmo nivel de src (../output) ---
.PHONY: saida
saida:
	@mkdir -p $(OUT)
	@echo "Diretorio de saida criado: $(OUT)"

# --- executavel principal ---
ted: saida $(OBJS) main.o
	$(CC) $(CFLAGS) $(LDLIBS) -o $(PROJ_NAME) $(OBJS) main.o

main.o: main.c geo.h qry.h forma.h vermelha.h svg.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

forma.o:      forma.c      forma.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

retangulo.o:  retangulo.c  retangulo.h forma.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

circulo.o:    circulo.c    circulo.h   forma.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

linha.o:      linha.c      linha.h     forma.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

texto.o:      texto.c      texto.h     forma.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

lista.o:      lista.c      lista.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

fila.o:       fila.c       fila.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

vermelha.o:   vermelha.c   vermelha.h forma.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

geo.o:        geo.c        geo.h     forma.h vermelha.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

qry.o:        qry.c        qry.h     geo.h vermelha.h svg.h txt.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

svg.o:        svg.c        svg.h     forma.h lista.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

txt.o:        txt.c        txt.h     forma.h lista.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

dot.o:        dot.c        dot.h     vermelha.h forma.h
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

# --- testes unitarios (Unity) ---

$(TST)/t_lista: $(TST)/t_lista.c lista.c $(UNITY)/unity.c
	$(CC) $(CFLAGS) $(INC) $(UNITY)/unity.c lista.c $(TST)/t_lista.c -o $@

$(TST)/t_fila: $(TST)/t_fila.c fila.c $(UNITY)/unity.c
	$(CC) $(CFLAGS) $(INC) $(UNITY)/unity.c fila.c $(TST)/t_fila.c -o $@

$(TST)/t_forma: $(TST)/t_forma.c forma.c retangulo.c circulo.c linha.c texto.c $(UNITY)/unity.c
	$(CC) $(CFLAGS) $(INC) -DUNITY_INCLUDE_DOUBLE \
	$(UNITY)/unity.c forma.c retangulo.c circulo.c linha.c texto.c $(TST)/t_forma.c -o $@

$(TST)/t_vermelha: $(TST)/t_vermelha.c vermelha.c forma.c retangulo.c circulo.c linha.c texto.c $(UNITY)/unity.c
	$(CC) $(CFLAGS) $(INC) -DUNITY_INCLUDE_DOUBLE \
	$(UNITY)/unity.c vermelha.c forma.c retangulo.c circulo.c linha.c texto.c \
	$(TST)/t_vermelha.c -o $@

$(TST)/t_geo: $(TST)/t_geo.c geo.c forma.c retangulo.c circulo.c linha.c texto.c vermelha.c $(UNITY)/unity.c
	$(CC) $(CFLAGS) $(INC) -DUNITY_INCLUDE_DOUBLE \
	$(UNITY)/unity.c geo.c forma.c retangulo.c circulo.c linha.c texto.c vermelha.c \
	$(TST)/t_geo.c -o $@

$(TST)/t_qry: $(TST)/t_qry.c qry.c geo.c forma.c retangulo.c circulo.c linha.c texto.c vermelha.c svg.c txt.c dot.c lista.c fila.c $(UNITY)/unity.c
	$(CC) $(CFLAGS) $(INC) -DUNITY_INCLUDE_DOUBLE \
	$(UNITY)/unity.c qry.c geo.c forma.c retangulo.c circulo.c linha.c texto.c vermelha.c \
	svg.c txt.c dot.c lista.c fila.c $(TST)/t_qry.c -o $@

# --- execucao dos testes ---
tst_lista:     $(TST)/t_lista     ; ./$(TST)/t_lista
tst_fila:      $(TST)/t_fila      ; ./$(TST)/t_fila
tst_forma:     $(TST)/t_forma     ; ./$(TST)/t_forma
tst_vermelha:  $(TST)/t_vermelha  ; ./$(TST)/t_vermelha
tst_geo:       $(TST)/t_geo       ; ./$(TST)/t_geo
tst_qry:       $(TST)/t_qry       ; ./$(TST)/t_qry

tstall: tst_lista tst_fila tst_forma tst_vermelha tst_geo tst_qry

# --- limpeza ---
clean:
	rm -f *.o $(PROJ_NAME)
	rm -f $(TST)/t_lista $(TST)/t_fila $(TST)/t_forma $(TST)/t_vermelha \
	      $(TST)/t_geo $(TST)/t_qry

.PHONY: saida ted clean tstall \
        tst_lista tst_fila tst_forma tst_vermelha tst_geo tst_qry