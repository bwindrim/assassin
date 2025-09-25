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

    # MC6809 opcode table (mnemonic: opcode)
    OPCODES = {
        'LDA': 0x86,      # LDA opcode (immediate/direct/extended)
        'STA': 0x97,      # STA opcode (direct)
        'ADD': 0x8B,      # ADD opcode (immediate/direct/extended)
        'SUB': 0x80,      # SUB opcode (immediate/direct/extended)
        'JMP': 0x7E,      # JMP opcode (extended)
        'JSR': 0xBD,      # JSR opcode (extended)
        'LDX': 0xCE,      # LDX opcode (immediate/direct/extended)
        'STX': 0xFF,      # STX opcode (extended)
        'LDY': 0x10CE,    # LDY opcode (immediate/direct/extended)
        'STY': 0x10FF,    # STY opcode (extended)
        'CMPA': 0x81,     # CMPA opcode (immediate/direct/extended)
        'CMPX': 0x8C,     # CMPX opcode (immediate/direct/extended)
        'BRA': 0x20,      # BRA opcode (relative)
        'BEQ': 0x27,      # BEQ opcode (relative)
        'BNE': 0x26,      # BNE opcode (relative)
        'RTS': 0x39,      # RTS opcode (inherent)
        'RTI': 0x3B,      # RTI opcode (inherent)
        'LEAY': 0x31,     # LEAY opcode (indexed addressing)
        'LEAX': 0x30,     # LEAX opcode (indexed addressing)
        'LDU': 0xCE,      # LDU opcode (immediate/direct/extended)
        'EXG': 0x1E,      # EXG opcode (register-to-register)
        'STB': 0xD7,      # STB opcode (direct)
        'ANDA': 0x84,     # ANDA opcode (immediate/direct/extended)
        'LDB': 0xD6,      # LDB opcode (direct)
        'ANDB': 0xC4,     # ANDB opcode (immediate/direct/extended)
        'CMPB': 0xC1,     # CMPB opcode (immediate/direct/extended)
    }

    def parse_line(self, line):
        def parse_value(val):
            val = val.strip()
            if val.startswith('$'):
                return int(val[1:], 16)
            return int(val, 0)

        line = line.strip()
        if not line or line.startswith(';'):
            return

        # Label support: detect and store label before instruction/directive
        label = None
        if ':' in line:
            parts = line.split(':', 1)
            label = parts[0].strip()
            line = parts[1].strip() if len(parts) > 1 else ''
            if label:
                self.symbol_table.add_symbol(label, str(self.current_addr))
                print(f'LABEL: {label} @ {self.current_addr:04X}')

        tokens = line.split()
        if not tokens:
            return
        directive = tokens[0].lower()
        if directive == 'org':
            if len(tokens) > 1:
                try:
                    self.origin = parse_value(tokens[1])
                    self.current_addr = self.origin
                    print(f'ORG set to {self.origin:04X}')
                except ValueError:
                    self.errors.append(f'Invalid ORG value: {tokens[1]}')
            else:
                self.errors.append('ORG directive requires an address')
        elif directive == 'begin':
            name = tokens[1] if len(tokens) > 1 else 'BLOCK'
            self.symbol_table.begin_block(name)
        elif directive == 'end':
            try:
                self.symbol_table.end_block()
            except Exception as e:
                self.errors.append(str(e))
        elif directive in ('db', 'defb'):
            for val in tokens[1:]:
                try:
                    b = parse_value(val)
                    print(f'GENBYTE: {b:02X} @ {self.current_addr:04X}')
                    self.current_addr += 1
                except ValueError:
                    self.errors.append(f'Invalid byte value: {val}')
        elif directive in ('dw', 'defw'):
            for val in tokens[1:]:
                try:
                    w = parse_value(val)
                    print(f'GENWORD: {w:04X} @ {self.current_addr:04X}')
                    self.current_addr += 2
                except ValueError:
                    self.errors.append(f'Invalid word value: {val}')
        elif directive in ('ds', 'defs'):
            if len(tokens) > 1:
                try:
                    count = parse_value(tokens[1])
                    print(f'GENSPACE: {count} bytes @ {self.current_addr:04X}')
                    self.current_addr += count
                except ValueError:
                    self.errors.append(f'Invalid space value: {tokens[1]}')
        elif '=' in line:
            parts = line.split('=')
            name = parts[0].strip()
            value = parts[1].strip()
            self.symbol_table.add_symbol(name, value)
        else:
            mnemonic = tokens[0].upper()
            opcode = self.OPCODES.get(mnemonic)
            if opcode is not None:
                operands = tokens[1:] if len(tokens) > 1 else []
                mode = None
                operand_bytes = []
                if not operands:
                    mode = 'INHERENT'
                elif operands[0].startswith('#'):
                    mode = 'IMMEDIATE'
                    try:
                        val = operands[0][1:]
                        # Try to resolve as label first
                        label_val = self.symbol_table.lookup(val)
                        if label_val is not None:
                            operand_bytes.append(parse_value(label_val))
                        else:
                            operand_bytes.append(parse_value(val))
                    except ValueError:
                        self.errors.append(f'Invalid immediate value: {operands[0]}')
                elif operands[0].startswith('<') or operands[0].startswith('>'):
                    mode = 'DIRECT' if operands[0][0] == '<' else 'EXTENDED'
                    try:
                        val = operands[0][1:]
                        operand_bytes.append(parse_value(val))
                    except ValueError:
                        self.errors.append(f'Invalid direct/extended value: {operands[0]}')
                elif ',' in ' '.join(operands):
                    mode = 'INDEXED'
                else:
                    mode = 'ABSOLUTE'
                    val = self.symbol_table.lookup(operands[0])
                    if val is not None:
                        try:
                            operand_bytes.append(parse_value(val))
                        except ValueError:
                            self.errors.append(f'Invalid symbol value: {val}')
                    else:
                        try:
                            operand_bytes.append(parse_value(operands[0]))
                        except ValueError:
                            self.errors.append(f'Unknown label or value: {operands[0]}')
                print(f'INSTR: {mnemonic} {" ".join(operands)} | MODE: {mode} | OPCODE: {opcode:04X} | OPERANDS: {operand_bytes} @ {self.current_addr:04X}')
                self.current_addr += 1 + len(operand_bytes)
            else:
                self.errors.append(f'Unknown instruction or directive: {mnemonic}')

    def parse_file(self, filename, collect_symbols=False, generate_code=False):
        with open(filename) as f:
            for line in f:
                if collect_symbols:
                    self.parse_line_collect_symbols(line)
                if generate_code:
                    self.parse_line_generate_code(line)
                self.lines.append(line)

    def parse_line_collect_symbols(self, line):
        line = line.strip()
        if not line or line.startswith(';'):
            return
        # Label support: detect and store label before instruction/directive
        if ':' in line:
            parts = line.split(':', 1)
            label = parts[0].strip()
            if label:
                self.symbol_table.add_symbol(label, str(self.current_addr))
        tokens = line.split()
        if not tokens:
            return
        directive = tokens[0].lower()
        if directive == 'org':
            if len(tokens) > 1:
                try:
                    self.origin = int(tokens[1], 0)
                    self.current_addr = self.origin
                except ValueError:
                    pass
        elif directive in ('db', 'defb'):
            self.current_addr += len(tokens) - 1
        elif directive in ('dw', 'defw'):
            self.current_addr += 2 * (len(tokens) - 1)
        elif directive in ('ds', 'defs'):
            if len(tokens) > 1:
                try:
                    count = int(tokens[1], 0)
                    self.current_addr += count
                except ValueError:
                    pass
        elif directive == 'begin':
            pass
        elif directive == 'end':
            pass
        elif '=' in line:
            parts = line.split('=')
            name = parts[0].strip()
            value = parts[1].strip()
            self.symbol_table.add_symbol(name, value)
        else:
            self.current_addr += 1 # opcode
            # crude: add 1 for operand if present
            if len(tokens) > 1:
                self.current_addr += 1

    def parse_line_generate_code(self, line):
        def parse_value(val):
            val = val.strip()
            if val.startswith('$'):
                return int(val[1:], 16)
            return int(val, 0)

        line = line.strip()
        if not line or line.startswith(';'):
            return
        label = None
        if ':' in line:
            parts = line.split(':', 1)
            label = parts[0].strip()
            line = parts[1].strip() if len(parts) > 1 else ''
            if label:
                print(f'LABEL: {label} @ {self.current_addr:04X}')
        tokens = line.split()
        if not tokens:
            return
        directive = tokens[0].lower()
        if directive == 'org':
            if len(tokens) > 1:
                try:
                    self.origin = parse_value(tokens[1])
                    self.current_addr = self.origin
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
                    b = parse_value(val)
                    bytes_out.append(b)
                    self.current_addr += 1
                except ValueError:
                    self.errors.append(f'Invalid byte value: {val}')
            self.print_listing(bytes_out, line)
        elif directive in ('dw', 'defw'):
            bytes_out = []
            for val in tokens[1:]:
                try:
                    w = parse_value(val)
                    bytes_out.extend([(w >> 8) & 0xFF, w & 0xFF])
                    self.current_addr += 2
                except ValueError:
                    self.errors.append(f'Invalid word value: {val}')
            self.print_listing(bytes_out, line)
        elif directive in ('ds', 'defs'):
            bytes_out = [0] * parse_value(tokens[1]) if len(tokens) > 1 else []
            self.current_addr += len(bytes_out)
            self.print_listing(bytes_out, line)
        elif '=' in line:
            self.print_listing([], line)
            parts = line.split('=')
            name = parts[0].strip()
            value = parts[1].strip()
            self.symbol_table.add_symbol(name, value)
        else:
            mnemonic = tokens[0].upper()
            opcode = self.OPCODES.get(mnemonic)
            bytes_out = []
            if opcode is not None:
                operands = tokens[1:] if len(tokens) > 1 else []
                mode = None
                operand_bytes = []
                if not operands:
                    mode = 'INHERENT'
                elif operands[0].startswith('#'):
                    mode = 'IMMEDIATE'
                    try:
                        val = operands[0][1:]
                        label_val = self.symbol_table.lookup(val)
                        if label_val is not None:
                            imm = parse_value(label_val)
                        else:
                            imm = parse_value(val)
                        # If loading to 16-bit register, output two bytes
                        if mnemonic in ('LDX', 'LDY', 'LDU', 'LDS', 'LDD'):
                            operand_bytes.append((imm >> 8) & 0xFF)
                            operand_bytes.append(imm & 0xFF)
                        else:
                            operand_bytes.append(imm)
                    except ValueError:
                        self.errors.append(f'Invalid immediate value: {operands[0]}')
                elif operands[0].startswith('<') or operands[0].startswith('>'):
                    mode = 'DIRECT' if operands[0][0] == '<' else 'EXTENDED'
                    try:
                        val = operands[0][1:]
                        operand_bytes.append(parse_value(val))
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
                                auto_inc = 2
                            elif reg.endswith('+'):
                                reg = reg[:-1]
                                auto_inc = 1
                            val = self.symbol_table.lookup(value)
                            if val is not None and value:
                                value_num = parse_value(val)
                            elif value:
                                value_num = parse_value(value)
                            else:
                                value_num = None
                            # MC6809 indexed mode: offset byte, register code (X=0x84, Y=0xA4, U=0xC4, S=0xE4)
                            reg_codes = {'X': 0x84, 'Y': 0xA4, 'U': 0xC4, 'S': 0xE4}
                            reg_code = reg_codes.get(reg, 0x84)
                            # Auto-increment: add 0x10 for +, 0x20 for ++
                            reg_code += auto_inc * 0x10
                            if value_num is not None:
                                operand_bytes.append(value_num)
                            operand_bytes.append(reg_code)
                        except Exception:
                            self.errors.append(f'Invalid indexed addressing: {" ".join(operands)}')
                else:
                    mode = 'ABSOLUTE'
                    val = self.symbol_table.lookup(operands[0])
                    if val is not None:
                        try:
                            operand_bytes.append(parse_value(val))
                        except ValueError:
                            self.errors.append(f'Invalid symbol value: {val}')
                    else:
                        try:
                            operand_bytes.append(parse_value(operands[0]))
                        except ValueError:
                            self.errors.append(f'Unknown label or value: {operands[0]}')
                # Output opcode and operands
                if opcode > 0xFF:
                    bytes_out.append((opcode >> 8) & 0xFF)
                    bytes_out.append(opcode & 0xFF)
                else:
                    bytes_out.append(opcode)
                bytes_out.extend(operand_bytes)
                self.print_listing(bytes_out, line)
                self.current_addr += len(bytes_out)
            else:
                self.print_listing([], line)
                self.errors.append(f'Unknown instruction or directive: {mnemonic}')

    def print_listing(self, bytes_out, line):
        if self.list_flag:
            line_prefix = f'{self.current_addr:04X}: '
            if bytes_out:
                hex_bytes = ' '.join(f'{b:02X}' for b in bytes_out)
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
        for filename in self.files:
            self.parse_file(filename, collect_symbols=False, generate_code=True)

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
