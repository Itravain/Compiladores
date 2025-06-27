# test_assembler.py
import pytest
from assembler import Assembler

def test_parse_simple_instruction():
    """Testa o parse de uma instrução com 3 registradores."""
    line = "ADD R1, R2, R3"
    assembler = Assembler()
    
    # Esperamos um dicionário estruturado
    expected = {
        'type': 'instruction',
        'mnemonic': 'ADD',
        'operands': ['R1', 'R2', 'R3']
    }
    
    result = assembler.parse_line(line)
    assert result == expected

def test_parse_immediate_instruction():
    """Testa instruções com imediato"""
    assembler = Assembler()
    line = "ADDI R1, R2, #3"
    
    expected = {
        'type': 'instruction',
        'mnemonic': 'ADDI',
        'operands': ['R1', 'R2', 3]
    }
    result = assembler.parse_line(line)
    assert result == expected

def test_parse_mov_immediate_instruction():
    """Testa instruções com imediato"""
    assembler = Assembler()
    line = "MOV R1, #3"

    expected = {
        'type': 'instruction',
        'mnemonic': 'MOV',
        'operands': ['R1', 3]
    }
    result = assembler.parse_line(line)
    assert result == expected

def test_traduzir_instrucao_add():
    """Teste de tradução de uma instrução add."""
    assembler = Assembler()
    
    line = "ADD R1, R2, R3"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100000011000010000010001100000"
    result = assembler.translate_instruction(parsed_line, 0)
    assert result == expected_binary

def test_traduzir_instrucao_addi():
    """Teste de tradução de uma instrução addi."""
    assembler = Assembler()
    
    line = "ADDI R1, R2, #1"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100010011000010000010000000001"
    result = assembler.translate_instruction(parsed_line, 0)
    assert result == expected_binary

def test_traduzir_instrucao_sub():
    """Teste de tradução de uma instrução sub."""
    assembler = Assembler()
    
    line = "SUB R5, R6, R7"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100000010000110001010011100000"
    result = assembler.translate_instruction(parsed_line, 0)
    assert result == expected_binary

def test_traduzir_instrucao_mult():
    """Teste de tradução de uma instrução MUL."""
    assembler = Assembler()
    
    line = "MUL R7, R6, R7"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100001010000110001110011100000"
    result = assembler.translate_instruction(parsed_line, 0)
    assert result == expected_binary

def test_traduzir_instrucao_mov():
    """Teste de tradução de uma instrução MOV."""
    assembler = Assembler()
    
    line = "MOV R7, R6"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100001001000000001110011000000"
    result = assembler.translate_instruction(parsed_line, 0)
    assert result == expected_binary

def test_traduzir_instrucao_movi():
    """Teste de tradução de uma instrução MOVI."""
    assembler = Assembler()
    
    line = "MOVI R7, #10"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100011001000000001110000001010"
    result = assembler.translate_instruction(parsed_line, 0)
    assert result == expected_binary

def test_traduzir_instrucao_udiv():
    """Testes de tradução para instrução UDIV"""
    assembler = Assembler()

    line = "UDIV R7, R0, R6"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100001011000000001110011000000"
    result = assembler.translate_instruction(parsed_line, 0)
    assert result == expected_binary

def test_parse_cmp_instruction():
    """Testa o parse de uma instrução CMP com registradores comuns."""
    assembler = Assembler()
    line = "CMP R1, R2"
    expected = {
        'type': 'instruction',
        'mnemonic': 'CMP',
        'operands': ['R1', 'R2']
    }
    result = assembler.parse_line(line)
    assert result == expected

def test_parse_cmp_with_special_registers():
    """Testa o parse de uma instrução CMP com registradores especiais."""
    assembler = Assembler()
    line = "CMP Rad, FP"
    expected = {
        'type': 'instruction',
        'mnemonic': 'CMP',
        'operands': ['Rad', 'FP']
    }
    result = assembler.parse_line(line)
    assert result == expected

def test_parse_b_instruction():
    """Testa o parse de uma instrução B com registrador comum."""
    assembler = Assembler()
    line = "B R3"
    expected = {
        'type': 'instruction',
        'mnemonic': 'B',
        'operands': ['R3']
    }
    result = assembler.parse_line(line)
    assert result == expected

def test_parse_b_with_special_register():
    """Testa o parse de uma instrução B com registrador especial."""
    assembler = Assembler()
    line = "B FP"
    expected = {
        'type': 'instruction',
        'mnemonic': 'B',
        'operands': ['FP']
    }
    result = assembler.parse_line(line)
    assert result == expected

def test_traduzir_instrucao_b():
    """Teste de tradução de uma instrução B."""
    assembler = Assembler()
    line = "B R3"
    parsed_line = assembler.parse_line(line)
    expected_binary = "11101000000110000000000000000000"
    result = assembler.translate_instruction(parsed_line, 0)
    assert result == expected_binary

def test_traduzir_instrucao_b_fp():
    """Teste de tradução de uma instrução B com registrador especial FP."""
    assembler = Assembler()
    line = "B FP"
    parsed_line = assembler.parse_line(line)
    expected_binary = "11101000110110000000000000000000"
    result = assembler.translate_instruction(parsed_line, 0)
    assert result
