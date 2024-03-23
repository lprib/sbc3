# Stack based VM

# VM bytecode specification
| opcode                           | val | stack effects            | description                              |
| -------------------------------- | --- | ------------------------ | ---------------------------------------- |
| `nop`                            | 0   | --                       | no op                                    |
| `+`                              | 1   | `l r -- l+r`             |                                          |
| `-`                              | 2   |                          |                                          |
| `*`                              | 3   |                          |                                          |
| `/`                              | 4   |                          |                                          |
| `%`                              | 5   |                          |                                          |
| `>>`                             | 6   |                          |                                          |
| `<<`                             | 7   |                          |                                          |
| `~`                              | 8   |                          |                                          |
| `>`                              | 9   | `l r -- l>r?`            | true=0, false=~0                         |
| `<`                              | 10  |                          |                                          |
| `>=`                             | 11  |                          |                                          |
| `<=`                             | 12  |                          |                                          |
| `==`                             | 13  |                          |                                          |
| `!=`                             | 14  |                          |                                          |
| `jump`                           | 15  | `addr -- `               | jump to addr on stack                    |
| `jump_imm[dest_lsb][dest_msb]`   | 16  | `--`                     | jump to addr in progmem                  |
| `call`                           | 17  | `addr -- `               | call to addr on stack                    |
| `call_imm[dest_lsb][dest_msb]`   | 18  | `--`                     | call to addr in progmem                  |
| `btrue`                          | 19  | `test addr -- `          | branch if test to addr on stack          |
| `btrue_imm[dest_lsb][dest_msb]`  | 20  | `test --`                | branch if test to addr in progmem        |
| `bfalse`                         | 21  | `test addr -- `          | branch if ~test to addr on stack         |
| `bfalse_imm[dest_lsb][dest_msb]` | 22  | `test --`                | branch if ~test to addr in progmem       |
| `return` or `;`                  | 23  | `--`                     | [return from call](#calling-convention)  |
| `load_module`                    | 24  | `nameptr -- id`          | load module from cstr-ptr on stack       |
| `extern_call`                    | 25  | `modid fnid --`          | call module/function                     |
| `loadword` or `@`                | 26  | `ptr -- n`               | load 2-byte word from progmem to stack   |
| `storeword` or `!`               | 27  | `n ptr --`               | store 2-byte word from stack to progmem  |
| `push_imm[imm_lsb][imm_msb]`     | 28  | `-- imm`                 | push immediate from progmem to stack     |
| `dup`                            | 29  | `n -- n n`               |                                          |
| `swap`                           | 30  | `a b -- b a`             |                                          |
| `drop`                           | 31  | `n --`                   |                                          |
| `over`                           | 32  | `a b -- a b a`           |                                          |
| `rot`                            | 33  | `a b c -- b c a`         |                                          |
| `dup2`                           | 34  | `n m -- n m n m`         |                                          |
| `swap2`                          | 35  | `a b c d -- c d a b`     |                                          |
| `over2`                          | 36  | `a b c d -- a b c d a b` |                                          |
| `drop2`                          | 37  | `a b --`                 | this is stupid                           |
| `rpush` or `>r`                  | 38  | `n -- `                  | pop from stack, push to return stack     |
| `rpop` or `r>`                   | 39  | `-- n`                   | pop from return stack, push to stack     |
| `rcopy` or `r@`                  | 40  | `-- n`                   | copy top item from return stack to stack |

# calling convention
TODO figure out how inter-module calls and returns work (push inter-module tag
to return stack so we know when returning?)

# `as2.py` syntax
```
(comment)
(
    multi
    line comment
)

fn: ( <- label )
    10 print
; ( <- return )

myvar: #0 ( <- #n adds 2-byte value n to progmem here )
#b255 ( <- single byte literal to progmem)
#0x255 ( <- hex literal to progmem)
#b0x255 ( <- single byte hex)

mystr: "henlo" ( <- adds null-terminated string here )

entry:
    &mylabel ( <- push address of mylabel to stack)
    mylabel ( <- call_imm to mylabel )
    123 ( <- push_imm 123 )
;

macro! ( <- call macro (syntax is dependent on macro ) )
```

# todo
- [ ] opcodes finish
- [ ] inter-module calls and rets
- [ ] port assembler to cpp
- [ ] unit test?