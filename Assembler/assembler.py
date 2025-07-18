import re
import sys

debugar = True
debugar = False

class Assembler:
    import re

class Assembler:
    def __init__(self):
        self.special_registers = {
            "Rret": 24, "Rad": 25, "SP": 26, "FP": 27, "Rin": 28, 
            "Rout": 29, "CPSR": 30, "Rlink": 31
        }
        self.symbol_table = {}

    def _get_operand_binary(self, operand, bits=5):
        """Converte um operando (registrador ou imediato) para binário."""
        
        numeric_value = 0

        if isinstance(operand, str):
            # Se for uma string, verifica se é um registrador especial ou normal
            if operand in self.special_registers:
                numeric_value = self.special_registers[operand]
            else:
                numeric_value = int(operand[1:])
                
        elif isinstance(operand, int):
            # Garante o complemento de dois para números negativos
            # Cria uma máscara para o número de bits desejado (ex: 24 bits -> 0xFFFFFF)
            mask = (1 << bits) - 1
            # Aplica a máscara. Para números positivos, não faz nada.
            # Para negativos, calcula o complemento de dois.
            numeric_value = operand & mask
            
        else:
            # Lança um erro se o tipo do operando for inesperado
            raise TypeError(f"Tipo de operando inválido: {type(operand)}")

        return f'{numeric_value:0{bits}b}'

    #Adicione o parâmetro 'current_address' à definição do método.
    def translate_instruction(self, parsed_line, current_address):
        mnemonic = parsed_line['mnemonic']
        operands = parsed_line['operands']

        try:
            if mnemonic == "ADD":
                final_binary = [
                    '1110', '00', '0', '0101', '0',
                    self._get_operand_binary(operands[1]),  # rn
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[2]),  # rm
                    '00000'
                ]
            elif mnemonic == "ADDI":
                final_binary = [
                    '1110', '00', '1', '0101', '0',
                    self._get_operand_binary(operands[1]),  # rn
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[2], 10)  # imm
                ]
            elif mnemonic == "SUB":
                final_binary = [
                    '1110', '00', '0', '0011', '0',
                    self._get_operand_binary(operands[1]),  # rn
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[2]),  # rm
                    '00000'
                ]
            elif mnemonic == "SUBI":
                final_binary = [
                    '1110', '00', '1', '0011', '0',
                    self._get_operand_binary(operands[1]),  # rn
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[2], 10)  # imm
                ]
            elif mnemonic == "MUL":
                final_binary = [
                    '1110', '00', '0', '1110', '0',
                    self._get_operand_binary(operands[1]),  # rn
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[2]),  # rm
                    '00000'
                ]
            elif mnemonic == "AND":
                final_binary = [
                    '1110', '00', '0', '0001', '0',
                    self._get_operand_binary(operands[1]),  # rn
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[2]),  # rm
                    '00000'
                ]
            elif mnemonic == "ANDI":
                final_binary = [
                    '1110', '00', '1', '0001', '0',
                    self._get_operand_binary(operands[1]),  # rn
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[2], 10)  # imm
                ]
            elif mnemonic == "ORR":
                final_binary = [
                    '1110', '00', '0', '1100', '0',
                    self._get_operand_binary(operands[1]),  # rn
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[2]),  # rm
                    '00000'
                ]
            elif mnemonic == "ORRI":
                final_binary = [
                    '1110', '00', '1', '1100', '0',
                    self._get_operand_binary(operands[1]),  # rn
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[2], 10)  # imm
                ]
            elif mnemonic == "MOV":
                final_binary = [
                    '1110', '00', '1', '1101', '0',
                    self._get_operand_binary(operands[1]),  # rm
                    self._get_operand_binary(operands[0]),  # rd
                    '00000',  # rn não usado
                    '00000'
                ]
            elif mnemonic == "MOVI":
                final_binary = [
                    '1110', '00', '1', '1101', '0',
                    '00000',  # rn não usado
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[1], 10)  # imm
                ]
            elif mnemonic == "LDR":
                # Exemplo: LDR R1, [R2, #4] -> operands = ['R1', 'R2', 4]
                final_binary = [
                    '1110', '01', '1', '0', '1',
                    self._get_operand_binary(operands[1], 5),  # rn (base)
                    self._get_operand_binary(operands[0], 5),  # rd (dest)
                    self._get_operand_binary(operands[2], 13)  # imm
                ]
            elif mnemonic == "STR":
                # Exemplo: STR R1, [R2, #4] -> operands = ['R1', 'R2', 4]
                final_binary = [
                    '1110', '01', '1', '0', '0',
                    self._get_operand_binary(operands[1], 5),  # rn (base)
                    self._get_operand_binary(operands[0], 5),  # rm (src)
                    self._get_operand_binary(operands[2], 13)  # imm
                ]
        
            # Lógica de salto para B e BL usando endereço absoluto.
            elif mnemonic == "B" or mnemonic == "BL":
                target_label = operands[0]
                
                # Se o operando for um registrador, mantenha a lógica antiga (salto para endereço em registrador).
                if target_label.startswith("R") or target_label in self.special_registers:
                    link_bit = '1' if mnemonic == 'BL' else '0'
                    final_binary = [
                        '1110', '10', '0', link_bit,
                        self._get_operand_binary(target_label),  # rn
                        '0'*19
                    ]
                # Se for um label, use o endereço absoluto da tabela de símbolos.
                else:
                    # Obtém o endereço absoluto (número da instrução) do label.
                    target_address = self.symbol_table[target_label]
                    
                    link_bit = '1' if mnemonic == 'BL' else '0'
                    # Formato para Branch com Imediato (endereço absoluto)
                    final_binary = [
                        '1110', '10', '1', link_bit,
                        self._get_operand_binary(target_address, 24)  # imm24
                    ]

            elif mnemonic == "BLE" or mnemonic == "BGE" or mnemonic == "BNE":
                target_label = operands[0]
                # Verifica condições do CPSR
                if mnemonic == "BNE":
                    cond_code = '0001'
                elif mnemonic == "BGE":
                    cond_code = '1010'
                elif mnemonic == "BLE":
                    cond_code = '1101'
                else:
                    raise ValueError(f"Condição desconhecida: {mnemonic}")

                if target_label.startswith("R") or target_label in self.special_registers:
                    # Branch condicional para endereço em registrador
                    final_binary = [
                        cond_code, '10', '0', '0',
                        self._get_operand_binary(target_label),
                        '0'*19
                    ]
                else:
                    # Branch condicional para endereço em label (imediato)
                    target_address = self.symbol_table[target_label]
                    final_binary = [
                        cond_code, '10', '1', '0', # L=0 para branches condicionais
                        self._get_operand_binary(target_address, 24)
                    ]
    

            elif mnemonic == "BI":
                # Exemplo: BI #42
                final_binary = [
                    '1110', '10', '1', '0',
                    self._get_operand_binary(operands[0], 24)  # imm
                ]
            elif mnemonic == "NOP":
                final_binary = ['1110', '11', '000', '0'*23]
            elif mnemonic == "IN":
                final_binary = ['1110', '11', '001', '0'*23]
            elif mnemonic == "OUT":
                final_binary = ['1110', '11', '010', '0'*23]
            elif mnemonic == "FINISH":
                final_binary = ['1110', '11', '011', '0'*23]
            elif mnemonic == "CMP":
                # Formato: Cond[31:28] 00 0 0111 1 Rn[19:15] xxxxx Rm[9:5] xxxxx
                final_binary = [
                    '1110', '00', '0', '1010', '1',
                    self._get_operand_binary(operands[0]),  # rn
                    '00000',                               # rd não usado
                    self._get_operand_binary(operands[1]), # rm
                    '00000'                                # xxxxx
                ]
            elif mnemonic == "UDIV":
                # Formato: Cond[31:28] 00 0 1011 0 Rn[19:15] Rd[14:10] Rm[9:5] xxxxx
                final_binary = [
                    '1110', '00', '0', '1111', '0',
                    self._get_operand_binary(operands[1]),  # rn
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[2]),  # rm
                    '00000'                                 # xxxxx
                ]
            elif mnemonic == "UDIVI":
                # Formato: Cond[31:28] 00 0 1011 0 Rn[19:15] Rd[14:10] Rm[9:5] xxxxx
                final_binary = [
                    '1110', '00', '1', '1111', '0',
                    self._get_operand_binary(operands[1]),  # rn
                    self._get_operand_binary(operands[0]),  # rd
                    self._get_operand_binary(operands[2], 10)  # imm
                ]
            else:
                # Erro para mnemônico não reconhecido
                raise ValueError(f"Instrução desconhecida ou não implementada: '{mnemonic}'")

            if(debugar):
                return " ".join(final_binary)
            else:
                return "".join(final_binary)

        except IndexError:
            # Captura erros como ADD R1, R2 (faltando um operando)
            raise ValueError(f"[Linha {current_address}] Erro de Sintaxe: Número incorreto de operandos para a instrução '{mnemonic}'. Recebido: {operands}")
        except KeyError as e:
            # Captura erros como B L_inexistente (label não encontrado na tabela)
            raise ValueError(f"[Linha {current_address}] Erro de Semântica: Label não definido '{e.args[0]}' usado na instrução '{mnemonic}'.")
        except Exception as e:
            # Captura outros erros e os relata com contexto
            raise RuntimeError(f"[Linha {current_address}] Erro inesperado ao traduzir '{mnemonic} {operands}': {e}")

    def parse_line(self, line):
        # Ignora comentários e espaços em branco no início/fim
        line = line.split(';')[0].strip()
        if not line:
            return None

        if ':' in line:
            parts = line.split(':', 1)
            label = parts[0].strip()
            rest = parts[1].strip()

            # Label simples (ex: "L0:", "main:")
            if not rest:
                return {'type': 'label', 'name': label}
            
            # Declaração de dados a ser ignorada (ex: "vet: .space 5")
            if rest.startswith('.'):
                return None

        # Ignora a diretiva .data
        if line == ".data":
            return None

        # Remove vírgulas, os[] e normaliza espaços para instruções
        line = re.sub(r'[,\[\]]', ' ', line).strip()

        # Diretivas de seção
        if line == ".text":
            return {'type': 'section', 'name': '.text'}

        # Trata labels que começam com ponto (ex: .main)
        if line.startswith('.') and ' ' not in line:
            return {'type': 'label', 'name': line.lstrip('.')}

        parts = line.split()
        if not parts:
            return None

        mnemonic = parts[0]
        raw_operands = parts[1:]

        operands = [int(op[1:]) if op.startswith('#') else op for op in raw_operands]
        return {
            'type': 'instruction',
            'mnemonic': mnemonic,
            'operands': operands
        }

assembler = Assembler()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python assembler.py <arquivo.asm>")
        sys.exit(1)

    arquivo = sys.argv[1]
    assembler = Assembler()
    
    # Primeira Passagem: Construir tabela de símbolos
    address = 0
    linhas_codigo = []
    with open(arquivo, 'r') as f:
        for i, linha in enumerate(f, 1):
            parsed = assembler.parse_line(linha)
            if not parsed:
                continue
            
            # Armazena a linha parseada com seu número original para mensagens de erro
            parsed['line_num'] = i
            linhas_codigo.append(parsed)

            if parsed['type'] == 'label':
                if parsed['name'] in assembler.symbol_table:
                    print(f"Aviso: Label '{parsed['name']}' redefinido na linha {i}.")
                assembler.symbol_table[parsed['name']] = address
            elif parsed['type'] == 'instruction':
                address += 1

    # Segunda Passagem: Gerar código binário
    address = 0
    for parsed in linhas_codigo:
        if parsed['type'] == 'instruction':
            try:
                binary_code = assembler.translate_instruction(parsed, address)
                if(debugar):
                    print(f"Endereço[{address}] (Linha {parsed['line_num']}): {parsed['mnemonic']} {parsed['operands']}")
                    print(f"\tBinário: {binary_code}")
                else:
                    print(binary_code)
                address += 1
            except Exception as e:
                print(f"ERRO FATAL: {e}", file=sys.stderr)
                sys.exit(1)