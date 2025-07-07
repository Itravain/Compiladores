# Generate the lexical analyzer
flex src/lexical_analyser.l

# Generate the parser
bison -d src/analise_sintatica.y

# Compile the generated C code along with any additional source files
gcc -g -o compiler lex.yy.c analise_sintatica.tab.c src/arvore.c src/tabSimbolos.c src/analise_semantica.c globals.h src/codigo_intermediario.c src/gerador_assembly.c src/main.c src/pilha.c -lfl

# Create a directory to store the output files
if [ ! -d "outputs" ]; then
  mkdir outputs
fi

# Run the compiled program
# ./compiler < test_codes/_teste_1.c > outputs/output.txt 2>&1
# ./compiler < test_codes/_teste_2.c > outputs/output.txt 2>&1
# ./compiler < test_codes/_teste_3.c > outputs/output.txt 2>&1
# ./compiler < test_codes/_teste_4.c > outputs/output.txt 2>&1
# ./compiler < test_codes/_teste_5.c > outputs/output.txt 2>&1
# ./compiler < test_codes/gcd.c > outputs/output.txt 2>&1
./compiler < test_codes/sort.c > outputs/output.txt 2>&1
# ./compiler < test_codes/very_basic.c > outputs/output.txt 2>&1

python Assembler/assembler.py outputs/assembly.asm > outputs/bin.txt