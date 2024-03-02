.entry jump

( todo: system module should be hardcoded to be module id 0, and dont need to
load it )

:systemname "system
:system ##0

:entry
    .systemname .system load_module
    1 1 add 3 mul
    .system * 0 extern_call
( ; )