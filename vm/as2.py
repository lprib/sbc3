#!/usr/bin/python3
from enum import Enum


class Token(Enum):
    COMMENT = 0
    WORD = 1
    LABEL = 2
    SHORT_IMM = 3
    BYTE_IMM = 4
    STRING_IMM = 5
    MACRO = 6
    BLOCK = 7
    EOF = 8
    ERROR = 9


class LexerEof(Exception):
    pass


class Lexer:
    def __init__(self, text: str):
        self.text = text
        self.idx = 0
        self.current_line = 0

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
                    elif (p in '#()"#[]') or p.isspace():
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
            if (byteval < max_exclusive) and (byteval > 0):
                token = Token.BYTE_IMM if isbyte else Token.SHORT_IMM
                return token, byteval
            else:
                return self.error(
                    f"literal must be nonzero and less than ({max_exclusive})"
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


def compile(program: str):
    content = bytearray()

    labels = {}
    patchups = {}

    l = Lexer(program)
    tok, data = l.next_token()
    while tok != Token.EOF:
        print(tok, data)
        tok, data = l.next_token()

        if tok == Token.WORD:
            if data.lower() in OPCODES:
                content.append(OPCODES[data.lower()])


# def compile(program: str) -> bytearray:
#     pass
compile(
    """ (a (nested) comment) "strlksdf ing"
#1000
#b250
#0xAAFF
#b0x0F
(ignore mre)
a:word
234
mac!another
[[blo[b]ck]]wow
"""
)
