#!/usr/bin/python3
import sys

OPCODES = {
    "nop": 0,
    "add": 1, # (n n -- n)
    "sub": 2,
    "mul": 3,
    "div": 4,
    # (n ip ip --)
    "if": 5,
    # (ip --)
    "jump": 6,
    # (ip --)
    "call": 7,

    "return": 8,
    ";": 8,

    "extern_call": 9, # (moduleid fn# --)

    "loadword": 10,
    "*": 10,

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
}

def compile(program: str) -> bytearray:
    out = bytearray()

    labels = {}
    patchups = {}

    comment = 0
    for raw_word in filetext.split():
        word = raw_word.lower()
        if word == "(":
            comment += 1
            continue
        elif word == ")":
            comment -= 1
            continue
        
        if not comment:
            # opcode word
            if word in OPCODES:
                out.append(OPCODES[word])
                continue

            # push const
            try:
                intword = int(word)
                out.append(OPCODES["push"])
                out.append(intword & 0xff)
                out.append((intword >> 8) & 0xff)
                continue
            except ValueError:
                pass
            
            # label
            if word.startswith(":"):
                labels[word[1:]] = len(out)
                continue
            
            # string literal
            if word.startswith("\""):
                for char in raw_word[1:]:
                    out.append(ord(char))
                out.append(0)
                continue

            JUMP_IMM = "jump_imm."
            if word.startswith(JUMP_IMM):
                label = word[len(JUMP_IMM):]
                out.append(OPCODES["jump_imm"])
                patchups[len(out)] = label
                out.append(0)
                out.append(0)
                continue

            CALL_IMM = "call_imm."
            if word.startswith(CALL_IMM):
                label = word[len(CALL_IMM):]
                out.append(OPCODES["call_imm"])
                patchups[len(out)] = label
                out.append(0)
                out.append(0)
                continue

            # push label value
            if word.startswith("."):
                out.append(OPCODES["push"])
                patchups[len(out)] = word[1:]
                out.append(0) # insert space for patchup
                out.append(0)
                continue
            
            
            # short literal
            if word.startswith("##"):
                intlit = int(word[2:])
                out.append(intlit & 0xff)
                out.append((intlit >> 8) & 0xff)
                continue

            # short literal
            if word.startswith("$$"):
                intlit = int(word[2:], 16)
                out.append(intlit & 0xff)
                out.append((intlit >> 8) & 0xff)
                continue

            # byte literal
            if word.startswith("#"):
                bytelit = int(word[1:])
                out.append(bytelit & 0xff)
                continue
            
            # byte literal
            if word.startswith("$"):
                bytelit = int(word[1:], 16)
                out.append(bytelit & 0xff)
                continue
            
            
            print(f"bad word \"{word}\"")
            exit(1)
    
    # print("labels", labels)
    # print("patchups", patchups)
    for patchup_location, labelname in patchups.items():
        if labelname in labels:
            location = labels[labelname]
            out[patchup_location] = location & 0xff
            out[patchup_location+1] = (location >> 8) & 0xff
        else:
            print(f"undefined label {labelname}")

    return out

if __name__ == "__main__":
    if len(sys.argv) != 3:
        exit(1)
    filetext = open(sys.argv[1]).read()

    compiled = compile(filetext)
    # for b in compiled:
    #     print(int(b))

    with open(sys.argv[2], "wb") as out:
        out.write(compiled)


