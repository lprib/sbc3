module_name! "program"
export! mul2
export! entry

system: "system"
system_module: #0

i: #0

mul2: 2 * ;

print: &system_module @ 0 extern_call ;

load_system:
    &system load_module &system_module !
;

printy:
    0 5 for! [
        r@ &i !
        0 &i @ for! [
            &i @ print
        ]
    ]
;

entry:
    load_system
    printy
;

(
35: push_imm
36-37: short_imm: 0
38: push_imm
39-40: short_imm: 5
(0 5)
41: rpush
42: rpush
(r: 5 0)
43: __generated_loop_start_1:
43: rcopy2
(5 0)
44: >
(5>0)
45: bfalse_imm
46-47: branch_target: __generated_end_2
48: push_imm
49-50: short_imm: 99
51: call_imm
52-53: call_target: print
54: rpop
(r: 5) (0)
55: inc
(r: 5) (1)
56: rpush
(r: 5 1) ()
57: jump_imm
58-59: branch_target: __generated_loop_start_1
60: __generated_end_2:
60: ;
)