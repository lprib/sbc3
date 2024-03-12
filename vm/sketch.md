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
- `@module:name`: set the module name
- `@export:label`: export the label

# todo
- [ ] if
- [ ] comparisons
- [ ] struct?
- [ ] while/for
- [ ] raylib backend
- [ ] header files