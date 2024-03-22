module_name! "program"
export! mul2
export! entry

system: "system"
system_module: #0

mul2: 2 * ;

print: system_modue @ 0 extern_call ;

load_system:
    system load_module system_module !
;

entry:
    5 print
    10 print
;