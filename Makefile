# Makefile para o Compilador C-

# Use '>' como prefixo de receita (em vez de TAB) e execute cada receita num único shell
.RECIPEPREFIX := >
.ONESHELL:

CC := gcc
CFLAGS := -g -Wall
LDFLAGS := -lfl

TARGET := compiler
SRC_DIR := src
OUTPUT_DIR := outputs
ASSEMBLER := Assembler/assembler.py

SRCS := $(SRC_DIR)/main.c \
        $(SRC_DIR)/arvore.c \
        $(SRC_DIR)/tabSimbolos.c \
        $(SRC_DIR)/analise_semantica.c \
        $(SRC_DIR)/codigo_intermediario.c \
        $(SRC_DIR)/gerador_assembly.c \
        $(SRC_DIR)/pilha.c \
        lex.yy.c \
        analise_sintatica.tab.c

OBJS := $(SRCS:.c=.o)

.PHONY: all clean compile

all: $(TARGET)

$(TARGET): $(OBJS)
> $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
> echo "Compilador '$(TARGET)' construído com sucesso."

# Bison gera .c e .h
analise_sintatica.tab.c analise_sintatica.tab.h: $(SRC_DIR)/analise_sintatica.y
> bison -d -o analise_sintatica.tab.c $(SRC_DIR)/analise_sintatica.y

# Flex depende do header do Bison
lex.yy.c: $(SRC_DIR)/lexical_analyser.l analise_sintatica.tab.h
> flex -o lex.yy.c $(SRC_DIR)/lexical_analyser.l

# Regra padrão para .o
%.o: %.c
> $(CC) $(CFLAGS) -c -o $@ $<

# DEPENDÊNCIAS GERADAS (garantem ordem correta)
# main.o precisa do header do Bison
$(SRC_DIR)/main.o: analise_sintatica.tab.h globals.h

# demais .o recompilam quando globals.h muda
$(SRC_DIR)/arvore.o \
$(SRC_DIR)/tabSimbolos.o \
$(SRC_DIR)/analise_semantica.o \
$(SRC_DIR)/codigo_intermediario.o \
$(SRC_DIR)/gerador_assembly.o \
$(SRC_DIR)/pilha.o: globals.h

# "Simula" um gcc: aceita INPUT=<arquivo.c-> e OUTPUT=<binário>
compile: all
> [ -n "$(INPUT)" ] || { echo "ERRO: use make compile INPUT=test_codes/fatorial.c [OUTPUT=outputs/fatorial.bin]"; exit 1; }
> mkdir -p $(OUTPUT_DIR)
> BASENAME=$$(basename -s .c $$(basename -s .c- "$(INPUT)"))
> OUTFILE="$(OUTPUT)"
> [ -n "$$OUTFILE" ] || OUTFILE="$(OUTPUT_DIR)/$$BASENAME.bin"
> echo "Compilando '$(INPUT)'..."
> ./$(TARGET) < "$(INPUT)"
> echo "Montando para '$$OUTFILE'..."
> python3 "$(ASSEMBLER)" "$(OUTPUT_DIR)/assembly.asm" > "$$OUTFILE"
> echo "OK: $$OUTFILE"

clean:
> echo "Limpando..."
> rm -f $(TARGET) $(OBJS) lex.yy.c analise_sintatica.tab.c analise_sintatica.tab.h
> rm -f $(OUTPUT_DIR)/*
> echo "Feito."