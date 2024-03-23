#!/usr/bin/python3
from enum import Enum
import argparse


class Token(Enum):
    WORD = 1
    LABEL = 2
    SHORT_IMM = 3
    BYTE_IMM = 4
    STRING_IMM = 5
    MACRO = 6
    BLOCK = 7
    ADDR_OF_WORD = 8
    EOF = 9
    ERROR = 10


class LexerEof(Exception):
    pass


class Lexer:
    def __init__(self, text: str, starting_line=0):
        self.text = text
        self.idx = 0
        self.current_line = starting_line

    END_OF_WORD_CHARS = '#()"[]&'

    def peek(self, n=1) -> str:
        return self.text[self.idx : self.idx + n]

    def next(self, n=1) -> str:
        substr = self.peek(n)
        if "\n" in substr:
            self.current_line += 1
        self.idx += n
        if self.idx > len(self.text):
            raise LexerEof()
        return substr

    def advance(self, n=1):
        self.idx += n
        if self.idx > len(self.text):
            raise LexerEof()

    def error(self, msg) -> tuple[Token, tuple[str, int]]:
        return Token.ERROR, (msg, self.current_line + 1)

    def next_token(self) -> tuple[Token, any]:
        try:
            return self.next_token_inner()
        except LexerEof:
            return Token.EOF, None

    def next_token_inner(self) -> tuple[Token, any]:
        while True:
            n = self.next()
            if n.isspace():
                continue
            elif n == "(":
                commentlevel = 1
                while commentlevel:
                    n = self.next()
                    if n == "(":
                        commentlevel += 1
                    elif n == ")":
                        commentlevel -= 1
                continue
            elif n == "[":
                return self.next_block()
            elif n == '"':
                return self.next_string_literal()
            elif n == "#":
                if self.peek() == "b":
                    isbyte = True
                    self.advance()
                else:
                    isbyte = False

                if self.peek(2) == "0x":
                    self.advance(2)
                    return self.next_int_literal("0123456789abcdefABCDEF", 16, isbyte)
                else:
                    return self.next_int_literal("0123456789", 10, isbyte)
            elif n == "&":
                word = ""
                while True:
                    p = self.peek()
                    if (p in Lexer.END_OF_WORD_CHARS) or p.isspace():
                        return Token.ADDR_OF_WORD, word
                    word += self.next()
            else:
                word = n
                while True:
                    p = self.peek()
                    if p == "!":
                        self.advance()
                        return Token.MACRO, word
                    if p == ":":
                        self.advance()
                        return Token.LABEL, word
                    elif (p in Lexer.END_OF_WORD_CHARS) or p.isspace():
                        # end of word
                        return Token.WORD, word

                    word += self.next()

    def next_block(self) -> tuple[Token, any]:
        blocklevel = 1
        block = ""
        while True:
            nblk = self.next()
            if nblk == "[":
                blocklevel += 1
                block += nblk
            elif nblk == "]":
                blocklevel -= 1
                if not blocklevel:
                    # chances are they're going to want to lex the contents of
                    # this block, so might as well give them a Lexer straight
                    # off the bat
                    return Token.BLOCK, Lexer(block, starting_line=self.current_line)
                else:
                    block += nblk
            else:
                block += nblk

    def next_string_literal(self) -> tuple[Token, any]:
        string = ""
        nstr = self.next()
        while nstr != '"':
            string += nstr
            nstr = self.next()
        return Token.STRING_IMM, string

    def next_int_literal(self, allowed_chars, base, isbyte) -> tuple[Token, any]:
        string = ""
        nint = self.next()
        while nint in allowed_chars:
            string += nint
            nint = self.next()
        if len(string) != 0:
            byteval = int(string, base)
            max_exclusive = 256 if isbyte else 65536
            if (byteval < max_exclusive) and (byteval >= 0):
                token = Token.BYTE_IMM if isbyte else Token.SHORT_IMM
                return token, byteval
            else:
                return self.error(
                    f"literal must be nonnegative and less than ({max_exclusive}): {byteval}"
                )
        else:
            return self.error("expected number")


OPCODES = {
    "nop": 0,
    "+": 1,
    "-": 2,
    "*": 3,
    "/": 4,
    "%": 5,
    ">>": 6,
    "<<": 7,
    "~": 8,
    ">": 9,
    "<": 10,
    ">=": 11,
    "<=": 12,
    "==": 13,
    "!=": 14,
    "jump": 15,
    "jump_imm": 16,
    "call": 17,
    "call_imm": 18,
    "btrue": 19,
    "btrue_imm": 20,
    "bfalse": 21,
    "bfalse_imm": 22,
    "return": 23,
    ";": 23,  # alias
    "load_module": 24,
    "extern_call": 25,
    "loadword": 26,
    "@": 26,  # alias
    "storeword": 27,
    "!": 27,  # alias
    "push_imm": 28,
    "dup": 29,
    "swap": 30,
    "drop": 31,
    "over": 32,
    "rot": 33,
    "dup2": 34,
    "swap2": 35,
    "over2": 36,
    "drop2": 37, # why?
    "rpush": 38,
    ">r": 38,  # alias
    "rpop": 39,
    "r>": 39,  # alias
    "rcopy": 40,
    "r@": 40,  # alias
    # "loadbyte": 14,
    # "*b": 14,
    # "storebyte": 15,
    # "!b": 15,
}


class Module:
    def __init__(self, text: str):
        self.program = bytearray()
        self.tracetext = ""
        self.labels = {}
        self.patchups = {}
        self.genlabel_counter = 0
        self.module_name = None
        self.exports = []
        self.resolved_exports = {}

        lexer = Lexer(text)
        self.compile_lexer_contents(Lexer(text))

        print(self.tracetext)

        print("labels", self.labels)
        print("patchups", self.patchups)

        for patchup_location, labelname in self.patchups.items():
            if labelname in self.labels:
                location = self.labels[labelname]
                self.program[patchup_location] = location & 0xFF
                self.program[patchup_location + 1] = (location >> 8) & 0xFF
            else:
                print(f"undefined label {labelname}")
                exit(1)

        # look up the final resting place of our declared exported symbols
        for export in self.exports:
            if export in self.labels:
                self.resolved_exports[export] = self.labels[export]
            else:
                print(f"undefined label for export: {labelname}")
                exit(1)

    def bytecode(self) -> bytearray:
        bytecode = self.generate_header()
        bytecode.extend(self.program)
        return bytecode

    def compile_lexer_contents(self, lexer: Lexer):
        tok, data = lexer.next_token()
        while tok != Token.EOF:
            print(tok, data)
            self.compile_token(lexer, tok, data)
            tok, data = lexer.next_token()

    def compile_token(self, lexer: Lexer, tok: Token, data):
        if tok == Token.WORD:
            self.compile_word(data)
        elif tok == Token.LABEL:
            self.register_label_here(data)
        elif tok == Token.SHORT_IMM:
            self.trace(f"short_imm: {data}", 2)
            self.program.append(data & 0xFF)
            self.program.append((data >> 8) & 0xFF)
        elif tok == Token.BYTE_IMM:
            self.trace(f"byte_imm: {data}")
            self.program.append(data)
        elif tok == Token.STRING_IMM:
            self.trace(f"string_imm: {data}", len(data) + 1)
            for char in data:
                self.program.append(ord(char))
            self.program.append(0)
        elif tok == Token.MACRO:
            if data == "if":
                self.if_macro(lexer)
            elif data == "module_name":
                self.module_name_macro(lexer)
            elif data == "export":
                self.export_macro(lexer)
        elif tok == Token.BLOCK:
            pass
        elif tok == Token.ADDR_OF_WORD:
            # push immediate label value
            self.emit_opcode("push_imm")
            self.register_patch_of_resolved_label_here(data)
            self.emit_short(f"add_of_word: {data}")
        elif tok == Token.EOF:
            return
        elif tok == Token.ERROR:
            msg, line = data
            print(f"Error on line {line}: {msg}")
            exit(1)
        else:
            assert False

    def compile_word(self, word):
        if word.lower() in OPCODES:
            self.emit_opcode(word.lower())
            return

        try:
            intword = int(word)
            self.emit_opcode("push_imm")
            self.emit_short(f"short_imm: {intword}", value=intword)
            return
        except ValueError:
            pass

        if word in self.labels:
            self.emit_opcode("call_imm")
            self.register_patch_of_resolved_label_here(word)
            self.emit_short(f"call_target: {word}")
            return

    def module_name_macro(self, lexer: Lexer):
        tok, data = lexer.next_token()
        assert tok == Token.STRING_IMM
        self.module_name = data

    def export_macro(self, lexer: Lexer):
        tok, data = lexer.next_token()
        assert tok == Token.WORD
        self.exports.append(data)

    def if_macro(self, lexer: Lexer):
        ifblock_tok, ifblock_lexer = lexer.next_token()
        # todo need to associate span with each token so we can pinpoint errors
        assert ifblock_tok == Token.BLOCK
        elseblock_tok, elseblock_lexer = lexer.next_token()
        # todo need to associate span with each token so we can pinpoint errors
        assert elseblock_tok == Token.BLOCK

        # generate label names to not clash
        else_label = self.generate_label_name("else")
        end_label = self.generate_label_name("end")

        # branch false to else branch
        self.emit_opcode("bfalse_imm")
        self.register_patch_of_resolved_label_here(else_label)
        self.emit_short(f"branch_target: {else_label}")

        # paste if block
        self.compile_lexer_contents(ifblock_lexer)

        # at end of if block, jump over else block
        self.emit_opcode("jump_imm")
        self.register_patch_of_resolved_label_here(end_label)
        self.emit_short(f"branch_target: {end_label}")

        # paste else block
        self.register_label_here(else_label)
        self.compile_lexer_contents(elseblock_lexer)

        self.register_label_here(end_label)

    def register_patch_of_resolved_label_here(self, label_name):
        self.patchups[len(self.program)] = label_name

    def register_label_here(self, label_name):
        self.trace(label_name + ":")
        self.labels[label_name] = len(self.program)

    def emit_opcode(self, opcode):
        self.trace(opcode)
        self.program.append(OPCODES[opcode])

    # little endian
    def emit_short(self, tracetext, value=0):
        self.trace(tracetext, 2)
        self.program.append(value & 0xFF)
        self.program.append((value >> 8) & 0xFF)

    def trace(self, message, proglen=1):
        self.tracetext += f"{len(self.program)}"
        if proglen != 1:
            self.tracetext += f"-{len(self.program) + proglen - 1}"
        self.tracetext += f": {message}\n"

    def generate_label_name(self, name):
        self.genlabel_counter += 1
        return f"__generated_{name}_{self.genlabel_counter}"

    def generate_header(self):
        header = bytearray()
        if self.module_name is None:
            print("no module_name!")
            exit(1)
        header.append(len(self.module_name))
        header.extend(self.module_name.encode("ascii"))
        header.append(len(self.resolved_exports))
        for fn_name, fn_offset in self.resolved_exports.items():
            header.append(len(fn_name))
            header.extend(fn_name.encode("ascii"))
            header.append(fn_offset & 0xFF)
            header.append((fn_offset >> 8) & 0xFF)

        return header


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("input")
    parser.add_argument("output")

    args = parser.parse_args()

    filetext = open(args.input).read()

    m = Module(filetext)

    with open(args.output, "wb") as outfile:
        outfile.write(m.bytecode())
