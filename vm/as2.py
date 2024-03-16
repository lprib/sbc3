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
    def __init__(self, text: str):
        self.text = text
        self.idx = 0
        self.current_line = 0

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
                    return Token.BLOCK, block
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

        l = Lexer(text)
        tok, data = l.next_token()
        while tok != Token.EOF:
            print(tok, data)

            if tok == Token.WORD:
                self.compile_word(data)
            elif tok == Token.LABEL:
                self.trace(data + ":")
                self.labels[data] = len(self.program)
            elif tok == Token.SHORT_IMM:
                self.trace(f"SHORT_IMM: {data}", 2)
                self.program.append(data & 0xFF)
                self.program.append((data >> 8) & 0xFF)
            elif tok == Token.BYTE_IMM:
                self.trace(f"BYTE_IMM: {data}")
                self.program.append(data)
            elif tok == Token.STRING_IMM:
                self.trace(f"STRING_IMM: {data}", len(data)+1)
                for char in data:
                    self.program.append(ord(char))
                self.program.append(0)
            elif tok == Token.MACRO:
                pass
            elif tok == Token.BLOCK:
                pass
            elif tok == Token.ADDR_OF_WORD:
                # push immediate label value
                self.opcode("push")
                self.trace(f"ADD_OF_WORD: {data}", 2)
                self.patchups[len(self.program)] = data
                self.program.append(0)
                self.program.append(0)
            elif tok == Token.EOF:
                return
            elif tok == Token.ERROR:
                msg, line = data
                print(f"Error on line {line}: {msg}")
                exit(1)
            else:
                assert False

            tok, data = l.next_token()

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

    def compile_word(self, word):
        if word.lower() in OPCODES:
            self.opcode(word.lower())
            return

        try:
            intword = int(word)
            self.opcode("push")
            self.trace(f"SHORT_IMM: {intword}", 2)
            self.program.append(intword & 0xFF)
            self.program.append((intword >> 8) & 0xFF)
            return
        except ValueError:
            pass

        if word in self.labels:
            self.opcode("call_imm")
            self.trace(f"CALL_TARGET: {word}", 2)
            self.patchups[len(self.program)] = word
            self.program.append(0)
            self.program.append(0)
            return

    def opcode(self, opcode):
        self.trace(opcode)
        self.program.append(OPCODES[opcode])

    def trace(self, message, proglen=1):
        self.tracetext += f"{len(self.program)}"
        if proglen != 1:
            self.tracetext += f"-{len(self.program) + proglen - 1}"
        self.tracetext += f": {message}\n"


if __name__ == "__main__":
    if len(sys.argv) != 2:
        exit(1)
    filetext = open(sys.argv[1]).read()

    m = Module(filetext)
    print(m.tracetext)
    for p in m.program:
        print(p)
