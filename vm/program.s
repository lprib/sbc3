module_name! "program"
export! mul2
export! entry

system: "system"
system_module: #0

mul2: 2 * ;

print: &system_module @ 0 extern_call ;

load_system:
    &system load_module &system_module !
;

entry:
    load_system
    5 print
    5 mul2 mul2 print
;