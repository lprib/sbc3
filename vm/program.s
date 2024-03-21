module_name! "program"
export! mul2
export! entry

mul2: 2 * ;

print: 0 0 extern_call ;

entry:
    5 print
    10 print
    ;