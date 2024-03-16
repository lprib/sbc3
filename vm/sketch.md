# ideas

module is unit of code

you can dynamically load modules using loadmodule

you can call functions by index on module

modules should have a header which defines the function offsets in a table


# module structure

| name            | size(bytes)     | description                             |
| --------------- | --------------- | --------------------------------------- |
| module_name_len | 1               | length in bytes of name to follow       |
| module_name     | module_name_len | name data                               |
| num_fns         | 1               | number of function entries to follow    |
| **fn_entry[n]** |                 | **single function entry:**              |
| fn_name_len[n]  | 1               | length in bytes of name to follow       |
| fn_name[n]      | fn_name_len     | name data                               |
| entry offset[n] | 2               | little endian offset from start of file |
| data and code   |                 | data and code                           |

# asm syntax (outdated)
- `<opcode name>`: add opcode to asm
- `:label`: label points to current place in program
- `"string`: add zero-term string to program at current position
- `jump_imm.label`: immediate-value jump to label
- `call_imm.label`: immediate-value jump to label
- `.label`: push value of label to stack
- `##123`: include raw word (LE) in program
- `#123`: include raw byte in program
- `$$123`: include raw hex word (LE) in program
- `$123`: include raw hex byte in program
- `123`: push 123 to stack
- `@module:name`: set the module name
- `@export:label`: export the label

# required opcodes
## control flow
```
3 4 < if
    ( things )
fi

( compiles to: )
3 4 < jumpfalse.label
    ( things )
:label
```

do loops:
need to put the loop counter on the return stack so it doesn't interfere with
the user's operations. loops cannot escape a function scope so it's ok.

Need a word for cloning the top of the return stack (loop counter) on to the
parameter stack.


```
0 10 do
    ( things )
od


( compiles to: )
10 rpush
0 rpush
loop:
    ( things )

    rpop ( pop loop counter off return stack )
    dup ( dup for comparison )
    rcopy ( copy loop bound )
    < ( stack: loopcounter, boundscheck)
    swap inc rpush ( increment loop counter, put back on return stack )
    ( stack: boundscheck )
    jumptrue.loop
```

Need expressive macro system so these can be expressed in standard lib.

```
( creating macro )
macro do $start $end $body
$end rpush
$start rpush
$:loop
    $body

    rpop dup rcopy <
    swap inc rpush
    jumptrue.$:loop
end

( using macro )
do![0 10 [ hello print ]]
```

When encountering `{`, fully macro expand the code within before storing it to `$body`.

# forward decls
```
decl![myfunc]

entry:
    myfunc (patchups[here] = myfunc)
    ret

myfunc:
    10 print
    ret
```

# With a better parser
```
(comments without spaces)
labels:
label (if a label, push the address of the label, otherwise treat as opcode)
"string literal" (in program memory)
jump![label]
call![label]
#1000 (decimal short literal)
#b255 (decimal byte literal)
#0xAAFF (hex short literal)
#b0x0F (hex byte literal)
module!(mymodule)
export!(myfunction)
```

# macros
## if
```
if![
    [ (if branch) ]
    [ (else branch) ]
]
```
will currently get lexed as
```
MACRO "if"
BLOCK "[ (if branch) ] [ (else branch) ]"
```

there should be a macro handler registered for `if` that feeds the next BLOCK
into another lexer. The output of _that_ lexer would be

```
BLOCK "[ (if branch) ]"
BLOCK "[ (else branch) ]"
```

Which again needs to be recursively expanded and pasted in alongside some jump
instructions.


If we abstract out `Module.compile_next_token` or similar, we don't even need
the surrounding block and can write if like this
```
if!  [
    (if block)
] else [
    (else block)
]
```

The `if!` macro handler can take control of the compilation and decide to
1. Get the next token from lexer, assert it is block and store
2. Get next token, assert it is word == "else"
3. Get the next token from lexer, assert it is block and store
4. Compile the jump structure and interlace with compiling the blocks as if they
    were pasted in to the program at that point


## generalizing macro structure
Maybe have rust-like macros where the macro can expect arbitrary tokens on input
that can be transformed
```
defmacro! if [ $ifblock else $elseblock ] => [
    &elselabel jumpfalse 
    $ifblock
    &exitlabel jump
elselabel:
    $elseblock
exitlabel:
]
```
In that case `$ifblock` will expect an arbitrary token (and expand if it's a
block), `else` will expect that specific syntax, and `$elseblock` will expect
token.

## do
```
do! 1 5 [
    (do things)
]
```



# todo
- [ ] if
- [ ] comparisons
- [ ] struct?
- [ ] while/for
- [ ] raylib backend
- [ ] header files