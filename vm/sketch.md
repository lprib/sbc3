# ideas

module is unit of code

you can dynamically load modules using loadmodule

you can call functions by index on module

modules should have a header which defines the function offsets in a table


libmymodule
```
@double
@triple
@get_magic_num

:double 2 mul ;
:triple 3 mul ;
:get_magic_num 42 ;
```


program
```
:libmymodule_name "libmymodule
:libmymodule #00

:main
    ( load the module )
    .libmymodule_name loadmodule

    .libmymodule 0 extern_call ( call first fn )
;
```

# asm syntax
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

# todo
- [ ] if
- [ ] comparisons
- [ ] struct?
- [ ] while/for
- [ ] raylib backend
- [ ] header files