# Compiladores · C- Compiler

[![Made with Flex & Bison](https://img.shields.io/badge/Made%20with-Flex%20%26%20Bison-1f6feb)](https://www.gnu.org/software/bison/)
[![Language](https://img.shields.io/badge/lang-C-blue.svg)](src/)
[![Build](https://img.shields.io/badge/build-Makefile-success)](Makefile)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](#license)

Compilador acadêmico para a linguagem C- com pipeline completo: Scanner (Flex), Parser (Bison), AST, Análise Semântica, TAC e Geração de Assembly + binário via montador Python.

<p align="center">
  <i>Tokens → Parser → AST → Semântica → TAC → Assembly → Binário</i>
</p>

---

## Sumário
- [Visão geral](#visão-geral)
- [Pipeline de compilação](#pipeline-de-compilação)
- [Recursos](#recursos)
- [Pré-requisitos](#pré-requisitos)
- [Uso rápido](#uso-rápido)
- [Comandos do Makefile](#comandos-do-makefile)
- [Exemplo de compilação](#exemplo-de-compilação)
- [Estrutura do repositório](#estrutura-do-repositório)
- [Detalhes de implementação](#detalhes-de-implementação)
- [Dicas e solução de problemas](#dicas-e-solução-de-problemas)
- [Contribuição](#contribuição)
- [Licença](#license)
- [Autores](#autores)

---

## Visão geral
Este projeto implementa um compilador para C-:
- Análise léxica com Flex.
- Análise sintática com Bison.
- Geração de AST e Tabela de Símbolos.
- Análise semântica (tipos, escopos, declarações).
- Geração de Código Intermediário (TAC).
- Geração de Assembly e montagem para binário.

---

## Pipeline de compilação

```mermaid
flowchart LR
    A[Fonte C-] --> B[Scanner (Flex)]
    B --> C[Parser (Bison)]
    C --> D[AST]
    D --> E[Análise Semântica<br/>Tabela de Símbolos]
    E --> F[TAC]
    F --> G[Assembly]
    G --> H[Montador Python]
    H --> I[Binário (bin/)]
```

---

## Recursos
- Suporte a variáveis globais/locais e parâmetros (incluindo arrays).
- Controle de fluxo: if/else, while, return.
- Expressões aritméticas e relacionais.
- TAC (Three-Address Code) com suporte a labels e saltos.
- Geração de assembly com acesso a pilha/FP/dados globais.
- Makefile com alvos práticos (compile, clean, tests, distclean).

---

## Pré-requisitos
- Linux, GNU Make
- gcc, flex, bison, python3
- Montador Python: [Assembler/assembler.py](Assembler/assembler.py)

Verificar ferramentas:
```bash
make check-tools
```

---

## Uso rápido
Compile o compilador:
```bash
make
```

Compile um programa C- e gere o binário em bin/:
```bash
make compile INPUT=test_codes/fatorial.c
# Saídas:
# - outputs/arvore.txt, tabsimb.txt, codInterm.txt
# - outputs/assembly.asm
# - bin/fatorial.bin
```

Escolher o nome do binário:
```bash
make compile INPUT=test_codes/_teste_5.c OUTPUT=bin/teste5.bin
```

Limpar:
```bash
make clean
# ou
make distclean  # limpa também gerados por Flex/Bison e .d
```

---

## Comandos do Makefile
| Alvo | Descrição |
|------|-----------|
| `make` / `make all` | Constrói o compilador (`compiler`). |
| `make compile INPUT=arquivo.c- [OUTPUT=bin/saida.bin]` | Compila um fonte C-, gera assembly e monta binário. |
| `make clean` | Remove binário do compilador, objetos e saídas. |

---

## Exemplo de compilação
Fonte de exemplo: [test_codes/fatorial.c](test_codes/fatorial.c)
```c
int fatorial(int numero){
    if (numero <= 1) return 1;
    return numero * fatorial(numero-1);
}
int main(){
    int a; 
    a = 5;
    a = fatorial(a);
    output(a);
}
```

Comando:
```bash
make compile INPUT=test_codes/fatorial.c
```

Saídas:
- TAC: [outputs/codInterm.txt](outputs/codInterm.txt)
- Assembly: [outputs/assembly.asm](outputs/assembly.asm)
- Binário: bin/fatorial.bin

Trecho do assembly (exemplo):
```asm
; início de main
PUSH FP
MOV FP, SP
ADDI SP, SP, #N
...
```

---

## Estrutura do repositório
```
.
├─ src/
│  ├─ main.c
│  ├─ arvore.c
│  ├─ tabSimbolos.c
│  ├─ analise_semantica.c
│  ├─ codigo_intermediario.c
│  ├─ gerador_assembly.c
│  └─ pilha.c
├─ Assembler/
│  └─ assembler.py
├─ test_codes/
├─ outputs/
├─ bin/
├─ globals.h
├─ analise_sintatica.y (Bison)
├─ lexical_analyser.l (Flex)
├─ Makefile
└─ README.md
```

---

## Detalhes de implementação
- AST: construída pelo parser ([src/main.c](src/main.c)) e impressa em [outputs/arvore.txt](outputs/arvore.txt).
- Tabela de símbolos: hash com escopos e offsets ([src/tabSimbolos.c](src/tabSimbolos.c)).
- TAC: gerado por [src/codigo_intermediario.c](src/codigo_intermediario.c) e salvo em [outputs/codInterm.txt](outputs/codInterm.txt).
- Assembly: gerado por [src/gerador_assembly.c](src/gerador_assembly.c).
- Considerações de registradores/stack para chamadas e recursão.

<details>
<summary>Tokens do scanner (resumo)</summary>

| Categoria | Exemplos |
|---|---|
| Palavras-chave | `if`, `else`, `while`, `int`, `void`, `return` |
| Operadores | `+ - * / % && || ! != == = >= <= > <` |
| Símbolos | `{ } ( ) [ ] ; , .` |
</details>

---



## Autores
- Eduardo Bouhid
- Ícaro Travain
