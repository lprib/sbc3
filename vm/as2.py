#!/usr/bin/python3
from enum import Enum
import sys


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
    "+": 1,  # (n n -- n)
    "-": 2,
    "*": 3,
    "/": 4,
    # (n ip ip --)
    "if": 5,
    # (ip --)
    "jump": 6,
    # (ip --)
    "call": 7,
    "return": 8,
    ";": 8,
    "extern_call": 9,  # (moduleid fn# --)
    "loadword": 10,
    "@": 10,
    "storeword": 11,
    "!": 11,
    "push": 12,
    "load_module": 13,
    "loadbyte": 14,
    "*b": 14,
    "storebyte": 15,
    "!b": 15,
    "jump_imm": 16,
    "call_imm": 17,
    "dup": 18,
    "swap": 19,
    "drop": 20,
    "over": 21,
    "rot": 22,
    "dup2": 23,
    "swap2": 24,
    "over2": 25,
    "drop2": 26,
    "btrue": 27,
    "btrue_imm": 28,
    "bfalse": 29,
    "bfalse_imm": 30,
    ">4": 31,
    "rpush": 31,
    "r>": 32,
    "rpop": 32,
    "r@": 33,
    "rcopy": 33,
}


class Module:
    def __init__(self, text: str):
        self.program = bytearray()

        self.tracetext = ""
        self.labels = {}
        self.patchups = {}

        self.genlabel_counter = 0

        self.compile_lexer_contents(Lexer(text))

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
        elif tok == Token.BLOCK:
            pass
        elif tok == Token.ADDR_OF_WORD:
            # push immediate label value
            self.emit_opcode("push")
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
            self.emit_opcode("push")
            self.emit_short(f"short_imm: {intword}", value=intword)
            return
        except ValueError:
            pass

        if word in self.labels:
            self.emit_opcode("call_imm")
            self.register_patch_of_resolved_label_here(word)
            self.emit_short(f"call_target: {word}")
            return

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


if __name__ == "__main__":
    if len(sys.argv) != 2:
        exit(1)
    filetext = open(sys.argv[1]).read()

    m = Module(filetext)
    print(m.tracetext)
    for p in m.program:
        print(p)
