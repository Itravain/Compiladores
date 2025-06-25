import re

class Assembler:
    import re

class Assembler:
    def __init__(self):
        self.type_format_set = {
            'data_proc': ['cond', 'tipo_00', 'i_0', 'opcode', 's_0', 'rn', 'rd', 'rm', 'shift_0'],
            'data_proc_imm': ['cond', 'tipo_00', 'i_1', 'opcode', 's_0', 'rn', 'rd', 'Imm'],
            # Para instruções MOV
            'data_proc_1': ['cond', 'tipo_00', 'i_0', 'opcode', 's_0', 'shift_0', 'rd', 'rm', 'shift_0'],
            'data_proc_1_imm': ['cond', 'tipo_00', 'i_1', 'opcode', 's_0', 'shift_0', 'rd', 'Imm'],
            # Instruções TST, CMP

        }
        
        self.instruction_set = {
            'ADD': {
                'type': 'data_proc',
                'opcode': '0011',
                'format': self.type_format_set['data_proc']
            },
            'ADDI': {
                'type': 'data_proc_imm',
                'opcode': '0011',
                'format': self.type_format_set['data_proc_imm']
            },
            'SUB': {
                'type': 'data_proc',
                'opcode': '0010',
                'format': self.type_format_set['data_proc']
            },
            'MUL': {
                'type': 'data_proc',
                'opcode': '1010',
                'format': self.type_format_set['data_proc']
            },
            'UDIV': {
                'type': 'data_proc',
                'opcode': '1011',
                'format': self.type_format_set['data_proc']
            },
            'MOV': {
                'type': 'data_proc_1',
                'opcode': '1001',
                'format': self.type_format_set['data_proc_1']
            },
            'MOVI': {
                'type': 'data_proc_1_imm',
                'opcode': '1001',
                'format': self.type_format_set['data_proc_1_imm']
            },
            'CMP': {
                'type': 'data_proc_s',
                'opcode': '1001',
                'format': self.type_format_set['data_proc_1_imm']
            },

            
        }
        self.special_registers = {
            "Rad": 25, "Sp": 26, "Fp": 27, "Rin": 28, 
            "Rout": 29, "Cpsr": 30, "Rlink": 31
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
            numeric_value = operand
            
        else:
            # Lança um erro se o tipo do operando for inesperado
            raise TypeError(f"Tipo de operando inválido: {type(operand)}")

        return f'{numeric_value:0{bits}b}'

    def translate_instruction(self, parsed_line):
        mnemonic = parsed_line['mnemonic']
        operands = parsed_line['operands']

        if mnemonic not in self.instruction_set:
            raise ValueError(f"Instrução desconhecida: {mnemonic}")

        info = self.instruction_set[mnemonic]
        
        # Mapeia operandos para seus papéis
        if(info['type'] == 'data_proc'):
            op_map = {'rd': operands[0], 'rn': operands[1], 'rm': operands[2]}
            binary_parts = {
                'cond': '1110',
                'tipo_00': '00',
                'i_0': '0',
                'opcode': info['opcode'],
                's_0': '0',
                'rn': self._get_operand_binary(op_map['rn']),
                'rd': self._get_operand_binary(op_map['rd']),
                'rm': self._get_operand_binary(op_map['rm']),
                'shift_0': '00000'
            }
        elif(info['type'] == 'data_proc_1'):
            op_map = {'rd': operands[0], 'rm': operands[1]}
            binary_parts = {
                'cond': '1110',
                'tipo_00': '00',
                'i_0': '0',
                'opcode': info['opcode'],
                's_0': '0',
                'shift_0': '00000',
                'rd': self._get_operand_binary(op_map['rd']),
                'rm': self._get_operand_binary(op_map['rm']),
                'shift_0': '00000'
            }
        elif(info['type'] == 'data_proc_imm'):
            op_map = {'rd': operands[0], 'rn': operands[1], 'imm': operands[2]}
            binary_parts = {
                'cond': '1110',
                'tipo_00': '00',
                'i_1': '1',
                'opcode': info['opcode'],
                's_0': '0',
                'rn': self._get_operand_binary(op_map['rn']),
                'rd': self._get_operand_binary(op_map['rd']),
                'Imm': self._get_operand_binary(op_map['imm'], 10)
            }
        elif(info['type'] == 'data_proc_1_imm'):
            op_map = {'rd': operands[0], 'imm': operands[1]}
            binary_parts = {
                'cond': '1110',
                'tipo_00': '00',
                'i_1': '1',
                'opcode': info['opcode'],
                's_0': '0',
                'shift_0': '00000',
                'rd': self._get_operand_binary(op_map['rd']),
                'Imm': self._get_operand_binary(op_map['imm'], 10)
            }
        else:
            raise ValueError(f"Tipo de Instrução desconhecido: {info['type']}")
        

        # Monta a string final usando a ordem do formato
        final_binary = "".join([binary_parts[part] for part in info['format']])
        return final_binary

    
    def parse_line(self, line):
        # Ignora comentários e espaços em branco no início/fim
        line = line.split(';')[0].strip()
        if not line:
            return None

        # Remove vírgulas e normaliza espaços
        line = re.sub(r'\s*,\s*', ' ', line)
        parts = line.split()
        
        mnemonic = parts[0]
        raw_operands = parts[1:]
    
        operands = [int(op[1:]) if op.startswith('#') else op for op in raw_operands]
        return {
            'mnemonic': mnemonic,
            'operands': operands
        }
