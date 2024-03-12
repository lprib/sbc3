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
    content = bytearray()

    labels = {}
    patchups = {}
    module_name = None
    exports = []

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
                content.append(OPCODES[word])
                continue

            # push const
            try:
                intword = int(word)
                content.append(OPCODES["push"])
                content.append(intword & 0xff)
                content.append((intword >> 8) & 0xff)
                continue
            except ValueError:
                pass
            
            # label
            if word.startswith(":"):
                labels[word[1:]] = len(content)
                continue
            
            # string literal
            if word.startswith("\""):
                for char in raw_word[1:]:
                    content.append(ord(char))
                content.append(0)
                continue

            JUMP_IMM = "jump_imm."
            if word.startswith(JUMP_IMM):
                label = word[len(JUMP_IMM):]
                content.append(OPCODES["jump_imm"])
                patchups[len(content)] = label
                content.append(0)
                content.append(0)
                continue

            CALL_IMM = "call_imm."
            if word.startswith(CALL_IMM):
                label = word[len(CALL_IMM):]
                content.append(OPCODES["call_imm"])
                patchups[len(content)] = label
                content.append(0)
                content.append(0)
                continue

            # push label value
            if word.startswith("."):
                content.append(OPCODES["push"])
                patchups[len(content)] = word[1:]
                content.append(0) # insert space for patchup
                content.append(0)
                continue
            
            
            # short literal
            if word.startswith("##"):
                intlit = int(word[2:])
                content.append(intlit & 0xff)
                content.append((intlit >> 8) & 0xff)
                continue

            # short literal
            if word.startswith("$$"):
                intlit = int(word[2:], 16)
                content.append(intlit & 0xff)
                content.append((intlit >> 8) & 0xff)
                continue

            # byte literal
            if word.startswith("#"):
                bytelit = int(word[1:])
                content.append(bytelit & 0xff)
                continue
            
            # byte literal
            if word.startswith("$"):
                bytelit = int(word[1:], 16)
                content.append(bytelit & 0xff)
                continue
            
            MODULE_NAME = "@module:"
            if word.startswith(MODULE_NAME):
                if module_name is not None:
                    print("redefinition of module name")
                    exit(1)
                else:
                    module_name = word[len(MODULE_NAME):]
                continue

            EXPORT = "@export:"
            if word.startswith(EXPORT):
                exports.append(word[len(EXPORT):])
                continue
            
            print(f"bad word \"{word}\"")
            exit(1)
    
    print("labels", labels)
    print("patchups", patchups)
    print("module_name", module_name)

    resolved_exports = {}
    for export in exports:
        if export in labels:
            resolved_exports[export] = labels[export]
        else:
            print(f"undefined label {labelname}")
            exit(1)

    print("resolved_exports", resolved_exports)


    for patchup_location, labelname in patchups.items():
        if labelname in labels:
            location = labels[labelname]
            content[patchup_location] = location & 0xff
            content[patchup_location+1] = (location >> 8) & 0xff
        else:
            print(f"undefined label {labelname}")
            exit(1)
    
    out = gen_header(module_name, resolved_exports)
    out.extend(content)

    return out

def gen_header(module_name, resolved_exports):
    header = bytearray()
    header.append(len(module_name))
    header.extend(module_name.encode("ascii"))
    header.append(len(resolved_exports))
    for fn_name, fn_offset in resolved_exports.items():
        header.append(len(fn_name))
        header.extend(fn_name.encode("ascii"))
        header.append(fn_offset)

    return header

if __name__ == "__main__":
    if len(sys.argv) != 3:
        exit(1)
    filetext = open(sys.argv[1]).read()

    compiled = compile(filetext)
    # for b in compiled:
    #     print(int(b))

    with open(sys.argv[2], "wb") as outfile:
        outfile.write(compiled)


