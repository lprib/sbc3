# Analysing opcode frequencies
`opcodes.csv` is taken from tilemap.sbcs as it was assembled (at compile time).

`running_opcodes.csv` is taken via runtime instrumentation of tilemap.sbcs.

## results (runtime)
Most common single opcodes:
1. push_imm
2. ret
3. call_imm
4. \+
5. \*
6. bfalse_imm (ie $if)
7. @
8. r\> (most likely from $for)
9. \>r (most likely from $for)

Most common pairs of opcodes:
1. push_imm + 
   - Eg struct address offsets. Should add a load with offset opcode?
2. call_imm push_imm
3. push_imm ret
   - because macros are not implemented, and constants are `const_name: 42 ;`
4. ret *
5. call_imm call_imm
6. \+ push_imm
7. \+ ret


Overall not a large or diverse enough program to properly profile yet