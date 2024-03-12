( this is a computed jump, push label address then jump to it )
( .entry jump )

( this is an immediate jump, dest baked to program )
( jump_imm.entry )

( todo: system module should be hardcoded to be module id 0, and dont need to
load it )

@module:program
@export:entry
@export:mul2

:systemname "system
:system ##99

:mul2 2 mul ;

:entry
    .systemname load_module
    .system !
    
    1 1 add 3 mul
    .system * 0 extern_call

    4 call_imm.mul2 call_imm.mul2 call_imm.mul2

    ( system should always be module 0 )
    0 0 extern_call
( ; )