# test_assembler.py
import pytest
from assembler import Assembler

def test_parse_simple_instruction():
    """Testa o parse de uma instrução com 3 registradores."""
    line = "ADD R1, R2, R3"
    assembler = Assembler()
    
    # Esperamos um dicionário estruturado
    expected = {
        'mnemonic': 'ADD',
        'operands': ['R1', 'R2', 'R3']
    }
    
    result = assembler.parse_line(line)
    assert result == expected

def test_parse_immediate_instruction():
    """Testa instruções com imediato"""
    assembler = Assembler()
    line = "ADDI R1, R2, #3"
    parsed_line = assembler.parse_line(line)

    expected = {
        'mnemonic': 'ADDI',
        'operands': ['R1', 'R2', 3]
    }
    result = assembler.parse_line(line)
    assert result == expected

def test_parse_mov_immediate_instruction():
    """Testa instruções com imediato"""
    assembler = Assembler()
    line = "MOV R1, #3"
    parsed_line = assembler.parse_line(line)

    expected = {
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
    result = assembler.translate_instruction(parsed_line)
    assert result == expected_binary

def test_traduzir_instrucao_addi():
    """Teste de tradução de uma instrução add."""
    assembler = Assembler()
    
    line = "ADDI R1, R2, #1"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100010011000010000010000000001"
    result = assembler.translate_instruction(parsed_line)
    assert result == expected_binary

def test_traduzir_instrucao_sub():
    """Teste de tradução de uma instrução sub."""
    assembler = Assembler()
    
    line = "SUB R5, R6, R7"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100000010000110001010011100000"
    result = assembler.translate_instruction(parsed_line)
    assert result == expected_binary

def test_traduzir_instrucao_mult():
    """Teste de tradução de uma instrução MUL."""
    assembler = Assembler()
    
    line = "MUL R7, R6, R7"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100001010000110001110011100000"
    result = assembler.translate_instruction(parsed_line)
    assert result == expected_binary

def test_traduzir_instrucao_mov():
    """Teste de tradução de uma instrução MOV."""
    assembler = Assembler()
    
    line = "MOV R7, R6"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100001001000000001110011000000"
    result = assembler.translate_instruction(parsed_line)
    assert result == expected_binary

def test_traduzir_instrucao_movi():
    """Teste de tradução de uma instrução MOVI."""
    assembler = Assembler()
    
    line = "MOVI R7, #10"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100011001000000001110000001010"
    result = assembler.translate_instruction(parsed_line)
    assert result == expected_binary

def test_traduzir_instrucao_udiv():
    """Testes de tradução para instrução UDIV"""

    assembler = Assembler()

    line = "UDIV R7, R0, R6"
    parsed_line = assembler.parse_line(line)

    expected_binary = "11100001011000000001110011000000"
    result = assembler.translate_instruction(parsed_line)
    assert result == expected_binary   

def test_traduzir_instrucao_cmp():
    """Testes de tradução para instrução UDIV"""

    assembler = Assembler()

    line = "CMP R2, R3"
    parsed_line = assembler.parse_line(line)

    expected_binary = "1110001011110001000000 0001100000"
    result = assembler.translate_instruction(parsed_line)
    assert result == expected_binary   

  