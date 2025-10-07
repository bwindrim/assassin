"""
Assassin MC6809 Assembler (Python rewrite)
Author: Brian Windrim (original C), rewritten in Python
"""
import sys
import argparse
from collections import defaultdict



class Block:
    def __init__(self, name, parent=None):
        self.name = name
        self.parent = parent
        self.symbols = {}
        self.children = []

class SymbolTable:
    def __init__(self):
        self.root = Block('ROOT')
        self.stack = [self.root]

    def begin_block(self, name):
        block = Block(name, self.stack[-1])
        self.stack[-1].children.append(block)
        self.stack.append(block)

    def end_block(self):
        if len(self.stack) > 1:
            self.stack.pop()
        else:
            raise Exception('Unmatched END directive')

    def add_symbol(self, name, value):
        self.stack[-1].symbols[name] = value

    def lookup(self, name):
        for block in reversed(self.stack):
            if name in block.symbols:
                return block.symbols[name]
        return None

class Assembler:
    def __init__(self, files, list_flag=False, gen_flag=False, obj_name=None):
        self.files = files
        self.list_flag = list_flag
        self.gen_flag = gen_flag
        self.obj_name = obj_name
        self.symbol_table = SymbolTable()
        self.lines = []
        self.errors = []
        self.origin = 0
        self.current_addr = 0
        self.binary = bytearray()

    # MC6809 opcode table: (mnemonic, addressing_mode) -> opcode
    OPCODES = {
        # LDA
        ('LDA', 'IMMEDIATE'): 0x86,
        ('LDA', 'DIRECT'):    0x96,
        ('LDA', 'EXTENDED'):  0xB6,
        ('LDA', 'INDEXED'):   0xA6,
        # STA
        ('STA', 'DIRECT'):    0x97,
        ('STA', 'EXTENDED'):  0xB7,
        ('STA', 'INDEXED'):   0xA7,
        # ADD
        ('ADD', 'IMMEDIATE'): 0x8B,
        ('ADD', 'DIRECT'):    0x9B,
        ('ADD', 'EXTENDED'):  0xBB,
        ('ADD', 'INDEXED'):   0xAB,
        # SUB
        ('SUB', 'IMMEDIATE'): 0x80,
        ('SUB', 'DIRECT'):    0x90,
        ('SUB', 'EXTENDED'):  0xB0,
        ('SUB', 'INDEXED'):   0xA0,
        # JMP
        ('JMP', 'EXTENDED'):  0x7E,
        ('JMP', 'INDEXED'):   0x6E,
        # JSR
        ('JSR', 'EXTENDED'):  0xBD,
        ('JSR', 'INDEXED'):   0xAD,
        # LDX
        ('LDX', 'IMMEDIATE'): 0x8E,
        ('LDX', 'DIRECT'):    0x9E,
        ('LDX', 'EXTENDED'):  0xBE,
        ('LDX', 'INDEXED'):   0xAE,
        # STX
        ('STX', 'DIRECT'):    0x9F,
        ('STX', 'EXTENDED'):  0xBF,
        ('STX', 'INDEXED'):   0xAF,
        # LDY
        ('LDY', 'IMMEDIATE'): 0x10CE,
        ('LDY', 'DIRECT'):    0x109E,
        ('LDY', 'EXTENDED'):  0x10BE,
        ('LDY', 'INDEXED'):   0x10AE,
        # STY
        ('STY', 'DIRECT'):    0x109F,
        ('STY', 'EXTENDED'):  0x10BF,
        ('STY', 'INDEXED'):   0x10AF,
        # CMPA
        ('CMPA', 'IMMEDIATE'): 0x81,
        ('CMPA', 'DIRECT'):    0x91,
        ('CMPA', 'EXTENDED'):  0xB1,
        ('CMPA', 'INDEXED'):   0xA1,
        # CMPX
        ('CMPX', 'IMMEDIATE'): 0x8C,
        ('CMPX', 'DIRECT'):    0x9C,
        ('CMPX', 'EXTENDED'):  0xBC,
        ('CMPX', 'INDEXED'):   0xAC,
        # Short relativebranches (not used by default)
        ('SBRA', 'RELATIVE'):   0x20,
        ('SBRN', 'RELATIVE'):   0x21,
        ('SBHI', 'RELATIVE'):   0x22,
        ('SBLS', 'RELATIVE'):   0x23,
        ('SBCC', 'RELATIVE'):   0x24,
        ('SBHS', 'RELATIVE'):   0x24,
        ('SBCS', 'RELATIVE'):   0x25,
        ('SBLO', 'RELATIVE'):   0x25,
        ('SBNE', 'RELATIVE'):   0x26,
        ('SBEQ', 'RELATIVE'):   0x27,
        ('SBVC', 'RELATIVE'):   0x28,
        ('SBVS', 'RELATIVE'):   0x29,
        ('SBPL', 'RELATIVE'):   0x2A,
        ('SBMI', 'RELATIVE'):   0x2B,
        ('SBLS', 'RELATIVE'):   0x2C,
        ('SBLT', 'RELATIVE'):   0x2D,
        ('SBGT', 'RELATIVE'):   0x2E,
        ('SBLE', 'RELATIVE'):   0x2F,
        ('SBSR', 'RELATIVE'):   0x8D,
        # Long relative branches (default)
        ('LBRA', 'RELATIVE'):  0x16,
        ('LBRN', 'RELATIVE'):  0x1021,
        ('LBHI', 'RELATIVE'):  0x1022,
        ('LBLS', 'RELATIVE'):  0x1023,
        ('LBCC', 'RELATIVE'):  0x1024,
        ('LBHS', 'RELATIVE'):  0x1024,
        ('LBLO', 'RELATIVE'):  0x1025,
        ('LBCS', 'RELATIVE'):  0x1025,
        ('LBNE', 'RELATIVE'):  0x1026,
        ('LBEQ', 'RELATIVE'):  0x1027,
        ('LBVC', 'RELATIVE'):  0x1028,
        ('LBVS', 'RELATIVE'):  0x1029,
        ('LBPL', 'RELATIVE'):  0x102A,
        ('LBMI', 'RELATIVE'):  0x102B,
        ('LBGE', 'RELATIVE'):  0x102C,
        ('LBLT', 'RELATIVE'):  0x102D,
        ('LBGT', 'RELATIVE'):  0x102E,
        ('LBLE', 'RELATIVE'):  0x102F,
        ('LBSR', 'RELATIVE'):  0x17,
        # RTS/RTI
        ('RTS', 'INHERENT'):   0x39,
        ('RTI', 'INHERENT'):   0x3B,
        # LEAY/LEAX
        ('LEAY', 'INDEXED'):   0x31,
        ('LEAX', 'INDEXED'):   0x30,
        # LDU
        ('LDU', 'IMMEDIATE'):  0xCE,
        ('LDU', 'DIRECT'):     0xDE,
        ('LDU', 'EXTENDED'):   0xFE,
        ('LDU', 'INDEXED'):    0xEE,
        # STU
        ('STU', 'DIRECT'):     0xDF,
        ('STU', 'EXTENDED'):   0xFF,
        ('STU', 'INDEXED'):    0xEF,
        # EXG
        ('EXG', 'EXG'):        0x1E,
        # STB
        ('STB', 'DIRECT'):     0xD7,
        ('STB', 'EXTENDED'):   0xF7,
        ('STB', 'INDEXED'):    0xE7,
        # ANDA
        ('ANDA', 'IMMEDIATE'): 0x84,
        ('ANDA', 'DIRECT'):    0x94,
        ('ANDA', 'EXTENDED'):  0xB4,
        ('ANDA', 'INDEXED'):   0xA4,
        # LDB
        ('LDB', 'DIRECT'):     0xD6,
        ('LDB', 'EXTENDED'):   0xF6,
        ('LDB', 'INDEXED'):    0xE6,
        # ANDB
        ('ANDB', 'IMMEDIATE'): 0xC4,
        ('ANDB', 'DIRECT'):    0xD4,
        ('ANDB', 'EXTENDED'):  0xF4,
        ('ANDB', 'INDEXED'):   0xE4,
        # CMPB
        ('CMPB', 'IMMEDIATE'): 0xC1,
        ('CMPB', 'DIRECT'):    0xD1,
        ('CMPB', 'EXTENDED'):  0xF1,
        ('CMPB', 'INDEXED'):   0xE1,
    }

    # Map short branch mnemonics to long branch mnemonics
    BRANCH_MAP = {
        'BRA': 'LBRA',
        'BEQ': 'LBEQ',
        'BNE': 'LBNE',
        # Add more as needed
    }

    def parse_value(self, val):
        if isinstance(val, int):
            return val
        if isinstance(val, str):
            val = val.strip()
            if val.startswith('$'):
                return int(val[1:], 16)
            return int(val, 0)
        raise ValueError(f'Cannot parse value: {val}')

    def parse_file(self, filename, collect_symbols=False, generate_code=False):
        with open(filename) as f:
            for line in f:
                if collect_symbols:
                    self.parse_line_generate_code(line, generate_code=False)
                if generate_code:
                    self.parse_line_generate_code(line, generate_code=True)
                self.lines.append(line)

    def parse_line_generate_code(self, line, generate_code=True):
        line = line.strip()
        if not line or line.startswith(';'):
            return
        label = None
        if ':' in line:
            parts = line.split(':', 1)
            label = parts[0].strip()
            line = parts[1].strip() if len(parts) > 1 else ''
            if label:
#                if generate_code:
#                    print(f'LABEL: {label} @ {self.current_addr:04X}')
                self.symbol_table.add_symbol(label, self.current_addr)
        tokens = line.split()
        if not tokens:
            return
        directive = tokens[0].lower()
        if directive == 'org':
            if len(tokens) > 1:
                try:
                    self.origin = self.parse_value(tokens[1])
                    self.current_addr = self.origin
                    if generate_code:
                        print(f'ORG set to {self.origin:04X}')
                except ValueError:
                    self.errors.append(f'Invalid ORG value: {tokens[1]}')
            else:
                self.errors.append('ORG directive requires an address')
        elif directive == 'begin':
            pass
        elif directive == 'end':
            pass
        elif directive in ('db', 'defb'):
            bytes_out = []
            for val in tokens[1:]:
                try:
                    b = self.parse_value(val)
                    assert 0 <= b <= 255, f'Byte out of range: {b}'
                    bytes_out.append(b)
                    self.current_addr += 1
                except ValueError:
                    self.errors.append(f'Invalid byte value: {val}')
            if generate_code:
                self.binary.extend(bytes_out)
                self.print_listing(bytes_out, line)
        elif directive in ('dw', 'defw'):
            bytes_out = []
            for val in tokens[1:]:
                try:
                    w = self.parse_value(val)
                    hi = (w >> 8) & 0xFF
                    lo = w & 0xFF
                    assert 0 <= hi <= 255, f'Word high byte out of range: {hi}'
                    assert 0 <= lo <= 255, f'Word low byte out of range: {lo}'
                    bytes_out.append(hi)
                    bytes_out.append(lo)
                    self.current_addr += 2
                except ValueError:
                    self.errors.append(f'Invalid word value: {val}')
            if generate_code:
                self.binary.extend(bytes_out)
                self.print_listing(bytes_out, line)
        elif directive in ('ds', 'defs'):
            bytes_out = [0] * self.parse_value(tokens[1]) if len(tokens) > 1 else []
            self.current_addr += len(bytes_out)
            if generate_code:
                self.binary.extend(bytes_out)
                self.print_listing(bytes_out, line)
        elif '=' in line:
            if generate_code:
                self.print_listing([], line)
            parts = line.split('=')
            name = parts[0].strip()
            value = parts[1].strip()
            # Always try to store as integer if possible
            try:
                num_value = self.parse_value(value)
                self.symbol_table.add_symbol(name, num_value)
            except Exception:
                self.symbol_table.add_symbol(name, value)
        else:
            mnemonic = tokens[0].upper()
            # Map short branch to long branch
            if mnemonic in self.BRANCH_MAP:
                mnemonic = self.BRANCH_MAP[mnemonic]
            operands = tokens[1:] if len(tokens) > 1 else []
            # Determine addressing mode
            mode = None
            if not operands:
                mode = 'INHERENT'
            elif mnemonic in ('LBRA', 'LBEQ', 'LBNE'):
                mode = 'RELATIVE'
            elif operands[0].startswith('#'):
                mode = 'IMMEDIATE'
            elif operands[0].startswith('<'):
                mode = 'DIRECT'
            elif operands[0].startswith('>'):
                mode = 'EXTENDED'
            elif ',' in ' '.join(operands):
                if mnemonic == 'EXG':
                    mode = 'EXG'
                else:
                    mode = 'INDEXED'
            else:
                # Default to EXTENDED for absolute addresses
                mode = 'EXTENDED'

            # Use (mnemonic, mode) as key for OPCODES
            opcode = self.OPCODES.get((mnemonic, mode))
            bytes_out = []
            if opcode is not None:
                operands = tokens[1:] if len(tokens) > 1 else []
                operand_bytes = []
                if not operands:
                    mode = 'INHERENT'
                elif operands[0].startswith('#'):
                    mode = 'IMMEDIATE'
                    try:
                        val = operands[0][1:]
                        label_val = self.symbol_table.lookup(val)
                        if label_val is not None:
                            imm = self.parse_value(label_val)
                        else:
                            imm = self.parse_value(val)
                        # If loading to 16-bit register, output two bytes
                        if mnemonic in ('LDX', 'LDY', 'LDU', 'LDS', 'LDD'):
                            hi = (imm >> 8) & 0xFF
                            lo = imm & 0xFF
                            assert 0 <= hi <= 255, f'Immediate high byte out of range: {hi}'
                            assert 0 <= lo <= 255, f'Immediate low byte out of range: {lo}'
                            operand_bytes.append(hi)
                            operand_bytes.append(lo)
                        else:
                            assert 0 <= imm <= 255, f'Immediate byte out of range: {imm & 0xFF}'
                            operand_bytes.append(imm)
                    except ValueError:
                        self.errors.append(f'Invalid immediate value: {operands[0]}')
                elif operands[0].startswith('<') or operands[0].startswith('>'):
                    mode = 'DIRECT' if operands[0][0] == '<' else 'EXTENDED'
                    try:
                        val = operands[0][1:]
                        operand_bytes.append(self.parse_value(val))
                    except ValueError:
                        self.errors.append(f'Invalid direct/extended value: {operands[0]}')
                elif ',' in ' '.join(operands):
                    if mnemonic == 'EXG':
                        mode = 'EXG'
                        # EXG r1,r2: MC6809 register codes
                        reg_map = {
                            'D': 0x00, 'X': 0x01, 'Y': 0x02, 'U': 0x03, 'S': 0x04,
                            'PC': 0x05, 'A': 0x08, 'B': 0x09, 'CC': 0x0A, 'DP': 0x0B
                        }
                        try:
                            exg_parts = ' '.join(operands).split(',')
                            r1 = exg_parts[0].strip().upper()
                            r2 = exg_parts[1].strip().upper()
                            code = (reg_map.get(r1, 0) << 4) | reg_map.get(r2, 0)
                            operand_bytes.append(code)
                        except Exception:
                            self.errors.append(f'Invalid EXG operands: {" ".join(operands)}')
                    else:
                        mode = 'INDEXED'
                        # Support MC6809 auto-increment: ,X+ and ,X++
                        try:
                            idx_str = ' '.join(operands)
                            idx_parts = idx_str.split(',')
                            value = idx_parts[0].strip()
                            reg = idx_parts[1].strip().upper()
                            # Detect auto-increment
                            auto_inc = 0
                            if reg.endswith('++'):
                                reg = reg[:-2]
                                auto_inc = 1
                            elif reg.endswith('+'):
                                reg = reg[:-1]
                                auto_inc = 0
                            val = self.symbol_table.lookup(value)
                            if val is not None and value:
                                value_num = self.parse_value(val)
                            elif value:
                                value_num = self.parse_value(value)
                            else:
                                value_num = None
                            # MC6809 indexed mode: postbyte calculation
                            reg_codes = {'X': 0x00, 'Y': 0x20, 'U': 0x40, 'S': 0x60}
                            reg_code = reg_codes.get(reg, 0x00)
                            # Auto-increment: set LSB for ++, clear for +
                            reg_code |= auto_inc
                            if value_num is not None and value != '':
                                # Compact 5-bit signed offset (-16..15)
                                if -16 <= value_num <= 15:
                                    # MC6809: postbyte = offset (5 bits, signed) | reg_code
                                    offset = value_num & 0x1F
                                    operand_bytes.append(offset | reg_code)
                                else:
                                    # 16-bit offset: postbyte = 0x89 + offset
                                    operand_bytes.append(0x89 | reg_code)
                                    operand_bytes.append((value_num >> 8) & 0xFF)
                                    operand_bytes.append(value_num & 0xFF)
                            else:
                                # No offset: just the register postbyte
                                operand_bytes.append(0x80 | reg_code)
                        except Exception:
                            self.errors.append(f'Invalid indexed addressing: {" ".join(operands)}')
                else:
                    mode = 'ABSOLUTE'
                    val = self.symbol_table.lookup(operands[0])
                    try:
                        if val is not None:
                            v = self.parse_value(val)
                        else:
                            v = self.parse_value(operands[0])
                        # Always split 16-bit operands into two bytes
                        operand_bytes.append((v >> 8) & 0xFF)
                        operand_bytes.append(v & 0xFF)
                    except ValueError:
                        self.errors.append(f'Unknown label or value: {operands[0]}')
                # Output opcode and operands
                if opcode > 0xFF:
                    bytes_out.append((opcode >> 8) & 0xFF)
                    bytes_out.append(opcode & 0xFF)
                else:
                    bytes_out.append(opcode)
                # Special handling for long branches (LBRA, LBEQ, LBNE)
                if mnemonic in ('LBRA', 'LBEQ', 'LBNE'):
                    # Long branch: opcode is 1 or 2 bytes, operand is 2-byte signed offset (PC-relative)
                    # Calculate offset: target_addr - (current_addr + instruction_length)
                    # instruction_length = 3 or 4 (1 or 2 bytes opcode + 2 bytes offset)
                    target = operands[0] if operands else None
                    offset = 0
                    if target is not None:
                        # Try to resolve label or value
                        val = self.symbol_table.lookup(target)
                        if val is not None:
#                            print(f'LONG BRANCH to label {target} @ {val}')
                            target_addr = self.parse_value(val)
                        else:
#                            print(f'LONG BRANCH to value {target}')
                            try:
                                target_addr = self.parse_value(target)
                            except Exception:
                                target_addr = 0
                        offset = 0x10000 + target_addr - (self.current_addr + 2 + len(bytes_out))
#                        print(f'LONG BRANCH to {target} @ {target_addr:04X}, OFFSET: {offset}')
                        offset &= 0xFFFF  # 16-bit signed
                    # Emit 16-bit signed offset (big endian)
                    bytes_out.append((offset >> 8) & 0xFF)
                    bytes_out.append(offset & 0xFF)
                    if generate_code:
                        self.binary.extend(bytes_out)
                        self.print_listing(bytes_out, line)
                    self.current_addr += len(bytes_out)
                    return
                elif mnemonic in ('SBRA', 'SBEQ', 'SBNE'):
                    # Short branch: opcode is 1 byte, operand is 1-byte signed offset (PC-relative)
                    # Calculate offset: target_addr - (current_addr + instruction_length)
                    # instruction_length = 2 (1 byte opcode + 1 byte offset)
                    target = operands[0] if operands else None
                    offset = 0
                    if target is not None:
                        # Try to resolve label or value
                        val = self.symbol_table.lookup(target)
                        if val is not None:
#                            print(f'SHORT BRANCH to label {target} @ {val}')
                            target_addr = self.parse_value(val)
                        else:
#                            print(f'SHORT BRANCH to value {target}')
                            try:
                                target_addr = self.parse_value(target)
                            except Exception:
                                target_addr = 0
                        offset = target_addr - (self.current_addr + 1 + len(bytes_out))
#                        print(f'SHORT BRANCH to {target} @ {target_addr:04X}, OFFSET: {offset}')
                    # Emit 8-bit signed offset
                    assert -128 <= offset <= 127, f'Short branch offset out of range: {offset}'
                    bytes_out.append((offset + 0x100) & 0xFF)
                    if generate_code:
                        self.binary.extend(bytes_out)
                        self.print_listing(bytes_out, line)
                    self.current_addr += len(bytes_out)
                    return
                else:
                    bytes_out.extend(operand_bytes)
                if generate_code:
                    self.binary.extend(bytes_out)
                    self.print_listing(bytes_out, line)
                self.current_addr += len(bytes_out)
            else:
                if generate_code:
                    self.print_listing([], line)
                self.errors.append(f'Unknown instruction or directive: {mnemonic}')

    def print_listing(self, bytes_out, line):
        if self.list_flag:
            line_prefix = f'{self.current_addr:04X}:'
            if bytes_out:
                hex_bytes = ''.join(f'{b:02X}' for b in bytes_out)
                print(f'{line_prefix}{hex_bytes:<12} {line}')
            else:
                print(f'{line_prefix}      {line}')

    def assemble(self):
        # Pass 1: symbol resolution
        self.current_addr = 0
        for filename in self.files:
            self.parse_file(filename, collect_symbols=True, generate_code=False)
        # Pass 2: code generation with resolved symbols
        self.current_addr = 0
        self.binary = bytearray()
        for filename in self.files:
            self.parse_file(filename, collect_symbols=False, generate_code=True)
        # Write binary output if not suppressed
        if not self.gen_flag:
            if self.obj_name:
                out_name = self.obj_name
            else:
                # Default: input file with .a replaced by .ex9
                in_name = self.files[0]
                if in_name.endswith('.a'):
                    out_name = in_name[:-2] + '.ex9'
                else:
                    out_name = in_name + '.ex9'
            with open(out_name, 'wb') as f:
                f.write(self.binary)
            print(f'Object code written to {out_name}')

    def report(self):
        print(f'Assembly complete, {len(self.lines)} lines')
        if self.errors:
            print(f'{len(self.errors)} error(s):')
            for err in self.errors:
                print('  ', err)
        else:
            print('No errors.')

def main():
    parser = argparse.ArgumentParser(description='Assassin MC6809 Assembler (Python)')
    parser.add_argument('files', nargs='+', help='Assembly source files')
    parser.add_argument('-l', action='store_true', help='Generate listing')
    parser.add_argument('-n', action='store_true', help='Suppress object code generation')
    parser.add_argument('-o', metavar='OBJ', help='Object file name')
    args = parser.parse_args()

    assembler = Assembler(args.files, list_flag=args.l, gen_flag=args.n, obj_name=args.o)
    assembler.assemble()
    assembler.report()

if __name__ == '__main__':
    main()
