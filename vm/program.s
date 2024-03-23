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
    4 23760 != if!
        [ 1 print ]
        [ 0 print ]
;