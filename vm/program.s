module_name! "program"
export! entry
export! frame

system: "system"
system_module: #0

graphics: "graphics"
graphics_module: #0


load_system: &system load_module &system_module ! ;
print: &system_module @ 0 extern_call ;

set_display_buf: &graphics_module @ 0 extern_call ;
iskeydown: &graphics_module @ 1 extern_call ;

setup_screen:
    &graphics load_module &graphics_module !
    &screen set_display_buf
;

entry:
    load_system
    setup_screen
;

drawpix: (x y --) 256 * + &screen + 15 swap !b ;

clear: 0 8192 for! [ 0 &screen r@ 2 * + ! ] ;

i: #0
offset: #0

frame:
    clear

    &i @ inc
    dup 156 > if!
        [ drop 0 &i !  ]
        [ &i ! ]

    32 iskeydown (space key) if!
    [ &i @ 3 + &i !]
    [ ]


    0 10 for! [
        r@ 5 * &offset !

        0 50 for! [
            r@ dup &i @ + &offset @ + swap drawpix
            50 r@ - &i @ + &offset @ + r@ drawpix
        ]
    ]

;

screen:
zeros! #16384
